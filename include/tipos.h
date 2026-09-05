/*
* structs compartidos por varios modulos
*/

#ifndef TIPOS_H 
#define TIPOS_H

// Definir constantes, tamaño de arreglos, etc.
#define MAX_IDENTIFICACION 7
#define MAX_NOMBRE 100
#define MAX_DIRECCION 100

// Definir estructuras de datos
typedef struct {
    char identificacion[MAX_IDENTIFICACION];
    char nombre[MAX_NOMBRE];
    char direccion[MAX_DIRECCION];
    int activo;
}Usuario;

#endif