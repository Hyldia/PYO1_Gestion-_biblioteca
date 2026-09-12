/*
* Se declaran las funciones necesarias para guardar y cargar la informacion de los archivos JSON
*/

#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include <stdio.h>
#include "tipos.h"

void guardarUsuariosJSON(Usuario usuario);
void mostrarUsuariosJSON();
int existeIdentificacionJSON(const char *identificacion);
int modificarUsuarioJSON(Usuario usuario);
int eliminarUsuarioJSON(const char *identificacion);
int tieneRegistrosAsociados(const char *identificacion);  

//catalogo y ejemolares
int existeProduccionJSON(const char *nombre);
void guardarProduccionJSON(Produccion prod);
void generarEjemplaresJSON(const char *nombre, int cantidad);

int  existeEjemplarJSON(const char *idEjemplar);
int  ejemplarDisponibleJSON(const char *idEjemplar, const char *fInicio, const char *fFin);
int  siguienteIdPrestamoJSON(void);
void guardarPrestamoJSON(Prestamo prestamo);
void cambiarEstadoEjemplarJSON(const char *idEjemplar, const char *nuevoEstado);
char *obtenerProduccionEjemplarJSON(const char *idEjemplar);

// Devoluciones
int existePrestamoActivoJSON(int idPrestamo);
int finalizarPrestamoJSON(int idPrestamo, const char *fechaDevoucion, int monto);

// Fechas para tarifa devolucion
int obtenerFechasPrestamoJSON(int idPrestamo, char *fechaInicio, char *fechaEntrega);

char *leerArchivoCompleto(FILE *archivo);

char *obtenerGeneroProduccionJSON(const char *nombre);

#endif