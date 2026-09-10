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
    if (texto == NULL || *texto == '\0') 
        return 0;
    while (*texto != '\0') {
        if (!isspace((unsigned char)*texto)) 
            return 1;//si encuentra un caracter visible retorna 1
        texto++;//avanza al siguiente caracter
    }
    return 0;
}

/*
 * Extrae elementos considerando campos vacios entre numerales (##).
 */
static char* extraerElemento(char **cadena, const char *delimitador) {
    if (*cadena == NULL) //si la cadena es nula retorna nulo
        return NULL;
    char *inicio = *cadena;//puntero al inicio de la cadena
    char *p = strpbrk(inicio, delimitador);//busca el primer delimitador en la cadena
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
    char *ruta = (char *)malloc(MAX_RUTA * sizeof(char));// Para almacenar la ruta del archivo de texto plano
    if (ruta == NULL) {
        printf("Error: No se pudo asignar memoria para la ruta.\n");
        return;//si no se puede asignar memoria para la ruta retorna
    }
    
    printf("Ingrese la ruta del archivo de texto plano a procesar: ");
    if (fgets(ruta, MAX_RUTA, stdin) == NULL) {
        free(ruta);
        return;//si no se puede leer la ruta retorna
    }
    ruta[strcspn(ruta, "\r\n")] = 0;// Limpiar saltos de linea

    FILE *archivo = fopen(ruta, "r");// Abrir el archivo de texto plano en modo lectura
    if (!archivo) { 
        printf("--------------------------------------------------\n");
        printf("Error: No se pudo abrir el archivo en la ruta: %s\n", ruta);
        printf("--------------------------------------------------\n");
        free(ruta);
        return;//si no se puede abrir el archivo retorna
    }

    char *linea = (char *)malloc(MAX_LINEA * sizeof(char));
    if (linea == NULL) {
        printf("Error: No se pudo asignar memoria para la línea.\n");
        fclose(archivo);
        free(ruta);
        return;//si no se puede asignar memoria para la linea retorna
    }
    
    int procesados = 0;//cantidad de registros procesados exitosamente
    int no_procesados = 0;//cantidad de registros no procesados por errores de validacion
    int numero_linea = 0;//numero de linea actual del archivo de texto plano

    printf("\n--- Procesando Archivo de Lote ---\n");

    while (fgets(linea, MAX_LINEA, archivo)) {
        numero_linea++;
        linea[strcspn(linea, "\r\n")] = 0; // Limpiar saltos de linea Windows/Linux

        if (!validarCampoNoVacio(linea)) continue;//si la linea esta 100% vacia se ignora

        Produccion prod;//estructura para almacenar los datos de la produccion
        prod.nombre = NULL;//inicializa los punteros a NULL para evitar errores de memoria
        prod.autor = NULL;
        prod.genero = NULL;
        prod.resumen = NULL;

        char *cursor = linea;//puntero para recorrer la linea y extraer los elementos
        char *elemento = NULL;//puntero para almacenar cada elemento extraido

        //valida Nombre
        elemento = extraerElemento(&cursor, "#");//extrae el primer elemento de la linea hasta el primer # o hasta el final de la linea
        if (!validarCampoNoVacio(elemento)) {
            printf("[OMITIDO - Línea %d] Falta el campo 'Nombre' por completar.\n", numero_linea);
            no_procesados++;
            continue;
        }
        prod.nombre = (char *)malloc(strlen(elemento) + 1);//reserva memoria para el nombre
        strcpy(prod.nombre, elemento);//copia el nombre a la estructura

        //valida que no haya repetidos/unicidad(el nombre da la unicidad)
        if (existeProduccionJSON(prod.nombre)) {
            printf("[OMITIDO - Línea %d] La producción '%s' ya existe en el catálogo.\n", numero_linea, prod.nombre);
            free(prod.nombre);//libera la memoria del nombre
            no_procesados++;
            continue;
        }

        //valida autor
        elemento = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(elemento)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Autor' por completar.\n", numero_linea, prod.nombre);
            free(prod.nombre);//libera la memoria del nombre
            no_procesados++;
            continue;
        }
        prod.autor = (char *)malloc(strlen(elemento) + 1);//reserva memoria para el autor
        strcpy(prod.autor, elemento);

        //valida Año de publicacion del libro
        elemento = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(elemento)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Año de publicación' por completar.\n", numero_linea, prod.nombre);
            free(prod.nombre);
            free(prod.autor);//libera la memoria del nombre y autor
            no_procesados++;
            continue;
        }
        prod.anio_publicacion = atoi(elemento);

        //valida genero del libro
        elemento = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(elemento)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Género' por completar.\n", numero_linea, prod.nombre);
            free(prod.nombre);
            free(prod.autor);
            no_procesados++;
            continue;
        }
        prod.genero = (char *)malloc(strlen(elemento) + 1);//reserva memoria para el genero
        strcpy(prod.genero, elemento);

        //valida resumen
        elemento = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(elemento)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Resumen' por completar.\n", numero_linea, prod.nombre);
            free(prod.nombre);
            free(prod.autor);
            free(prod.genero);
            no_procesados++;
            continue;
        }
        prod.resumen = (char *)malloc(strlen(elemento) + 1);//reserva memoria para el resumen
        strcpy(prod.resumen, elemento);

        //Valida cantidad
        elemento = extraerElemento(&cursor, "#");
        if (!validarCampoNoVacio(elemento)) {
            printf("[OMITIDO - Línea %d] En '%s': falta el campo 'Cantidad' por completar.\n", numero_linea, prod.nombre);
            free(prod.nombre);
            free(prod.autor);
            free(prod.genero);
            free(prod.resumen);
            no_procesados++;
            continue;
        }
        prod.cantidad = atoi(elemento);//convierte el elemento a entero para la cantidad de ejemplares

        if (prod.cantidad <= 0) {
            printf("[OMITIDO - Línea %d] En '%s': la cantidad debe ser mayor a 0.\n", numero_linea, prod.nombre);
            free(prod.nombre);
            free(prod.autor);
            free(prod.genero);
            free(prod.resumen);
            no_procesados++;
            continue;
        }

        guardarProduccionJSON(prod);
        generarEjemplaresJSON(prod.nombre, prod.cantidad);

        //libera la memoria de los campos de la estructura prod
        free(prod.nombre);
        free(prod.autor);
        free(prod.genero);
        free(prod.resumen);

        procesados++;
    }

    fclose(archivo);

    printf("\n==================================================\n");
    printf("Carga finalizada.\n");
    printf("Registros procesados con éxito: %d\n", procesados);
    printf("Registros no procesados/omitidos: %d\n", no_procesados);
    printf("==================================================\n");
    
    free(linea);
    free(ruta);
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

    char *contenedor = (char *)malloc(MAX_CONTENEDOR_CATALOGO * sizeof(char));
    if (contenedor == NULL) {
        printf("Error: No se pudo asignar memoria para el contenedor.\n");
        fclose(archivo);
        return;//si no se puede asignar memoria para el contenedor retorna
    }
    
    size_t largoArchivo = fread(contenedor, 1, MAX_CONTENEDOR_CATALOGO - 1, archivo);// Leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0';
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenedor);// Convertir el contenido del contenedor a un objeto JSON
    //valida que el archivo no este vacio o tenga un error de lectura
    if (raiz == NULL) {// Si el archivo JSON no existe o esta vacio, meustra un mensaje de error y se sale
        printf("\n--------------------------------------------------\n");
        printf("Error al leer el archivo de catálogo o está vacío.\n");
        printf("--------------------------------------------------\n");
        free(contenedor);
        return;
    }

    int cantidad = cJSON_GetArraySize(raiz);// Obtener la cantidad de producciones en el array de producciones
    //valida que el catalogo no este vacio
    if (cantidad == 0) {//si es = a 0 retorna que el catalogo esta vacio
        printf("\n--------------------------------------------------\n");
        printf("El catálogo está vacío.\n");
        printf("--------------------------------------------------\n");
        cJSON_Delete(raiz);
        free(contenedor);
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
    free(contenedor);
}


