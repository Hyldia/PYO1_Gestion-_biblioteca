/*
* Lee y escribe los datos en JSON
* convierte los structs a JSON y viceversa
* Encapsula la persistencia de datos
* Busca y modifica la información almacenada en os JSON
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "persistencia.h"
#include <cjson/cJSON.h>

/*
 * ---------------------------Usuarios---------------------------
 */


/*
 * Convierte la estructura Usuario en un objeto JSON y la guarda dentro del archivo usuarios.json.
 */
void guardarUsuariosJSON(Usuario usuario){
    FILE *archivo; //Puntero al archivo JSON
    char contenedor[10000]; // Para almacenar el JSON
    cJSON *raiz = NULL; //Puntero a la raiz del JSON

    archivo = fopen("data/usuarios.json", "r"); //Abrir el archivo JSON en modo lectura
    if (archivo != NULL) {
        size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) -1, archivo); //Leer el contenido del archivo JSON
        contenedor[largoArchivo] = '\0'; //Agregar un caracter nulo al final del buffer
        fclose(archivo); //Cerrar el archivo JSON
        raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    }
    if(raiz == NULL){
        raiz = cJSON_CreateArray(); //Si el archivo JSON no existe, crear un nuevo objeto JSON
    }
    cJSON *nuevoUsuario = cJSON_CreateObject(); //Crear un nuevo objeto JSON para el usuario
    cJSON_AddStringToObject(nuevoUsuario, "Identificacion", usuario.identificacion); //Agregar el id del usuario al objeto JSON
    cJSON_AddStringToObject(nuevoUsuario, "nombre", usuario.nombre); //Agregar el nombre del usuario al objeto JSON
    cJSON_AddStringToObject(nuevoUsuario, "Direccion", usuario.direccion); //Agregar el direccion del usuario al objeto JSON
    cJSON_AddNumberToObject(nuevoUsuario, "activo", usuario.activo); //Agregar el estado del usuario al objeto JSON
    cJSON_AddItemToArray(raiz, nuevoUsuario); // Agregar el nuevo usuario al array de usuarios

    char *jsonString = cJSON_Print(raiz); // Convertir el objeto JSON a una cadena de caracteres

    //Guardar en el archivo
    archivo = fopen("data/usuarios.json", "w"); // brir el archivo JSON en modo escritura
    if(archivo != NULL){
        fprintf(archivo, "%s", jsonString); // Escribir la cadena de caracteres en el archivo JSON
        fclose(archivo); // Cerrar el archivo JSON
    }
    free(jsonString); // Liberar la memoria de la cadena de caracteres
    cJSON_Delete(raiz); // Liberar la memoria del objeto JSON
    printf("-----------------------------\n");
    printf("Usuario guardado con exito\n"); 
}

/*
 * Lee el archivo usuarios.json y muestra todos los usuarios registrados.
 */
void mostrarUsuariosJSON(){
    FILE *archivo = fopen("data/usuarios.json", "r"); //Abrir el archivo JSON en modo lectura
    if(archivo == NULL){
        printf("No hay usuarios registados.\n");
        return;
    }
    char contenedor[10000]; //Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) -1, archivo); //Leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0'; //Agregar un caracter nulo al final del buffer
    fclose(archivo); //Cerrar el archivo JSON

    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    if(raiz == NULL){
        return; //Si el archivo JSON no existe, salir de la función
    }
    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuario = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        printf("\n---------------------------------\n");
        printf("Identificacion: %s\n", cJSON_GetObjectItem(usuario, "identificacion")->valuestring); //Imprimir el id del usuario
        printf("Nombre: %s\n", cJSON_GetObjectItem(usuario, "nombre")->valuestring); //Imprimir el nombre del usuario
        printf("Direccion: %s\n", cJSON_GetObjectItem(usuario, "direccion")->valuestring); //Imprimir el correo del usuario
        printf("---------------------------------\n");
    }
    cJSON_Delete(raiz); //Liberar la memoria del objeto JSON
}

/*
 * Busca un usuario en usuarios.json usando la identificacion
 
 * Si encuentra el usuario:
 * - Actualiza el nombre.
 * - Actualiza la dirección.
 * - Guarda nuevamente el archivo JSON.

 * Retorna:
 * 1 Si el usuario se modificado correctamente.
 * 0 Si el suario no se encontro o error al leer el archivo.
 */
int modificarUsuarioJSON(Usuario usuario){
    FILE *archivo =fopen("data/usuarios.json","r"); //Abrir el archivo JSON en modo lectura
    if(archivo == NULL){
        return 0;
    }
    char contenedor[10000]; //Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) -1, archivo); //Leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0'; //Agregar un caracter nulo al final del buffer
    fclose(archivo); //Cerrar el archivo JSON
    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    if(raiz == NULL) {
        return 0;
    }
    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuarioJSON = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id = cJSON_GetObjectItem(usuarioJSON, "identificacion"); // Obtener la identifación del usuario
        // Comprara la identifiacion ingresada con las almacenadas
        if(strcmp(id->valuestring, usuario.identificacion) == 0){
            cJSON_ReplaceItemInObject(usuarioJSON, "direccion", cJSON_CreateString(usuario.direccion)); //Borra la dirección antigua que estaba guardada en el JSON y la cambia por la ingresada.

            char *jsonString = cJSON_Print(raiz); //Convertir el objeto JSON a una cadena de caracteres

            //Guardar en el archivo
            archivo = fopen("data/usuarios.json", "w"); //Abrir el archivo JSON en modo escritura
            fprintf(archivo, "%s", jsonString); //Escribir la cadena de caracteres en el archivo JSON
            fclose(archivo); // Cerrar el archivo JSON
            free(jsonString); // Liberar la memoria de la cadena de caracteres
            cJSON_Delete(raiz); // Liberar la memoria del objeto JSON
            return 1;
        }
    }
    cJSON_Delete(raiz);
    return 0;
}

/*
* Elimina del archivo JSON el usuario del cual se ingresa la identificacion
* Retorna:
* 1 Si el usaurio fue eliminado
* 0 si el usuario no fue encontrado
*/
int eliminarUsuarioJSON(const char *identificacion){
    FILE *archivo = fopen("data/usuarios.json","r"); //Abrir el archivo JSON en modo lectura
    if(archivo == NULL){
        return 0;
    }
    char contenedor[10000]; //Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) -1, archivo); //Leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0'; //Agregar un caracter nulo al final del buffer
    fclose(archivo); //Cerrar el archivo JSON
    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    if(raiz == NULL) {
        return 0;
    }
    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuario = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id = cJSON_GetObjectItem(usuario, "identificacion"); // Obtener la identifación del usuario
        // Comprara la identifiacion ingresada con las almacenadas
        if(strcmp(id->valuestring, identificacion) == 0){
            cJSON_DeleteItemFromArray(raiz, indice); // Elimina la direccion que esta guardada

            char *jsonString = cJSON_Print(raiz); //Convertir el objeto JSON a una cadena de caracteres

            //Guardar en el archivo
            archivo = fopen("data/usuarios.json", "w"); //Abrir el archivo JSON en modo escritura
            fprintf(archivo, "%s", jsonString); //Escribir la cadena de caracteres en el archivo JSON
            fclose(archivo); // Cerrar el archivo JSON
            free(jsonString); // Liberar la memoria de la cadena de caracteres
            cJSON_Delete(raiz); // Liberar la memoria del objeto JSON
            return 1;
        } 
    }
    cJSON_Delete(raiz);
    return 0;
}

/*
 * Verifica si una identificacion ya se esta registrada dentro del archivo usuarios.json.
 * Retorna:
 * 1 si existe.
 * 0 Si no existe.
 */
int existeIdentificacionJSON(const char *identificacion){
    FILE *archivo = fopen("data/usuarios.json","r"); //Abrir el archivo JSON en modo lectura
    if(archivo == NULL) {
        return 0;
    }
    char contenedor[10000]; //Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) -1, archivo); //Leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0'; //Agregar un caracter nulo al final del buffer
    fclose(archivo); //Cerrar el archivo JSON
    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    if(raiz == NULL) {
        return 0;
    }
    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuario = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id =cJSON_GetObjectItem(usuario, "identificacion"); // Obtener la identifación
        // Comprara la identifiacion ingresada con las almacenadas
        if(strcmp(id->valuestring, identificacion) == 0){
            cJSON_Delete(raiz);
            return 1;
        }
    }
    cJSON_Delete(raiz);
    return 0;
}

/*
* Verifica si el usuario tiene registros asociados
* Retorna:
* 1 Si el usuario tine regitros asociados
* 0 Si el usuario no tiene regstros asociados

* La funcion actualamnete retorna 0 porue la parte de prestamos no esta implementada todavia.
*/
int tieneRegistrosAsociados(const char *identificacion){
    (void)identificacion;
    return 0;
}

/*
 * ---------------------------Catalogo---------------------------
 */

/*
 * Verifica si una produccion por su nombre ya existe en catalogo.json.
 * Retorna: 1 si existe, 0 si no existe o falla la lectura.
 */
int existeProduccionJSON(const char *nombre) {
    FILE *archivo = fopen("data/catalogo.json", "r");
    if (archivo == NULL) return 0;

    char contenedor[10000];// Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) - 1, archivo);//leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0';
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenedor);
    if (raiz == NULL) return 0;

    int cantidad = cJSON_GetArraySize(raiz);
    for (int i = 0; i < cantidad; i++) {
        cJSON *prod = cJSON_GetArrayItem(raiz, i);
        cJSON *nom = cJSON_GetObjectItem(prod, "nombre");
        if (nom && nom->valuestring && strcmp(nom->valuestring, nombre) == 0) {//Compara el nombre de la produccion con el nombre buscado
            cJSON_Delete(raiz);
            return 1;
        }
    }

    cJSON_Delete(raiz);// Liberar la memoria del objeto JSON
    return 0;
}

/*
 * Guarda una nueva produccion en catalogo.json.
 */
void guardarProduccionJSON(Produccion prod) {
    FILE *archivo;
    char contenedor[10000];
    cJSON *raiz = NULL;

    archivo = fopen("data/catalogo.json", "r");
    if (archivo != NULL) {
        size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) - 1, archivo);
        contenedor[largoArchivo] = '\0';
        fclose(archivo);
        raiz = cJSON_Parse(contenedor);
    }

    if (raiz == NULL) {
        raiz = cJSON_CreateArray();
    }

    cJSON *nuevaProd = cJSON_CreateObject();
    cJSON_AddStringToObject(nuevaProd, "nombre", prod.nombre);
    cJSON_AddStringToObject(nuevaProd, "autor", prod.autor);
    cJSON_AddNumberToObject(nuevaProd, "anio_publicacion", prod.anio_publicacion);
    cJSON_AddStringToObject(nuevaProd, "genero", prod.genero);
    cJSON_AddStringToObject(nuevaProd, "resumen", prod.resumen);
    cJSON_AddNumberToObject(nuevaProd, "cantidad", prod.cantidad);

    cJSON_AddItemToArray(raiz, nuevaProd);

    char *jsonString = cJSON_Print(raiz);
    archivo = fopen("data/catalogo.json", "w");
    if (archivo != NULL) {
        fprintf(archivo, "%s", jsonString);
        fclose(archivo);
    }

    free(jsonString);
    cJSON_Delete(raiz);
}


/*
 * ---------------------------Ejemplares---------------------------
 */

/*
 * Genera N ejemplares segun se indique en el lote en ejemplares.json vinculados al nombre de la producción.
 * Formato de ID de ejemplar: "NombreProduccion:1", "NombreProduccion:2"...
 */

void generarEjemplaresJSON(const char *nombre, int cantidad) {
    FILE *archivo;
    char contenedor[20000];
    cJSON *raiz = NULL;

    archivo = fopen("data/ejemplares.json", "r");
    if (archivo != NULL) {
        size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) - 1, archivo);
        contenedor[largoArchivo] = '\0';
        fclose(archivo);
        raiz = cJSON_Parse(contenedor);
    }

    if (raiz == NULL) {
        raiz = cJSON_CreateArray();
    }

    for (int i = 1; i <= cantidad; i++) {
        cJSON *ejemplar = cJSON_CreateObject();

        char idEjemplar[256];
        snprintf(idEjemplar, sizeof(idEjemplar), "%s: %d", nombre, i);

        cJSON_AddStringToObject(ejemplar, "id", idEjemplar);
        cJSON_AddStringToObject(ejemplar, "produccion", nombre);
        cJSON_AddStringToObject(ejemplar, "estado", "Disponible");

        cJSON_AddItemToArray(raiz, ejemplar);
    }

    char *jsonString = cJSON_Print(raiz);
    archivo = fopen("data/ejemplares.json", "w");
    if (archivo != NULL) {
        fprintf(archivo, "%s", jsonString);
        fclose(archivo);
    }

    free(jsonString);
    cJSON_Delete(raiz);
}