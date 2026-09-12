#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "persistencia.h"
#include "busquedas.h"

/*
 * Realiza una busqueda simple de ejemplares
 * Busca el dato/paramtro en nombre, autor o resumen (técnica "contiene")
 * Muestra: identificador, nombre, resumen y estado
 */
void busquedaSimple(void) {
    char *dato = (char *)malloc(MAX_RUTA * sizeof(char));//reservar memoria para el dato de busqueda
    if (dato == NULL) {
        printf("Error: No se pudo asignar memoria para el dato.\n");
        return;
    }

    printf("\n---------BÚSQUEDA SIMPLE DE EJEMPLARES---------\n");
    printf("Ingrese texto a buscar: ");
    
    if (fgets(dato, MAX_RUTA, stdin) == NULL) {
        free(dato);
        return;
    }
    dato[strcspn(dato, "\r\n")] = 0;

    if (strlen(dato) == 0) {
        printf("El parametro de búsqueda no puede estar vacio.\n");
        free(dato);
        return;
    }

    // Convertir a minúsculas el texto ingresado por el usuario
    char *datoLower = aMinusculas(dato);

    int total = 0;
    ResultadoBusquedaEjemplar **resultados = buscarEjemplarSimple(dato, &total);//llama a la funcion buscarEjemplarSimple para realizar la busqueda y obtener los resultados

    if (resultados == NULL || total == 0) {
        printf("\n--------------------------------------------------\n");
        printf("No se encontraron ejemplares coincidentes con '%s'.\n", dato);
        printf("--------------------------------------------------\n");
        free(datoLower);
        free(dato);
        return;
    }

    printf("\n--------------------------------------------------\n");
    printf("Se encontraron %d ejemplar(es) coincidentes:\n", total);
    printf("--------------------------------------------------\n");

    for (int i = 0; i < total; i++) {
        printf("[%d]\n", i + 1);
        printf("  Identificador: %s\n", resultados[i]->id_ejemplar);
        printf("  Nombre:        %s\n", resultados[i]->nombre_produccion);
        printf("  Resumen:       %s\n", resultados[i]->resumen);
        printf("  Estado:        %s\n", resultados[i]->estado);
        printf("--------------------------------------------------\n");
    }

    for (int i = 0; i < total; i++) {
        free(resultados[i]->id_ejemplar);
        free(resultados[i]->nombre_produccion);
        free(resultados[i]->resumen);
        free(resultados[i]->estado);
        free(resultados[i]);
    }
    free(resultados);
    free(datoLower);
    free(dato);
}

/*
 * Solicita un parametro de búsqueda al usuario
 * Retorna el texto ingresado o NULL si está vacio se omite la busqueda
 */
static char *solicitarParametro(const char *nombre) {
    char *buffer = (char *)malloc(MAX_RUTA * sizeof(char));//reserva memoria para el parametro de busqueda
    if (buffer == NULL) return NULL;

    printf("  %s (dejalo vacio para omitir): ", nombre);
    if (fgets(buffer, MAX_RUTA, stdin) == NULL) {
        free(buffer);
        return NULL;
    }
    buffer[strcspn(buffer, "\r\n")] = 0;

    if (strlen(buffer) == 0) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

/*
 * Realiza una busqueda avanzada interactiva con mltiples parametros
 * Permite elegir: (contiene/exacta), operador (Y/O), y campos (nombre/autor/genero/resumen)
 */
void busquedaAvanzada(void) {
    parametrosBusquedaAvanzada param;
    memset(&param, 0, sizeof(parametrosBusquedaAvanzada));

    printf("\n---------BÚSQUEDA AVANZADA DE EJEMPLARES---------\n");

    // 1. Solicitar tipo de busqueda
    int tipoBusqueda = 0;
    do {
        printf("\nTipo de búsqueda:\n");
        printf("  1. Contiene (busca si está incluido en el texto)\n");
        printf("  2. Exacta (busca coincidencia exacta)\n");
        printf("Opción (1-2): ");
        if (scanf("%d", &tipoBusqueda) != 1) {
            while (getchar() != '\n');
            tipoBusqueda = 0;
        }
    } while (tipoBusqueda < 1 || tipoBusqueda > 2);
    while (getchar() != '\n');

    // pide el operador logico al usuario
    int operador = 0;
    do {
        printf("\nOperador lógico entre campos:\n");
        printf("  1. operador Y (AND) - Todos los campos ingresados deben coincidir\n");
        printf("  2. operador O (OR) - Al menos uno de los campos ingresados debe coincidir\n");
        printf("Opción (1-2): ");
        if (scanf("%d", &operador) != 1) {
            while (getchar() != '\n');
            operador = 0;
        }
    } while (operador < 1 || operador > 2);
    while (getchar() != '\n'); // limpiar buffer

    //pide los parametros de busqueda
    printf("\nIngrese los criterios de búsqueda:\n");
    
    char *nombre = solicitarParametro("Nombre");
    if (nombre != NULL) {
        strcpy(param.nombre, nombre);
        param.modo_nombre = tipoBusqueda;
        free(nombre);
    }

    char *autor = solicitarParametro("Autor");
    if (autor != NULL) {
        strcpy(param.autor, autor);
        param.modo_autor = tipoBusqueda;
        free(autor);
    }

    char *genero = solicitarParametro("Género");
    if (genero != NULL) {
        strcpy(param.genero, genero);
        param.modo_genero = tipoBusqueda;
        free(genero);
    }

    char *resumen = solicitarParametro("Resumen");
    if (resumen != NULL) {
        strcpy(param.resumen, resumen);
        param.modo_resumen = tipoBusqueda;
        free(resumen);
    }

    //valida que aunqeu sea un patametro fue ingresado, al menos uno debe tener contenido
    if (strlen(param.nombre) == 0 && strlen(param.autor) == 0 &&
        strlen(param.genero) == 0 && strlen(param.resumen) == 0) {
        printf("\nDebe ingresar al menos un criterio de búsqueda.\n");
        return;
    }

    //guarda el operador
    param.operador_logico = operador;

    //hace la busqueda
    int total = 0;
    ResultadoBusquedaEjemplar **resultados = buscarEjemplarAvanzado(&param, &total);

    //muestra los resultados
    if (resultados == NULL || total == 0) {
        printf("\n--------------------------------------------------\n");
        printf("No se encontraron ejemplares que coincidan con los criterios.\n");
        printf("--------------------------------------------------\n");
        return;//no se encontraron resultados
    }

    printf("\n--------------------------------------------------\n");
    printf("Se encontraron %d ejemplar(es) coincidentes:\n", total);
    printf("--------------------------------------------------\n");

    for (int i = 0; i < total; i++) {
        printf("[%d]\n", i + 1);
        printf("  Identificador: %s\n", resultados[i]->id_ejemplar);
        printf("  Nombre:        %s\n", resultados[i]->nombre_produccion);
        printf("  Resumen:       %s\n", resultados[i]->resumen);
        printf("--------------------------------------------------\n");
    }

    //libera la memoria de los resultados de la busqueda
    for (int i = 0; i < total; i++) {
        free(resultados[i]->id_ejemplar);
        free(resultados[i]->nombre_produccion);
        free(resultados[i]->resumen);
        free(resultados[i]->estado);
        free(resultados[i]);
    }
    free(resultados);//libera la memoria del arreglo de resultados
}