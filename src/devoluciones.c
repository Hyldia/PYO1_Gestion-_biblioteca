/*
* Modulo de devolucion de ejemplares.
* Registra una devolucion: valida usuario, actualiza el estado de los ejemplares
* El acceso a los archivos JSON se hace por medio de persistencia.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
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
    strftime(fechaActual, 11, "%Y-%m-%d", fechaLocal); // genera la cadena con el formato YYYY-MM-DD
    return fechaActual;
}

/*
* Objetivo: Convertir una fecha en una cantidad aproximada de días.
* Entradas: anio, mes y dia.
* Salidas: Valor acumulado de días.
*/
static int convertirADias(int anio, int mes, int dia){
    return (anio * 365) + (mes * 30) + dia;
}

/*
* Objetivo: Calcular la diferencia en días entre dos fechas.
* Entradas: fechaInicio y fechaFin en formato YYYY-MM-DD.
* Salidas: Cantidad de días entre ambas fechas.
*/
static int diferenciaDias(const char *fechaInicio, const char *fechaFin){
    int anioInicio;
    int mesInicio;
    int diaInicio;
    int anioFin;
    int mesFin;
    int diaFin;
    sscanf(fechaInicio, "%d-%d-%d", &anioInicio, &mesInicio, &diaInicio);
    sscanf(fechaFin, "%d-%d-%d", &anioFin, &mesFin, &diaFin);

    int totalInicio = convertirADias(anioInicio, mesInicio, diaInicio);
    int totalFin = convertirADias(anioFin, mesFin, diaFin);
    return totalFin - totalInicio;
}

/*
*Objetivo: Obtener las tarifas segun la duración el prestamo.
* Entradas: diasPrestamo.
* Salidas: TarifaPrest*mo y tarifaTardia.
*/
static void obtenerTarifas(int diasPrestao, int *tarifaPrestamo, int *tarifaTardia){
    if(diasPrestao <= 7){
        *tarifaPrestamo = 175;
        *tarifaTardia = 180;
    }else if(diasPrestao <= 15){
        *tarifaPrestamo = 150;
        *tarifaTardia = 75;
    }else{
        *tarifaPrestamo = 100;
        *tarifaTardia = 50;
    }
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
    char *fechaInicio = malloc(11);
    char *fechaEntrega = malloc(11);
    if(fechaInicio == NULL || fechaEntrega == NULL){
        free(fechaInicio);
        free(fechaEntrega);
        free(fecha); // fecha de devolucion
        return;
    }
    if(!obtenerFechasPrestamoJSON(idPrestamo, fechaInicio, fechaEntrega)){
        printf("No se logro recuperar la informacion del prestamo.\n");
        free(fechaInicio);
        free(fechaEntrega);
        free(fecha); // fecha de devolucion
        return;
    }

    // Calcula la duracion original del prestamo.
    int diasPrestamo =diferenciaDias(fechaInicio, fechaEntrega);
    // Calcula cuantos dias tarde se devolvio.
    int diasTardia = diferenciaDias(fechaEntrega, fecha);
    // Si no hubo atraso, la multa es cero.
    if(diasTardia < 0){
        diasTardia = 0;
    }
    int tarifaPrestamo;
    int tarifaTardia;
    // Obtiene las tarifas correspondientes.
    obtenerTarifas(diasPrestamo, &tarifaPrestamo, &tarifaTardia);
    // Calcula el costo base del prestamo.
    int montoPrestamo = diasPrestamo * tarifaPrestamo;
    // Calcula la multa por atraso.
    int multa = diasTardia * tarifaTardia;
    // Calcula el monto total a cancelar.
    int total = montoPrestamo + multa;

    printf("\n");
    printf("===================================\n");
    printf("COMPROBANTE DE DEVOLUCION\n");
    printf("===================================\n");

    printf("ID Prestamo: %d\n", idPrestamo);
    printf("Fecha inicio: %s\n", fechaInicio);
    printf("Fecha entrega: %s\n", fechaEntrega);
    printf("Fecha devolucion: %s\n", fecha);
    printf("Duracion del prestamo: %d dias\n",diasPrestamo);
    printf("Tarifa diaria: %d\n", tarifaPrestamo);
    printf("Monto prestamo: %d\n", montoPrestamo);
    printf("Dias de atraso: %d\n",diasTardia);
    printf("Multa: %d\n", multa);
    printf("TOTAL A CANCELAR: %d\n", total);
    printf(
        "===================================\n"
    );

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
    free(fechaInicio);
    free(fechaEntrega);
    free(fecha); // fecha de devolucion
}