/*
* funciones necesarias para guardar y cargar JSON
*/

#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H
#include "tipos.h"

void guardarUsuariosJSON(Usuario usuario);
void mostrarUsuariosJSON();
int existeIdentificacionJSON(const char *identificacion);

#endif