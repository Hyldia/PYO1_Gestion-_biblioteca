#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    int total = 0;
    ResultadoBusquedaEjemplar **resultados = buscarEjemplarSimple(dato, &total);//llama a la funcion buscarEjemplarSimple para realizar la busqueda y obtener los resultados

    if (resultados == NULL || total == 0) {
        printf("\n--------------------------------------------------\n");
        printf("No se encontraron ejemplares coincidentes con '%s'.\n", dato);
        printf("--------------------------------------------------\n");
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
    free(dato);
}