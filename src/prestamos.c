/*
* Modulo de prestamo de ejemplares.
* Registra un prestamo: valida usuario, fechas y ejemplares, actualiza el estado de los ejemplares y genera el comprobante.
* El acceso a los archivos JSON se hace por medio de persistencia.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "prestamos.h"
#include "persistencia.h"
#include "tipos.h"

#include <time.h>
#include <cjson/cJSON.h>

/* Helpers generales*/

/* Objetivo:  crear una copia en memoria dinamica de una cadena
* Entradas:  origen - cadena para copiar
* Salidas:   puntero malloc con la copia, o NULL si falla la reserva.
*/
static char *copiarCadena(const char *origen) {
    char *copia = malloc(strlen(origen) + 1);
    if (copia == NULL) return NULL;
    strcpy(copia, origen);
    return copia;
}

/*
* Objetivo:  leer una linea completa de la consola en memoria dinamica. (fgetc + realloc)
* Entradas:  ninguna
* Salidas:   puntero malloc con el texto sin '\n'; el que llama lo libera.Devuelve NULL si se llega al fin de la entrada sin leer nada.
* Restricciones: el buffer crece al doble cada vez que se llena.
*/
static char *leerLinea(void) {
    size_t capacidad = 16;
    size_t largo = 0;
    char  *linea = malloc(capacidad);
    if (linea == NULL) return NULL;

    int c;
    while ((c = fgetc(stdin)) != EOF && c != '\n') {
        if (largo + 1 >= capacidad) {
            capacidad *= 2;
            char *tmp = realloc(linea, capacidad);
            if (tmp == NULL) { free(linea); return NULL; }
            linea = tmp;
        }
        linea[largo++] = (char)c;
    }

    if (c == EOF && largo == 0) {
        free(linea);
        return NULL;
    }

    linea[largo] = '\0';
    if (largo > 0 && linea[largo - 1] == '\r') {   // entrada estilo Windows
        linea[largo - 1] = '\0';
    }
    return linea;
}

/*
* Objetivo: liberar un arreglo dinamico de cadenas y el arreglo mismo.
* Entradas: lista - arreglo de punteros; cantidad - cuantos elementos tiene.
* Salidas: ninguna.
*/
static void liberarLista(char **lista, int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        free(lista[i]);
    }
    free(lista);
}

/* Validaciones*/

/*
* Objetivo: validar que una fecha tenga el formato "YYYY-MM-DD".
* Entradas: fecha - cadena a validar.
* Salidas: 1 si el formato es correcto, 0 si no.
* Restricciones: solo valida el formato, no el calendario.
*/
static int fechaValida(const char *fecha) {
    if (fecha == NULL || strlen(fecha) != 10) return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (fecha[i] != '-') return 0;
        } else if (!isdigit((unsigned char)fecha[i])) {
            return 0;
        }
    }
    return 1;
}

/* Entrada de datos */

/*
* Objetivo:  pedir la identificacion del usuario y validar que exista.
* Entradas: ninguna porque lee desde consola
* Salidas: puntero malloc con la identificacion, o NULL si el usuario no existe o si se corto la entrada.
* Restricciones: el que llama debe liberar la memoria.
*/
static char *pedirUsuario(void) {
    while (1) {
        printf("Identificacion del usuario (vacio para cancelar): ");
        char *usuario = leerLinea();
        if (usuario == NULL){
            return NULL;
        }
        if (usuario[0] == '\0') {
            free(usuario); return NULL;// cancelar
        }
        if (existeIdentificacionJSON(usuario)) {
            return usuario;// ok
        }
        printf("El usuario no existe. Intente de nuevo.\n");
        free(usuario);
    }
}

/*
* Objetivo: pedir una fecha por consola y validar su formato.
* Entradas: rotulo - texto que se muestra al pedir la fecha.
* Salidas: puntero malloc con la fecha valida, o NULL si el formato falla.
*/
static char *pedirFecha(const char *rotulo) {
    while (1) {
        printf("%s", rotulo);
        char *fecha = leerLinea();
        if (fecha == NULL){
            return NULL;
        }
        if (fecha[0] == '\0') {
            free(fecha);
            return NULL; 
        }       // cancelar
        if (fechaValida(fecha)) {
            return fecha;
        }
        printf("Formato invalido, use YYYY-MM-DD. Intente de nuevo.\n");
        free(fecha);
    }
}

/*
* Objetivo:  pedir fecha de inicio y de entrega, repitiendo hasta que sean validas y inicio <= entrega. Vacio en cualquiera cancela
* Entradas: inicio, entrega - direcciones donde se dejan las fechas
* Salidas: 1 si quedaron dos fechas validas; 0 si el usuario cancelo
*/
static int pedirRangoFechas(char **inicio, char **entrega) {
    while (1) {
        char *fi = pedirFecha("Fecha de inicio (YYYY-MM-DD, vacio para cancelar): ");
        if (fi == NULL){
            return 0;
        }
        char *fe = pedirFecha("Fecha de entrega (YYYY-MM-DD, vacio para cancelar): ");
        if (fe == NULL) {
            free(fi);
            return 0; 
        }
        if (strcmp(fi, fe) <= 0) {
            *inicio  = fi;
            *entrega = fe;
            return 1;
        }
        printf("   La fecha de inicio no puede ser posterior a la de entrega. Intente de nuevo.\n");
        free(fi);
        free(fe);
    }
}

/*
* Objetivo: agregar un id de ejemplar a un arreglo dinamico de cadenas.
* Entradas: 
*            lista- direccion del arreglo
*            cantidad - direccion del contador
*            id- cadena a agregar, el arreglo se queda con el puntero.
* Salidas: 1 si se agrego, 0 si fallo la reserva de memoria.
*/
static int agregarEjemplar(char ***lista, int *cantidad, char *id) {
    char **tmp = realloc(*lista, (*cantidad + 1) * sizeof(char *));
    if (tmp == NULL) return 0;
    *lista = tmp;
    (*lista)[*cantidad] = id;
    (*cantidad)++;
    return 1;
}

/*
* Objetivo:  pedir ids de ejemplares hasta una linea vacia, validando cada uno.
* Entradas:  fInicio, fFin - rango de fechas del prestamo. cantidad - direccion donde se deja el total agregado (por referencia).
* Salidas:   arreglo dinamico de ids validos (o NULL si no hubo ninguno).
* Restricciones: cada id debe existir y estar disponible en ese rango.
*/
static char **pedirEjemplares(const char *fInicio, const char *fFin, int *cantidad) {
    char **ejemplares = NULL;
    *cantidad = 0;

    printf("Ingrese los identificadores de ejemplares que desea.\n");
    printf("(deje la linea vacia y presione Enter para terminar)\n");

    while (1) { /* while 1 porque es un bucle infinito hasta que se ingrese una linea vacia */
        printf(" id ejemplar: ");
        char *id = leerLinea();
        if (id == NULL){
            break;
        }
        if (id[0] == '\0') { 
            free(id); break; 
        }

        if (!existeEjemplarJSON(id)) {
            printf(" Ese ejemplar no existe, se omite.\n");
            free(id);
            continue;
        }
        if (!ejemplarDisponibleJSON(id, fInicio, fFin)) {
            printf("No esta disponible en esas fechas, se omite.\n");
            free(id);
            continue;
        }
        if (!agregarEjemplar(&ejemplares, cantidad, id)) {
            printf("Error de memoria.\n");
            free(id);
            break;
        }
        printf("Desea ingresar otro ejemplar? (s/n): ");
        char *respuesta = leerLinea();
        int continuar = 0;
        if (respuesta != NULL) {
            if (respuesta[0] == 's' || respuesta[0] == 'S') {
                continuar = 1;
            }
            free(respuesta);
        }
        if (!continuar) {
            break;
        }
    }
    return ejemplares;
}

/* Acciones sobre los datos*/

/*
* Objetivo:  poner en estado "Prestado" cada ejemplar de la lista.
* Entradas:  ejemplares - arreglo de ids; cantidad - cuantos son.
* Salidas:   ninguna (modifica ejemplares.json).
*/
static void marcarEjemplaresPrestados(char **ejemplares, int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        cambiarEstadoEjemplarJSON(ejemplares[i], "Prestado");
    }
}

/*
* Objetivo:  mostrar en pantalla el comprobante del prestamo.
* Entradas:  p - puntero al prestamo (paso por referencia, solo lectura).
* Salidas:   ninguna.
*/
static void imprimirComprobante(const Prestamo *p) { 
    printf("\n==================================================\n");
    printf("            COMPROBANTE DE PRESTAMO\n");
    printf("==================================================\n");
    printf("Prestamo #%d\n", p->id);
    printf("Usuario: %s\n", p->usuario);
    printf("Fecha inicio:%s\n", p->fecha_inicio);
    printf("Fecha entrega: %s\n", p->fecha_entrega);
    printf("Ejemplares prestados:\n");
     for (int i = 0; i < p->cantidad_ejemplares; i++) {
        char *nombre = obtenerProduccionEjemplarJSON(p->ejemplares[i]);
        if (nombre != NULL) {
            printf("   - %s  (id: %s)\n", nombre, p->ejemplares[i]);
            free(nombre);
        } else {
            printf("   - ?  (id: %s)\n", p->ejemplares[i]);
        }
    }
    printf("==================================================\n");
}

/*
* Objetivo:liberar toda la memoria dinamica de un Prestamo
* Entradas:p - puntero al prestamo (paso por referencia)
* Salidas:ninguna
*/
static void liberarPrestamo(Prestamo *p) {
    liberarLista(p->ejemplares, p->cantidad_ejemplares);
    free(p->usuario);
    free(p->fecha_inicio);
    free(p->fecha_entrega);
    free(p->estado);
}

/* Funcion publica: orquesta todo el flujo */

/*
* Objetivo:  registrar un nuevo prestamo de ejemplares
* Entradas:  por consola (usuario, fechas, ids de ejemplares)
* Salidas:   crea el registro en prestamos.json, marca los ejemplares como "Prestado" e imprime el comprobante
* Restricciones:
*  - el usuario debe existir
*  - fechas en formato YYYY-MM-DD y inicio <= entrega
*  - cada ejemplar debe existir y estar disponible en ese rango
*  - si no queda ningun ejemplar valido, no se crea el prestamo
*/
void registrarPrestamo(void) {

    char *usuario = pedirUsuario();
    if (usuario == NULL) {
        printf("Operacion cancelada.\n");
        return;
    }
    char *fechaInicio  = NULL;
    char *fechaEntrega = NULL;
    if (!pedirRangoFechas(&fechaInicio, &fechaEntrega)) {
        printf("Operacion cancelada.\n");
        free(usuario);
        return;
    }

    int cantidad = 0;
    char **ejemplares = pedirEjemplares(fechaInicio, fechaEntrega, &cantidad);

    if (cantidad == 0) {
        printf("-----------------------------\n");
        printf("No se registro el prestamo: ningun ejemplar valido.\n");
        free(usuario);
        free(fechaInicio);
        free(fechaEntrega);
        liberarLista(ejemplares, cantidad);
        return;
    }

    Prestamo prestamo;  /*falta agregar los datos*/
    prestamo.id = siguienteIdPrestamoJSON();
    prestamo.usuario = usuario;
    prestamo.fecha_inicio = fechaInicio;
    prestamo.fecha_entrega = fechaEntrega;
    prestamo.estado = copiarCadena("activo");
    prestamo.ejemplares = ejemplares;
    prestamo.cantidad_ejemplares = cantidad;

    guardarPrestamoJSON(prestamo);
    marcarEjemplaresPrestados(ejemplares, cantidad);
    imprimirComprobante(&prestamo);

    liberarPrestamo(&prestamo);
}

/* Objetivo:obtener la fecha del sistema en formato "YYYY-MM-DD".
* Entradas: ninguna.
* Salidas: puntero malloc con la fecha (11 bytes); el que llama lo libera.NULL si falla la reserva.
*/
static char *fechaHoy(void) {
    char *fecha = malloc(11);// "YYYY-MM-DD" + '\0'. porque 10+1=11 para el malloc
    if (fecha == NULL){
        return NULL;
    }
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    strftime(fecha, 11, "%Y-%m-%d", lt);
    return fecha;
}
/*
* Objetivo:calcular el estado que se debe mostrar de un prestamo.
* Entradas:estadoGuardado - lo que dice prestamos.json ("activo"/"finalizado").
*  fechaEntrega - fecha de entrega pactada.
*  hoy - fecha del sistema.
* Salidas: "finalizado", "vencido" o "activo".
* Restricciones: compara fechas como texto (formato YYYY-MM-DD).
*/
static const char *estadoParaMostrar(const char *estadoGuardado, const char*fechaEntrega,const char *hoy) {
    if (estadoGuardado != NULL && strcmp(estadoGuardado, "finalizado") == 0) {
        return "finalizado";
    }
    if (fechaEntrega != NULL && strcmp(fechaEntrega, hoy) < 0) {
        return "vencido";
    }
    return "activo";
}

/*Objetivo:  mostrar los prestamos cuya fecha de entrega cae dentro de un rango indicado por el usuario.
* Entradas:por consola: fecha inicio y fecha fin del rango.
* Salidas: imprime id, usuario, estado, ejemplares (nombre + id) y entrega tardia de cada prestamo del rango.
* Restricciones: el filtro es por "fecha_entrega". Vacio en una fecha cancela.
*/
void mostrarHistorialPrestamos(void) {
    //para pedir el inicio del rango, se reutiliza la funcion pedirFecha
    char *desde = pedirFecha("Fecha inicio del rango (YYYY-MM-DD, vacio para cancelar): ");
    if (desde == NULL) {
        printf("Operacion cancelada.\n");
        return;
    }

    // para pedir la fecha fin del rango, se reutiliza la funcion pedirFecha
    char *hasta = pedirFecha("Fecha fin del rango (YYYY-MM-DD, vacio para cancelar): ");
    if (hasta == NULL) {
        printf("Operacion cancelada.\n");
        free(desde);
        return;
    }

    // para validar que desde <= hasta, se comparan como cadenas (formato YYYY-MM-DD)
    if (strcmp(desde, hasta) > 0) {
        printf("El rango es invalido (inicio posterior a fin).\n");
        free(desde); free(hasta);
        return;
    }

    // para obtener la fecha de hoy, se reutiliza la funcion fechaHoy
    char *hoy = fechaHoy();
    if (hoy == NULL) { free(desde); free(hasta); return; }

    // para leer el archivo prestamos.json, se reutiliza la funcion leerArchivoCompleto
    FILE *archivo = fopen("data/prestamos.json", "r");
    if (archivo == NULL) {
        printf("No hay prestamos registrados.\n");
        free(desde); free(hasta); free(hoy);
        return;
    }
    //para leer el contenido completo del archivo, se reutiliza la funcion leerArchivoCompleto
    char *contenido = leerArchivoCompleto(archivo);
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenido);
    free(contenido);

    if (!cJSON_IsArray(raiz)) {
        printf("No hay prestamos registrados.\n");
        cJSON_Delete(raiz);
        free(desde); free(hasta); free(hoy);
        return;
    }

    // para cada prestamo se obtiene la fecha de entrega y se compara con el rango. Si esta dentro del rango se imprime.
    printf("\n===== HISTORIAL DE PRESTAMOS (%s a %s) =====\n", desde, hasta);

    int mostrados = 0;
    int total = cJSON_GetArraySize(raiz);
    for (int i = 0; i < total; i++) {
        cJSON *p = cJSON_GetArrayItem(raiz, i);

        cJSON *jEntrega = cJSON_GetObjectItem(p, "fecha_entrega");
        if (!cJSON_IsString(jEntrega)) continue;
        const char *fEntrega = jEntrega->valuestring;

        // filtro: desde <= fecha_entrega <= hasta
        if (strcmp(fEntrega, desde) < 0 || strcmp(fEntrega, hasta) > 0) {
            continue;
        }

        cJSON *jId      = cJSON_GetObjectItem(p, "id");
        cJSON *jUsuario = cJSON_GetObjectItem(p, "usuario");
        cJSON *jEstado  = cJSON_GetObjectItem(p, "estado");
        cJSON *jEjs     = cJSON_GetObjectItem(p, "ejemplares");
        cJSON *jDevol   = cJSON_GetObjectItem(p, "fecha_devolucion");

        const char *estadoGuardado = cJSON_IsString(jEstado) ? jEstado->valuestring : "activo";
        const char *estado = estadoParaMostrar(estadoGuardado, fEntrega, hoy);

        printf("\n--------------------------------------------------\n");
        printf("Prestamo #%d\n", cJSON_IsNumber(jId) ? jId->valueint : 0);
        printf("Usuario:       %s\n", cJSON_IsString(jUsuario) ? jUsuario->valuestring : "?");
        printf("Estado:        %s\n", estado);
        printf("Fecha entrega: %s\n", fEntrega);

        printf("Ejemplares:\n");
        if (cJSON_IsArray(jEjs)) {
            int ne = cJSON_GetArraySize(jEjs);
            for (int k = 0; k < ne; k++) {
                cJSON *item = cJSON_GetArrayItem(jEjs, k);
                if (!cJSON_IsString(item)) continue;

                char *nombre = obtenerProduccionEjemplarJSON(item->valuestring);
                if (nombre != NULL) {
                    printf("   - %s  (id: %s)\n", nombre, item->valuestring);
                    free(nombre);
                } else {
                    printf("   - ?  (id: %s)\n", item->valuestring);
                }
            }
        }

        // entrega tardia
        printf("Entrega tardia: ");
        if (strcmp(estadoGuardado, "finalizado") == 0 && cJSON_IsString(jDevol)) {
            if (strcmp(jDevol->valuestring, fEntrega) > 0) {
                printf("si\n");
            } else {
                printf("no\n");
            }
        } else {
            printf("-\n");
        }

        mostrados++;
    }

    if (mostrados == 0) {
        printf("\nNo hay prestamos con fecha de entrega en ese rango.\n");
    }
    printf("==================================================\n");

    // liberar memoria
    cJSON_Delete(raiz);
    free(desde);
    free(hasta);
    free(hoy);
}
/*
* Objetivo:imprimir los datos de un prestamo.
* Entradas:p   - objeto cJSON del prestamo.
* hoy - fecha del sistema (para calcular "vencido").
* Salidas: imprime id, usuario, estado, fecha de entrega, ejemplares y entrega tardia.
*/
static void imprimirUnPrestamo(cJSON *p, const char *hoy) {
    cJSON *jId      = cJSON_GetObjectItem(p, "id");
    cJSON *jUsuario = cJSON_GetObjectItem(p, "usuario");
    cJSON *jEstado  = cJSON_GetObjectItem(p, "estado");
    cJSON *jEntrega = cJSON_GetObjectItem(p, "fecha_entrega");
    cJSON *jEjs     = cJSON_GetObjectItem(p, "ejemplares");
    cJSON *jDevol   = cJSON_GetObjectItem(p, "fecha_devolucion");

    // fecha de entrega como texto
    const char *fEntrega;
    if (cJSON_IsString(jEntrega)) {
        fEntrega = jEntrega->valuestring;
    } else {
        fEntrega = "?";
    }

    // estado guardado en el archivo
    const char *estadoGuardado;
    if (cJSON_IsString(jEstado)) {
        estadoGuardado = jEstado->valuestring;
    } else {
        estadoGuardado = "activo";
    }

    // estado a mostrar
    const char *estado = estadoParaMostrar(estadoGuardado, fEntrega, hoy);

    // id
    int id = 0;
    if (cJSON_IsNumber(jId)) {
        id = jId->valueint;
    }

    // usuario
    const char *usuario;
    if (cJSON_IsString(jUsuario)) {
        usuario = jUsuario->valuestring;
    } else {
        usuario = "?";
    }

    printf("\n--------------------------------------------------\n");
    printf("Prestamo #%d\n", id);
    printf("Usuario:       %s\n", usuario);
    printf("Estado:        %s\n", estado);
    printf("Fecha entrega: %s\n", fEntrega);

    printf("Ejemplares:\n");
    if (cJSON_IsArray(jEjs)) {
        int ne = cJSON_GetArraySize(jEjs);
        for (int k = 0; k < ne; k++) {
            cJSON *item = cJSON_GetArrayItem(jEjs, k);
            if (!cJSON_IsString(item)) {
                continue;
            }

            char *nombre = obtenerProduccionEjemplarJSON(item->valuestring);
            if (nombre != NULL) {
                printf("   - %s  (id: %s)\n", nombre, item->valuestring);
                free(nombre);
            } else {
                printf("   - ?  (id: %s)\n", item->valuestring);
            }
        }
    }

    printf("Entrega tardia: ");
    if (strcmp(estadoGuardado, "finalizado") == 0 && cJSON_IsString(jDevol)) {
        if (strcmp(jDevol->valuestring, fEntrega) > 0) {
            printf("si\n");
        } else {
            printf("no\n");
        }
    } else {
        printf("-\n");
    }
}
/*
* Objetivo:mostrar TODOS los prestamos registrados, sin filtrar por fechas.
* Entradas:ninguna.
* Salidas:imprime cada prestamo con id, usuario, estado, ejemplares y entrega tardia.
*/
void mostrarTodosLosPrestamos(void) {

    char *hoy = fechaHoy();
    if (hoy == NULL) {
        return;
    }

    FILE *archivo = fopen("data/prestamos.json", "r");
    if (archivo == NULL) {
        printf("No hay prestamos registrados.\n");
        free(hoy);
        return;
    }

    char *contenido = leerArchivoCompleto(archivo);
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenido);
    free(contenido);

    if (!cJSON_IsArray(raiz)) {
        printf("No hay prestamos registrados.\n");
        cJSON_Delete(raiz);
        free(hoy);
        return;
    }

    int total = cJSON_GetArraySize(raiz);
    if (total == 0) {
        printf("No hay prestamos registrados.\n");
    } else {
        printf("\n===== TODOS LOS PRESTAMOS =====\n");
        for (int i = 0; i < total; i++) {
            cJSON *p = cJSON_GetArrayItem(raiz, i);
            imprimirUnPrestamo(p, hoy);
        }
        printf("==================================================\n");
    }

    cJSON_Delete(raiz);
    free(hoy);
}

/* Vencimiento de prestamos */

/* Objetivo:convertir una fecha "YYYY-MM-DD" a time_t (segundos)
* Entradas:fecha - cadena con formato YYYY-MM-DD
* Salidas:el time_t de esa fecha*/
static time_t fechaATiempo(const char *fecha) {
    if (fecha == NULL || strlen(fecha) != 10) { /*strlen es para obtener la longitud de la cadena*/
        return (time_t)-1; /*es -1 si la fecha es invalida*/
    }
    struct tm t; /*estructura para almacenar la fecha*/
    memset(&t, 0, sizeof(t));
    t.tm_year  = atoi(fecha) - 1900;// "YYYY"
    t.tm_mon   = atoi(fecha + 5) - 1;// "MM"
    t.tm_mday  = atoi(fecha + 8);// "DD"
    t.tm_hour  = 12;// mediodia
    t.tm_isdst = -1;

    return mktime(&t); /*mktime convierte una estructura tm a time_t*/
}

/*
* Objetivo:calcular cuantos dias hay desde desde hasta
* Entradas:desde, hasta - fechas "YYYY-MM-DD".
* Salidas:diferencia en dias (hasta - desde). Positivo si 'hasta' es posterior. Devuelve LONG_MIN si alguna fecha es invalida.
*/
static long diasEntre(const char *desde, const char *hasta) {
    time_t td = fechaATiempo(desde); /*td es la fecha de inicio en time_t*/
    time_t th = fechaATiempo(hasta); /*th es la fecha de final en time_t*/

    if (td == (time_t)-1 || th == (time_t)-1) {
        return LONG_MIN; /*se devueve el long min que es el valor mas pequeño posible*/
    }

    double segundos = difftime(th, td); /*diferencia en segundos con difftime que hace la resta*/
    return (long)(segundos / 86400.0); /*convertir a dias*/
}
/*
* Objetivo:  imprimir la lista de ejemplares de un prestamo (nombre + id).
* Entradas:  jEjemplares - arreglo cJSON con los ids de ejemplar.
* Salidas:   imprime cada ejemplar; nada si el arreglo es invalido.
*/
static void imprimirEjemplaresDe(cJSON *jEjemplares) {
    if (!cJSON_IsArray(jEjemplares)) { /*si no es un arreglo*/
        return;
    }

    int ne = cJSON_GetArraySize(jEjemplares); /*obtener el tamaño del arreglo*/
    for (int k = 0; k < ne; k++) {
        cJSON *item = cJSON_GetArrayItem(jEjemplares, k); /*obtener el elemento k del arreglo*/
        if (!cJSON_IsString(item)) { /*si el elemento no es una cadena*/
            continue; /* continuar con el siguiente elemento */
        }

        char *nombre = obtenerProduccionEjemplarJSON(item->valuestring); /*obtener el nombre del ejemplar*/
        if (nombre != NULL) {
            printf("- %s  (id: %s)\n", nombre, item->valuestring);
            free(nombre); /*se libera la memoria del nombre*/
        } else {
            printf("- ?  (id: %s)\n", item->valuestring);
        }
    }
}

/*
* Objetivo:mostrar los prestamos vencidos y los proximos a vencer(fecha de entrega de 0 a 5 dias respecto de hoy).
* Entradas:ninguna (usa la fecha del sistema).
* Salidas:imprime id, usuario, fecha de entrega, estatus y ejemplares.
* Restricciones: solo considera prestamos con estado "activo".
*/
void mostrarVencimientoPrestamos(void) {

    char *hoy = fechaHoy();
    if (hoy == NULL) {
        return;
    }

    FILE *archivo = fopen("data/prestamos.json", "r"); /*abrir el archivo en modo lectura*/
    if (archivo == NULL) { /*si no se puede abrir el archivo*/
        printf("No hay prestamos registrados.\n"); /*imprimir mensaje de error*/
        free(hoy);
        return;
    }

    char *contenido = leerArchivoCompleto(archivo); /*leer el contenido del archivo*/
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenido); /*parsear el contenido como JSON*/
    free(contenido);

    if (!cJSON_IsArray(raiz)) {
        printf("No hay prestamos registrados.\n");
        cJSON_Delete(raiz);
        free(hoy);
        return;
    }

    printf("\n==== VENCIMIENTO DE PRESTAMOS ====\n");

    int mostrados = 0;
    int total = cJSON_GetArraySize(raiz);
    for (int i = 0; i < total; i++) {

        /* p es un prestamo , JEstado es el objeto JSON del estado , JEntrega es el objeto JSON de la fecha de entrega */
        cJSON *p = cJSON_GetArrayItem(raiz, i);

        cJSON *jEstado  = cJSON_GetObjectItem(p, "estado");
        cJSON *jEntrega = cJSON_GetObjectItem(p, "fecha_entrega");

        // solo prestamos activos
        if (!cJSON_IsString(jEstado) || strcmp(jEstado->valuestring, "activo") != 0) {
            continue;
        }
        if (!cJSON_IsString(jEntrega)) {
            continue;
        }

        const char *fEntrega = jEntrega->valuestring; 
        long dias = diasEntre(hoy, fEntrega);   // dias desde hoy hasta la entrega
        if (dias == LONG_MIN) {
            continue;
        }

        // clasificar
        const char *estatus;
        if (dias < 0) {
            estatus = "vencido";
        } else if (dias <= 5) {
            estatus = "proximo a vencer";
        } else {
            continue;   // falta mas de 5 dias, nada
        }

        // datos
        cJSON *jId         = cJSON_GetObjectItem(p, "id");
        cJSON *jUsuario    = cJSON_GetObjectItem(p, "usuario");
        cJSON *jEjemplares = cJSON_GetObjectItem(p, "ejemplares");

        int id = 0;
        if (cJSON_IsNumber(jId)) {
            id = jId->valueint;
        }

        const char *usuario;
        if (cJSON_IsString(jUsuario)) { /*el dato esta bien*/
            usuario = jUsuario->valuestring;
        } else {
            usuario = "?";  /*el dato esta mal, se usa ? */
        }

        printf("\n--------------------------------------------------\n");
        printf("Prestamo #%d\n", id);
        printf("Usuario: %s\n", usuario);
        printf("Fecha entrega: %s\n", fEntrega);
        printf("Estatus: %s\n", estatus);
        printf("Ejemplares:\n");
        imprimirEjemplaresDe(jEjemplares);

        mostrados++;
    }

    if (mostrados == 0) {
        printf("\nNo hay prestamos vencidos ni proximos a vencer.\n");
    }
    printf("==================================================\n");

    cJSON_Delete(raiz);
    free(hoy);
}