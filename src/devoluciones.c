/*
* Modulo de devolucion de ejemplares.
* Registra una devolucion: valida usuario, actualiza el estado de los ejemplares
* El acceso a los archivos JSON se hace por medio de persistencia.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "devoluciones.h"
#include "persistencia.h"

/*
* Objetivo: Obtener la fecha actual del sistema.
* Entradas: Ninguna
* Salidas: Puntero dinamico con la fecha en formato YYYY-MM-DD.
* Restricciones: El que llama debe liberar la memoria con free().
*/
static char *obtenerFecha(void){
    char *fechaActual = malloc(11);
    if(fechaActual == NULL){
        return NULL;
    }
    time_t tiempoActual = time(NULL); // obtiene fecha y hora actual

    struct tm *fechaLocal = localtime(&tiempoActual); // convierte el tiempo a una estructura con año, mes y día
    strftime(fechaActual, 11, "%y-%m-%d", fechaLocal); // genera la cadena con el formato YYYY-MM-DD
    return fechaActual;
}

/*
* Objetivo: Registrar la devolucion de un prestamo
* Entrada: Id del prestamos que se desea cancelar
* Salida: 
    - Cambia el estado del prestamo a finalizado
    - Libera los ejemplres asociados
    - Registra la fecha de devolucion.
    - Muestra mensajes de confirmacion o error.
*/
void registrarDevolucion(void) {
    int idPrestamo;
    printf("\n========== DEVOLUCION ==========\n");
    printf("Ingrese el ID del prestamo a devolver:");
    if (scanf("%d", &idPrestamo) != 1){
        printf("El ID ingresado es invalido.\n");
        while(getchar() != '\n'); // limpia caracteres sobrantes del buffer
        return;
    }
    while(getchar() != '\n');
    if (!existePrestamoActivoJSON(idPrestamo)){
        printf("--------------------------------\n"),
        printf("No existe un prestamo activo con ese ID.\n");
        printf("--------------------------------\n");
        return;
    }
    char *fecha = obtenerFecha();
    if(fecha == NULL){
        printf("Error al obtener la fecha.\n");
        return;
    }

    if(finalizarPrestamoJSON(idPrestamo, fecha)){
        printf("---------------------------------\n");
        printf("Devolucion registrada con exito.\n");
        printf("Fecha devolucion: %s\n", fecha);
        printf("---------------------------------\n");
    }else{
        printf("---------------------------------\n");
        printf("Error al registrar la devolucion.\n");
        printf("---------------------------------\n");
    }
    free(fecha);
}