#ifndef BUSQUEDAS_H
#define BUSQUEDAS_H

#include "tipos.h"

/*
 * Realiza una búsqueda simple de ejemplares
 * Busca el dato en nombre, autor o resumen (técnica "contiene")
 * Muestra: identificador, nombre, resumen y estado
 */
void busquedaSimple(void);

/*
 * Realiza una búsqueda avanzada de ejemplares
 * Permite filtrar por multiples criterios
 */
void busquedaAvanzada(void);

#endif
