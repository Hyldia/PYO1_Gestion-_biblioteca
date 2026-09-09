/*
* Inicio del programa
*/

//Menu provisional 

#include <stdio.h>
#include "usuarios.h"

int main() {

    int opcion;

    do {

        printf("\n=== USUARIOS ===\n");
        printf("1. Agregar usuario\n");
        printf("2. Listar usuarios\n");
        printf("3. Modificar usuario\n");
        printf("4. Eliminar usuario\n");
        printf("0. Salir\n");

        printf("Opcion: ");
        scanf("%d", &opcion);

        getchar();

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
        }

    } while(opcion != 0);

    return 0;
}