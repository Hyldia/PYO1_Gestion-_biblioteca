#include <stdio.h>

/*E: Lee una opción del menú */
/*R: n/a */
/*S: Devuelve la opción leída o -1 si hay un error */
int leerOpcion(void)
{
    int op;
    printf("\nOpcion: ");
    if (scanf("%d", &op) != 1) {
        while (getchar() != '\n'); /*getchar() para limpiar el buffer */
        return -1; /*para errores*/
    }
    while (getchar() != '\n');
    return op;
}

void menuOperativas(void)
{
    int op;
    do {
        printf("\nOPCIONES OPERATIVAS\n");
        printf("1. Gestion de catalogo\n");
        printf("2. Gestion de usuarios\n");
        printf("3. Historial de prestamos\n");
        printf("4. Vencimiento de prestamos\n");
        printf("5. Estadisticas\n");
        printf("6. Volver\n");
        op = leerOpcion();

        switch (op) {
            case 1:
                printf("\n(pendiente)\n");
                break;
            case 2:
                printf("\n(pendiente)\n");
                break;
            case 3:
                printf("\n(pendiente)\n");
                break;
            case 4:
                printf("\n(pendiente)\n");
                break;
            case 5:
                printf("\n(pendiente)\n");
                break;
            case 6: 
                break;
            default: printf("\nOpcion invalida.\n"); break;
        }
    } while (op != 6);
}

void menuGenerales(void)
{
    int op;
    do {
        printf("\nOPCIONES GENERALES\n");
        printf("1. Busqueda simple\n");
        printf("2. Busqueda avanzada\n");
        printf("3. Prestamo de ejemplares\n");
        printf("4. Devolucion de ejemplares\n");
        printf("5. Volver\n");
        op = leerOpcion();

        switch (op) {
            case 1:
                printf("\n(pendiente)\n");
                break;
            case 2:
                printf("\n(pendiente)\n");
                break;
            case 3:
                printf("\n(pendiente)\n");
                break;
            case 4:
                printf("\n(pendiente)\n");
                break;
            case 5: 
                break;
            default: printf("\nOpcion invalida.\n"); break;
        }
    } while (op != 5);
}

int main(void)
{
    int op;
    do {
        printf("\nMENU PRINCIPAL\n");
        printf("1. Opciones Operativas\n");
        printf("2. Opciones Generales\n");
        printf("3. Salir\n");
        op = leerOpcion();

        switch (op) {
            case 1:
                menuOperativas();
                break;
            case 2:
                menuGenerales(); 
                break;
            case 3:
                printf("\nSaliendo :)\n");
                break;
            default:
                printf("\nOpcion invalida.\n"); break;
        }
    } while (op != 3);

    return 0;
}