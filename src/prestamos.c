/*
* Modulo de prestamo de ejemplares.
* Registra un prestamo: valida usuario, fechas y ejemplares, actualiza el estado de los ejemplares y genera el comprobante.
* El acceso a los archivos JSON se hace por medio de persistencia.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "prestamos.h"
#include "persistencia.h"
#include "tipos.h"

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
    printf("Identificacion del usuario: ");
    char *usuario = leerLinea();
    if (usuario == NULL) return NULL;

    if (!existeIdentificacionJSON(usuario)) {
        printf("-----------------------------\n");
        printf("Error: el usuario no existe.\n");
        free(usuario);
        return NULL;
    }
    return usuario;
}

/*
* Objetivo: pedir una fecha por consola y validar su formato.
* Entradas: rotulo - texto que se muestra al pedir la fecha.
* Salidas: puntero malloc con la fecha valida, o NULL si el formato falla.
*/
static char *pedirFecha(const char *rotulo) {
    printf("%s", rotulo);
    char *fecha = leerLinea();
    if (fecha == NULL) return NULL;

    if (!fechaValida(fecha)) {
        printf("-----------------------------\n");
        printf("Error: la fecha debe tener el formato YYYY-MM-DD.\n");
        free(fecha);
        return NULL;
    }
    return fecha;
}

/*
* Objetivo: agregar un id de ejemplar a un arreglo dinamico de cadenas.
* Entradas: 
*            lista    - direccion del arreglo
*            cantidad - direccion del contador
*            id       - cadena a agregar, el arreglo se queda con el puntero.
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

    printf("Ingrese los identificadores de ejemplares.\n");
    printf("(deje la linea vacia y presione Enter para terminar)\n");

    while (1) {
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
        printf("   - %s\n", p->ejemplares[i]);
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
    if (usuario == NULL) return;

    char *fechaInicio = pedirFecha("Fecha de inicio (YYYY-MM-DD): ");
    if (fechaInicio == NULL) {
        free(usuario);
        return;
    }

    char *fechaEntrega = pedirFecha("Fecha de entrega (YYYY-MM-DD): ");
    if (fechaEntrega == NULL) {
        free(usuario);
        free(fechaInicio);
        return;
    }

    if (strcmp(fechaInicio, fechaEntrega) > 0) {
        printf("-----------------------------\n");
        printf("Error: la fecha de inicio no puede ser posterior a la de entrega.\n");
        free(usuario);
        free(fechaInicio);
        free(fechaEntrega);
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