/* Inicio del programa*/

#include <stdio.h>
#include "prestamos.h"
#include "usuarios.h"
#include "catalogo.h"
#include "devoluciones.h"
#include "busquedas.h"

/*
* Objetivo: leer un numero de opcion de la consola y limpiar el resto de la linea.
* Entradas: ninguna (lee de stdin).
* Salidas:  el numero ingresado, o -1 si lo ingresado no es un numero.
*/
static int leerOpcion(void) {
    int op;
    if (scanf("%d", &op) != 1) {
        op = -1;
    }
    while (getchar() != '\n' && !feof(stdin)) { }   // limpiar el buffer
    return op;
}

/* Menú de Gestión de Usuarios */
static void menuUsuarios(void) {
    int opcion;
    do {
        printf("\n=== USUARIOS ===\n");
        printf("1. Agregar usuario\n");
        printf("2. Listar usuarios\n");
        printf("3. Modificar usuario\n");
        printf("4. Eliminar usuario\n");
        printf("0. Salir\n");
        printf("Opcion: "); 
        opcion = leerOpcion();

        switch(opcion) {
            case 1:
                agregarUsuario();
                break;
            case 2:
                mostrarUsuarios();
                break;
            case 3:
                modificarUsuario();
                break;
            case 4:
                eliminarUsuario();
                break;
            case 0:
                printf("Saliendo del menu de usuarios...\n");
                break;
            default:
                printf("Opcion invalida. Intente de nuevo.\n");
        }

    } while(opcion != 0);
}

/* Opciones Generales.*/
static void menuGenerales(void) {
    int opcion;
    do {
        printf("\n===== OPCIONES GENERALES =====\n");
        printf("1. Busqueda simple\n");
        printf("2. Busqueda avanzada\n");
        printf("3. Prestamo de ejemplares\n");
        printf("4. Devolucion de ejemplares\n");
        printf("0. Volver al menu principal\n");
        printf("Opcion: ");
        opcion = leerOpcion();

        switch (opcion) {
            case 1: 
                busquedaSimple();
                break;
            case 2: 
                printf("\n(Pendiente)\n");
                break;
            case 3:
                registrarPrestamo();
                break;
            case 4:
                registrarDevolucion();
                break;
            case 0: 
                printf("Saliendo del menu de opciones generales...\n");
                break;
            default: printf("Opcion invalida.\n");
        }
    } while (opcion != 0);
}
/* Submenu: gestion de catalogo */
static void menuCatalogo(void) {
    int opcion;
    do {
        printf("\n===== GESTION DE CATALOGO =====\n");
        printf("1. Agregar libros en lote\n");
        printf("2. Ver todo el catalogo\n");
        printf("0. Volver\n");
        printf("Opcion: ");
        opcion = leerOpcion();

        switch (opcion) {
            case 1: agregarLote();     break;
            case 2: mostrarCatalogo(); break;
            case 0: break;
            default: printf("Opcion invalida.\n");
        }
    } while (opcion != 0);
}

/* Opciones Operativas. */
static void menuOperativas(void) {
    int opcion;
    do {
        printf("\n===== OPCIONES OPERATIVAS =====\n");
        printf("1. Gestion de catalogo\n");
        printf("2. Gestion de usuarios\n");
        printf("3. Historial de prestamos\n");
        printf("4. Vencimiento de prestamos\n");
        printf("5. Estadisticas\n");
        printf("0. Volver al menu principal\n");
        printf("Opcion: ");
        opcion = leerOpcion();

        switch (opcion) {
            case 1:
                menuCatalogo(); // definida en catalogo.c
                break;
            case 2:
                menuUsuarios();
                break;
            case 3:
                int sub;
                printf("\n1. Ver todos\n");
                printf("2. Ver por rango de fechas\n");
                printf("0. Volver\n");
                printf("Opcion: ");
                sub = leerOpcion();

                if (sub == 1) {
                    mostrarTodosLosPrestamos();
                } else if (sub == 2) {
                    mostrarHistorialPrestamos();
                } else if (sub == 0) {
                    printf("Volviendo al menu de opciones operativas...\n");
                    break;
                } else {
                    printf("Opcion invalida.\n");
                }
                break;
            case 4:
                printf("\n(Pendiente)\n");
                break;
            case 5:
                printf("\n(Pendiente)\n");
                break;
            case 0:
                printf("Volviendo al menu principal...\n");
                break;
            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 0);
}

/*menu principal del sistema.
* 0 al terminar el programa.
*/
int main(void) {
    int opcion;
    do {
        printf("\n===MENU PRINCIPAL ====\n");
        printf("1. Opciones Operativas\n");
        printf("2. Opciones Generales\n");
        printf("0. Salir\n");
        printf("Opcion: ");
        opcion = leerOpcion();

        switch (opcion) {
            case 1:
                menuOperativas();
                break;
            case 2:
                menuGenerales();
                break;
            case 0:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 0);

    return 0;
}
