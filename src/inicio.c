/*
* Inicio del programa
*/

//Menu provisional 

#include <stdio.h>
//#include "usuarios.h"
#include "catalogo.h"

//int main() {

   // int opcion;

  //  do {

     //   printf("\n=== USUARIOS ===\n");
      //  printf("1. Agregar usuario\n");
     //   printf("2. Listar usuarios\n");
     //   printf("3. Modificar usuario\n");
    //    printf("4. Eliminar usuario\n");
     //   printf("0. Salir\n");

      //  printf("Opcion: ");
    //    scanf("%d", &opcion);

     //   getchar();

     //   switch(opcion) {
    //        case 1:
   //             agregarUsuario();
    //            break;
   //         case 2:
    //            mostrarUsuarios();
    //            break;
   //         case 3:
 //               modificarUsuario();
 //               break;
 //           case 4:
//                eliminarUsuario();
//        }

//    } while(opcion != 0);

//    return 0;
//}

/*
* Menú de Gestión de Catálogo
*/

//menu temporal para el catálogo
int main() {
    int opcion;

    do {
        printf("\n=== GESTIÓN DE CATÁLOGO ===\n");
        printf("1. Agregar libros en lote\n");
        printf("2. Ver todo el catálogo\n");
        printf("0. Volver al menú principal\n");

        printf("Opción: ");
        if (scanf("%d", &opcion) != 1) {
            // Limpia el búfer en caso de entrada no numérica
            while (getchar() != '\n');
            opcion = -1;
            continue;
        }

        getchar(); // Limpiar el salto de línea sobrante

        switch (opcion) {
            case 1:
                agregarLote();
                break;
            case 2:
                mostrarCatalogo();
                break;
            case 0:
                printf("Regresando al menú principal...\n");
                break;
            default:
                printf("Opción inválida. Intente de nuevo.\n");
                break;
        }

    } while (opcion != 0);
}