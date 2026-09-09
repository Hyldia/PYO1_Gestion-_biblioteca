/*
* Se declaran las funciones necesarias para guardar y cargar la informacion de los archivos JSON
*/

#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H
#include "tipos.h"

void guardarUsuariosJSON(Usuario usuario);
void mostrarUsuariosJSON();
int existeIdentificacionJSON(const char *identificacion);
int modificarUsuarioJSON(Usuario usuario);
int eliminarUsuarioJSON(const char *identificacion);
int tieneRegistrosAsociados(const char *identificacion);  

#endif