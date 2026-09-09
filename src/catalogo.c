#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cjson/cJSON.h>

#include "catalogo.h"
#include "persistencia.h"
#include "tipos.h"

/*
 * Verifica si un campo de texto contiene al menos un caracter visible.
 * Retorna:
 * 1 Si el campo tiene texto valido.
 * 0 Si es nulo, vacio o solo tiene espacios en blanco.
 */
int validarCampoNoVacio(const char *texto) {
    if (texto == NULL || *texto == '\0') return 0;
    while (*texto != '\0') {
        if (!isspace((unsigned char)*texto)) return 1;
        texto++;
    }
    return 0;
}

/*
 * Extrae tokens considerando campos vacios entre numerales (##).
 */
static char* extraerElemento(char **cadena, const char *delimitador) {
    if (*cadena == NULL) return NULL;
    char *inicio = *cadena;
    char *p = strpbrk(inicio, delimitador);
    if (p) {
        *p = '\0';
        *cadena = p + 1;
    } else {
        *cadena = NULL;
    }
    return inicio;
}

/*
*agrega los lotes de libros al catalogo
*/
void agregarLote(void) {
    char ruta[256];
    printf("Ingrese la ruta del archivo de texto plano a procesar: ");
    if (fgets(ruta, sizeof(ruta), stdin) == NULL) return;
    ruta[strcspn(ruta, "\r\n")] = 0;

    FILE *archivo = fopen(ruta, "r");
    if (!archivo) { 
        printf("--------------------------------------------------\n");
        printf("Error: No se pudo abrir el archivo en la ruta: %s\n", ruta);
        printf("--------------------------------------------------\n");
        return;
    }

    char linea[1024];
    int procesados = 0;
    int no_procesados = 0;
    int numero_linea = 0;

    printf("\n--- Procesando Archivo de Lote ---\n");

    while (fgets(linea, sizeof(linea), archivo)) {
        numero_linea++;
        linea[strcspn(linea, "\r\n")] = 0; // Limpiar saltos de linea Windows/Linux

        if (!validarCampoNoVacio(linea)) continue;//si la linea esta 100% vacia se ignora

        Produccion prod;
        memset(&prod, 0, sizeof(Produccion));// Inicializar la estructura de producción

        char *cursor = linea;
        char *token = NULL;

        //valida Nombre
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] Falta el campo 'Nombre' por completar.\n", numero_linea);
            no_procesados++;
            continue;
        }
        snprintf(prod.nombre, sizeof(prod.nombre), "%s", token);

        //valida que no haya repetidos/unicidad(el nombre da la unicidad)
        if (existeProduccionJSON(prod.nombre)) {
            printf("[OMITIDO - Línea %d] La producción '%s' ya existe en el catálogo.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }

        //valida autor
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Autor' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        snprintf(prod.autor, sizeof(prod.autor), "%s", token);

        //valida Año de publicacion del libro
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Año de publicación' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        prod.anio_publicacion = atoi(token);

        //valida genero del libro
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Género' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        snprintf(prod.genero, sizeof(prod.genero), "%s", token);

        //valida resumen
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Resumen' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        snprintf(prod.resumen, sizeof(prod.resumen), "%s", token);

        //Valida cantidad
        token = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(token)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Cantidad' por completar.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }
        prod.cantidad = atoi(token);

        if (prod.cantidad <= 0) {
            printf("[OMITIDO - Línea %d] En '%s': la cantidad debe ser mayor a 0.\n", numero_linea, prod.nombre);
            no_procesados++;
            continue;
        }

        guardarProduccionJSON(prod);
        generarEjemplaresJSON(prod.nombre, prod.cantidad);

        procesados++;
    }

    fclose(archivo);

    printf("\n==================================================\n");
    printf("Carga finalizada.\n");
    printf("Registros procesados con éxito: %d\n", procesados);
    printf("Registros no procesados/omitidos: %d\n", no_procesados);
    printf("==================================================\n");
}


/*
*muestra todos las producciones del catalogo y sus detalles
*/
void mostrarCatalogo(void) {
    FILE *archivo = fopen("data/catalogo.json", "r");// Abrir el archivo JSON en modo lectura
    //valida que el archivo exista o no tenga un error de ruta
    if (archivo == NULL) {// Si el archivo JSON no existe, mostrar un mensaje de error y salir
        printf("\n--------------------------------------------------\n");
        printf("No hay producciones registradas en el catálogo.\n");
        printf("--------------------------------------------------\n");
        return;
    }

    char contenedor[20000]; // Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) - 1, archivo);//leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0';//Agregar un caracter nulo al final del buffer
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenedor);// Convertir el contenido del contenedor a un objeto JSON
    //valida que el archivo no este vacio o tenga un error de lectura
    if (raiz == NULL) {// Si el archivo JSON no existe o esta vacio, meustra un mensaje de error y se sale
        printf("\n--------------------------------------------------\n");
        printf("Error al leer el archivo de catálogo o está vacío.\n");
        printf("--------------------------------------------------\n");
        return;
    }

    int cantidad = cJSON_GetArraySize(raiz);// Obtener la cantidad de producciones en el array de producciones
    //valida que el catalogo no este vacio
    if (cantidad == 0) {//si es = a 0 retorna que el catalogo esta vacio
        printf("\n--------------------------------------------------\n");
        printf("El catálogo está vacío.\n");
        printf("--------------------------------------------------\n");
        cJSON_Delete(raiz);
        return;
    }

    printf("\n==================================================\n");
    printf("              CATÁLOGO DE PRODUCCIONES            \n");
    printf("==================================================\n");

    for (int i = 0; i < cantidad; i++) {//Recorre sobre cada produccion en el array de producciones
        cJSON *prod = cJSON_GetArrayItem(raiz, i);

        cJSON *nombre = cJSON_GetObjectItem(prod, "nombre");
        cJSON *autor = cJSON_GetObjectItem(prod, "autor");
        cJSON *anio = cJSON_GetObjectItem(prod, "anio_publicacion");
        cJSON *genero = cJSON_GetObjectItem(prod, "genero");
        cJSON *resumen = cJSON_GetObjectItem(prod, "resumen");
        cJSON *cant = cJSON_GetObjectItem(prod, "cantidad");

        printf("Título:     %s\n", nombre ? nombre->valuestring : "N/A");//Imprime el nombre de la produccion o "N/A" si es nulo
        printf("Autor:      %s\n", autor ? autor->valuestring : "N/A");//Imprime el autor de la produccion o "N/A" si es nulo
        printf("Año:        %d\n", anio ? anio->valueint : 0);//Imprime el año de la produccion o 0 si es nulo
        printf("Género:     %s\n", genero ? genero->valuestring : "N/A");//Imprime el genero de la produccion o "N/A" si es nulo
        printf("Cantidad:   %d ejemplares\n", cant ? cant->valueint : 0);//Imprime la cantidad de ejemplares de la produccion o 0 si es nulo
        printf("Resumen:    %s\n", resumen ? resumen->valuestring : "N/A");//Imprime el resumen de la produccion o "N/A" si es nulo
        printf("--------------------------------------------------\n");
    }

    cJSON_Delete(raiz);
}

/*
*submenu:catalogo
*/
void menuCatalogo(void) {
    int opcion;
    while (1) {
        printf("\n--- Menú Catálogo ---\n");
        printf("1. Agregar catálogo por lote\n");
        printf("2. Mostrar catálogo\n");
        printf("0. Volver\n");
        printf("Seleccione una opción: ");
        
        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n'); // Limpiar buffer
            printf("Opción inválida.\n");
            continue;
        }
        while (getchar() != '\n'); // Limpiar buffer
        
        switch (opcion) {
            case 1:
                agregarLote();
                break;
            case 2:
                mostrarCatalogo();
                break;
            case 0:
                return;
            default:
                printf("Opción inválida.\n");
        }
    }
}

