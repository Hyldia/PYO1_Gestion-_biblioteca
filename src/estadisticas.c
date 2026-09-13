/*
* Modulo de estadisticas.
* Calcula y muestra: top producciones prestadas, top usuarios con mas prestamos, top mes-anio con mas recaudacion, y genero con mas/menos prestamos. Lee los datos de prestamos.json y de persistencia.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "estadisticas.h"
#include "persistencia.h"

typedef struct { /*para almacenar el nombre de una produccion o usuario y su cantidad de prestamos */
    char *nombre;
    int   contador;
} Conteo;

/*
* Objetivo:  sumar un valor al contador de una clave, agregandola si no existe.
* Entradas:  lista - direccion del arreglo dinamico; cantidad - direccion deltotal de elementos; clave - texto a buscar/agregar; valor - loque se suma (1 para contar, un monto para acumular dinero)
* Salidas:   ninguna (modifica lista y cantidad)
*/
static void sumarConteo(Conteo **lista, int *cantidad, const char *clave, int valor) {
    for (int i = 0; i < *cantidad; i++) {
        if (strcmp((*lista)[i].nombre, clave) == 0) {
            (*lista)[i].contador += valor;
            return;
        }
    }

    Conteo *tmp = realloc(*lista, (*cantidad + 1) * sizeof(Conteo)); /*realloc es para redimensionar un bloque de memoria , porque necesitamos aumentar el tamaño del arreglo */
    if (tmp == NULL) {
        return;
    }
    *lista = tmp; /*para actualizar la dirección del arreglo*/

    char *copia = malloc(strlen(clave) + 1);
    if (copia == NULL) {
        return;
    }
    strcpy(copia, clave);

    (*lista)[*cantidad].nombre   = copia;
    (*lista)[*cantidad].contador = valor;
    (*cantidad)++;
}

/*
* Objetivo:liberar un arreglo de Conteo y sus nombres
* Entradas:lista - arreglo; cantidad - cuantos elementos tiene
* Salidas:ninguna
*/
static void liberarConteos(Conteo *lista, int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        free(lista[i].nombre);
    }
    free(lista);
}

/*
* Objetivo: mostrar los "n" elementos con mayor contador, de mayor a menor.
* Entradas:lista - arreglo de conteos; cantidad - cuantos hay; n - cuantos mostrar; etiqueta - texto que acompaña al numero.
* Salidas:imprime "puesto. nombre - numero etiqueta".
*/
static void mostrarTop(Conteo *lista, int cantidad, int n, const char *etiqueta) {
    if (cantidad == 0) {
        printf("No hay datos.\n");
        return;
    }

    int *usado = malloc(cantidad * sizeof(int)); /*malloc para reservar memoria para el arreglo de enteros y sizeof(int) para obtener el tamaño de un entero */
    if (usado == NULL) {
        return;
    }
    for (int i = 0; i < cantidad; i++) { /*para saber qué elementos ya se han mostrado */
        usado[i] = 0;
    }

    int mostrados = 0;
    while (mostrados < n && mostrados < cantidad) {
        int mejor = -1;
        for (int i = 0; i < cantidad; i++) {
            if (usado[i]) {
                continue;
            }
            if (mejor == -1 || lista[i].contador > lista[mejor].contador) {
                mejor = i;
            }
        }
        if (mejor == -1) { /*si no se encontró un mejor elemento */
            break;
        }
        printf("   %d. %s - %d %s\n", mostrados + 1, lista[mejor].nombre, lista[mejor].contador, etiqueta);
        usado[mejor] = 1;
        mostrados++;
    }

    free(usado);
}

/* Cada estadistica*/

/*
* Objetivo:Top 3 de producciones mas prestadas.
* Entradas:prestamos - arreglo cJSON con todos los prestamos.
* Salidas:imprime el top 3.
*/
static void estadisticaA(cJSON *prestamos) {
    Conteo *lista = NULL;
    int cantidad = 0;

    int total = cJSON_GetArraySize(prestamos);
    for (int i = 0; i < total; i++) {
        cJSON *p = cJSON_GetArrayItem(prestamos, i);
        cJSON *jEjemplares = cJSON_GetObjectItem(p, "ejemplares");
        if (!cJSON_IsArray(jEjemplares)) {
            continue;
        }

        int ne = cJSON_GetArraySize(jEjemplares);
        for (int k = 0; k < ne; k++) {
            cJSON *item = cJSON_GetArrayItem(jEjemplares, k);
            if (!cJSON_IsString(item)) {
                continue;
            }
            char *nombre = obtenerProduccionEjemplarJSON(item->valuestring);
            if (nombre != NULL) {
                sumarConteo(&lista, &cantidad, nombre, 1);
                free(nombre);
            }
        }
    }

    printf("\nA. Top 3 producciones mas prestadas:\n");
    mostrarTop(lista, cantidad, 3, "prestamo(s)");
    liberarConteos(lista, cantidad);
}

/*
* Objetivo:B. Top 3 de usuarios con mas prestamos.
* Entradas:prestamos - arreglo cJSON con todos los prestamos.
* Salidas:imprime el top 3.
*/
static void estadisticaB(cJSON *prestamos) {
    Conteo *lista = NULL;
    int cantidad = 0;

    int total = cJSON_GetArraySize(prestamos);
    for (int i = 0; i < total; i++) {
        cJSON *p = cJSON_GetArrayItem(prestamos, i);
        cJSON *jUsuario = cJSON_GetObjectItem(p, "usuario");
        if (cJSON_IsString(jUsuario)) {
            sumarConteo(&lista, &cantidad, jUsuario->valuestring, 1);
        }
    }

    printf("\nB. Top 3 usuarios con mas prestamos:\n");
    mostrarTop(lista, cantidad, 3, "prestamo(s)");
    liberarConteos(lista, cantidad);
}

/*
* Objetivo:  C. Top 5 de mes-anio con mayor monto recaudado (segun fecha de inicio del prestamo). Solo cuenta prestamos ya finalizados.
* Entradas:prestamos - arreglo cJSON con todos los prestamos.
* Salidas: imprime el top 5.
*/
static void estadisticaC(cJSON *prestamos) {
    Conteo *lista = NULL;
    int cantidad = 0;

    int total = cJSON_GetArraySize(prestamos);
    for (int i = 0; i < total; i++) {
        cJSON *p = cJSON_GetArrayItem(prestamos, i); /*aqui se obtiene cada prestamo */

        cJSON *jEstado = cJSON_GetObjectItem(p, "estado");
        cJSON *jMonto  = cJSON_GetObjectItem(p, "monto");
        cJSON *jInicio = cJSON_GetObjectItem(p, "fecha_inicio");

        if (!cJSON_IsString(jEstado) || strcmp(jEstado->valuestring, "finalizado") != 0) {
            continue;
        }
        if (!cJSON_IsNumber(jMonto) || !cJSON_IsString(jInicio)) {
            continue;
        }
        if (strlen(jInicio->valuestring) < 7) {
            continue;
        }

        char *mesAnio = malloc(8);   // "YYYY-MM" + '\0'
        if (mesAnio == NULL) {
            continue;
        }
        strncpy(mesAnio, jInicio->valuestring, 7);
        mesAnio[7] = '\0';

        sumarConteo(&lista, &cantidad, mesAnio, jMonto->valueint);
        free(mesAnio);
    }

    printf("\nC. Top 5 mes-año con mayor monto recaudado:\n");
    mostrarTop(lista, cantidad, 5, "colones recaudados");
    liberarConteos(lista, cantidad);
}

/*
* Objetivo:D. Genero con mas prestamos y genero con menos prestamos.
* Entradas:prestamos - arreglo cJSON con todos los prestamos.
* Salidas:imprime ambos generos con su cantidad.
*/
static void estadisticaD(cJSON *prestamos) {
    Conteo *lista = NULL;
    int cantidad = 0;

    int total = cJSON_GetArraySize(prestamos);
    for (int i = 0; i < total; i++) {
        cJSON *p = cJSON_GetArrayItem(prestamos, i);
        cJSON *jEjemplares = cJSON_GetObjectItem(p, "ejemplares");
        if (!cJSON_IsArray(jEjemplares)) {
            continue;
        }

        int ne = cJSON_GetArraySize(jEjemplares); /*ne = numero de ejemplares*/
        for (int k = 0; k < ne; k++) {
            cJSON *item = cJSON_GetArrayItem(jEjemplares, k);
            if (!cJSON_IsString(item)) {
                continue;
            }

            char *nombreProd = obtenerProduccionEjemplarJSON(item->valuestring);
            if (nombreProd == NULL) {
                continue;
            }
            char *genero = obtenerGeneroProduccionJSON(nombreProd);
            free(nombreProd);

            if (genero != NULL) {
                sumarConteo(&lista, &cantidad, genero, 1);
                free(genero);
            }
        }
    }

    printf("\nD. Genero con mas y con menos prestamos:\n");
    if (cantidad == 0) {
        printf("   No hay datos.\n");
    } else {
        int indiceMax = 0;
        int indiceMin = 0;
        for (int i = 1; i < cantidad; i++) {
            if (lista[i].contador > lista[indiceMax].contador) {
                indiceMax = i;
            }
            if (lista[i].contador < lista[indiceMin].contador) {
                indiceMin = i;
            }
        }
        printf("   Con mas prestamos:   %s (%d)\n", lista[indiceMax].nombre, lista[indiceMax].contador);
        printf("   Con menos prestamos: %s (%d)\n", lista[indiceMin].nombre, lista[indiceMin].contador);
    }

    liberarConteos(lista, cantidad);
}
/*
* Objetivo:mostrar las 4 estadisticas del sistema (A, B, C, D).
* Entradas:ninguna
* Salidas:imprime cada estadistica en pantalla.
*/
void mostrarEstadisticas(void) {
    FILE *archivo = fopen("data/prestamos.json", "r");
    if (archivo == NULL) {
        printf("No hay prestamos registrados.\n");
        return;
    }

    char *contenido = leerArchivoCompleto(archivo);
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenido);
    free(contenido);

    if (!cJSON_IsArray(raiz)) {
        printf("No hay prestamos registrados.\n");
        cJSON_Delete(raiz);
        return;
    }

    /*toma los datos de los prestamos, raiz es el arreglo de prestamos*/
    printf("\n===== ESTADISTICAS =====\n");
    estadisticaA(raiz);
    estadisticaB(raiz);
    estadisticaC(raiz);
    estadisticaD(raiz);
    printf("=========================\n");

    cJSON_Delete(raiz); /*liberar memoria para el arreglo de prestamos*/
}


/*
* Logica de cada estadistica:
* A. Top 3 producciones mas prestadas:
*    Se recorre cada prestamo y se busca en cada ejemplar que contiene. Se busca de que produccion es (obtenerProduccionEjemplarJSON) y se le suma 1 a esa produccion en la lista de conteos. Al final se muestran las 3 producciones con el numero mas alto.
*
* B. Top 3 usuarios con mas prestamos:
*    Se recorre cada prestamo (no ejemplar) y se le suma 1 al usuario dueño de ese prestamo. Un prestamo con varios ejemplares sigue contando como un solo prestamo para el usuario.
*
* C. Top 5 mes-año con mayor monto recaudado:
*    Solo se toman en cuenta los prestamos con estado "finalizado". De la fecha_inicio se usan los primeros 7 caracteres ("YYYY-MM") como la clave del mes-año, y se le SUMA el monto. Al final semuestran los 5 mes-año que acumularon mas dinero.
*
* D. Genero con mas y con menos prestamos:
*    Igual que la A (prestamo -> ejemplar -> produccion) pero con el nombre de la produccion se busca su genero (obtenerGeneroProduccionJSON) y se cuenta por genero en vez de por libro. Al final NO se usa el top: se recorre la lista una vez a mano para encontrar el genero con el numero mas alto y el que tiene el numero mas bajo.
*
*/