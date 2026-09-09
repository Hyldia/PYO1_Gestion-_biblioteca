/*
* Tinee los structs(estruturas) de datos compartidos por varios modulos del sistema
* Define el tipo de información que se va a usar en los distinyos modulos
*/

#ifndef TIPOS_H 
#define TIPOS_H

// Definir constantes, tamaño de arreglos, etc.
#define MAX_IDENTIFICACION 7
#define MAX_NOMBRE 100
#define MAX_DIRECCION 100
#define MAX_AUTOR 100
#define MAX_GENERO 50
#define MAX_RESUMEN 500

// Definir estructuras de datos
typedef struct {
    char identificacion[MAX_IDENTIFICACION];
    char nombre[MAX_NOMBRE];
    char direccion[MAX_DIRECCION];
    int activo;
}Usuario;

typedef struct {
    char nombre[MAX_NOMBRE];
    char autor[MAX_AUTOR];
    int anio_publicacion;
    char genero[MAX_GENERO];
    char resumen[MAX_RESUMEN];
    int cantidad;
}Produccion;

#endif