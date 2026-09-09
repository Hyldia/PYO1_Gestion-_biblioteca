#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "catalogo.h"
#include "persistencia.h"
#include "tipos.h"

/*
 * Verifica si un campo de texto contiene al menos un caracter visible.
 * Retorna:
 * 1 Si el campo tiene texto valido.
 * 0 Si es nulo, vacio o solo tiene espacios en blanco.
 */
int validarCampoNoVacio(const char *texto) {
    if (texto == NULL || *texto == '\0') return 0;
    while (*texto != '\0') {
        if (!isspace((unsigned char)*texto)) return 1;
        texto++;
    }
    return 0;
}

/*
 * Extrae tokens considerando campos vacios entre numerales (##).
 */
static char* extraerElemento(char **cadena, const char *delimitador) {
    if (*cadena == NULL) return NULL;
    char *inicio = *cadena;
    char *p = strpbrk(inicio, delimitador);
    if (p) {
        *p = '\0';
        *cadena = p + 1;
    } else {
        *cadena = NULL;
    }
    return inicio;
}

/*
*agrega los lotes de libros al catalogo
*/
void agregarLote(void) {
    char ruta[256];
    printf("Ingrese la ruta del archivo de texto plano a procesar: ");
    if (fgets(ruta, sizeof(ruta), stdin) == NULL) return;
    ruta[strcspn(ruta, "\r\n")] = 0;

    FILE *archivo = fopen(ruta, "r");
    if (!archivo) { 
        printf("--------------------------------------------------\n");
        printf("Error: No se pudo abrir el archivo en la ruta: %s\n", ruta);
        printf("--------------------------------------------------\n");
        return;
    }

    char linea[1024];
    int procesados = 0;
    int no_procesados = 0;
    int numero_linea = 0;

    printf("\n--- Procesando Archivo de Lote ---\n");

    while (fgets(linea, sizeof(linea), archivo)) {
        numero_linea++;
        linea[strcspn(linea, "\r\n")] = 0; // Limpiar saltos de linea Windows/Linux

        if (!validarCampoNoVacio(linea)) continue;//si la linea esta 100% vacia se ignora

        Produccion prod;
        memset(&prod, 0, sizeof(Produccion));// Inicializar la estructura de producción

        char *cursor = linea;
        char *token = NULL;

        //valida Nombre
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] Falta el campo 'Nombre' por completar.\n", numero_linea);
            no_procesados++;
            continue;
        }
        snprintf(prod.nombre, sizeof(prod.nombre), "%s", token);

        //valida que no haya repetidos/unicidad(el nombre da la unicidad)
        if (existeProduccionJSON(prod.nombre)) {
            printf("[OMITIDO - Línea %d] La producción '%s' ya existe en el catálogo.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }

        //valida autor
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Autor' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        snprintf(prod.autor, sizeof(prod.autor), "%s", token);

        //valida Año de publicacion del libro
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Año de publicación' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        prod.anio_publicacion = atoi(token);

        //valida genero del libro
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Género' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        snprintf(prod.genero, sizeof(prod.genero), "%s", token);

        //valida resumen
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Resumen' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        snprintf(prod.resumen, sizeof(prod.resumen), "%s", token);

        //Valida cantidad
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Cantidad' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        prod.cantidad = atoi(token);

        if (prod.cantidad <= 0) {
            printf("[OMITIDO - Línea %d] En '%s': la cantidad debe ser mayor a 0.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }

        guardarProduccionJSON(prod);
        generarEjemplaresJSON(prod.nombre, prod.cantidad);

        procesados++;
    }

    fclose(archivo);

    printf("\n==================================================\n");
    printf("Carga finalizada.\n");
    printf("Registros procesados con éxito: %d\n", procesados);
    printf("Registros no procesados/omitidos: %d\n", no_procesados);
    printf("==================================================\n");
}

void mostrarCatalogo(void) {
    printf("\n--- Catálogo de Producciones ---\n");
    printf("(Funcionalidad en desarrollo)\n");
}

void menuCatalogo(void) {
    int opcion;
    while (1) {
        printf("\n--- Menú Catálogo ---\n");
        printf("1. Agregar catálogo por lote\n");
        printf("2. Mostrar catálogo\n");
        printf("0. Volver\n");
        printf("Seleccione una opción: ");
        
        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n'); // Limpiar buffer
            printf("Opción inválida.\n");
            continue;
        }
        while (getchar() != '\n'); // Limpiar buffer
        
        switch (opcion) {
            case 1:
                agregarLote();
                break;
            case 2:
                mostrarCatalogo();
                break;
            case 0:
                return;
            default:
                printf("Opción inválida.\n");
        }
    }
}

