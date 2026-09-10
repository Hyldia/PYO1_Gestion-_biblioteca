/*
* Tinee los structs(estruturas) de datos compartidos por varios modulos del sistema
* Define el tipo de información que se va a usar en los distinyos modulos
*/

#ifndef TIPOS_H 
#define TIPOS_H

// Definir constantes, tamaño de arreglos, etc.
#define MAX_IDENTIFICACION 11
#define MAX_NOMBRE 100
#define MAX_DIRECCION 100
#define MAX_AUTOR 100
#define MAX_GENERO 50
#define MAX_RESUMEN 500
#define MAX_RUTA 256
#define MAX_LINEA 1024
#define MAX_CONTENEDOR_USUARIOS 10000
#define MAX_CONTENEDOR_CATALOGO 20000

// Definir estructuras de datos
typedef struct {
    char *identificacion;
    char *nombre;
    char *direccion;
} Usuario;

typedef struct {
    char *nombre;
    char *autor;
    int anio_publicacion;
    char *genero;
    char *resumen;          
    int cantidad;
}Produccion;
#endif