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
* Lee todo el contenido de un archivo y lo almacena en memoria dinamica
* Funcionamiento:
* - Obtiene el tamaño total del archivo.
* - Reserva memoria con malloc().
* - Lee el contenido completo del archivo.
* - Agrega el caracter nulo '\0' al final para
* tratar el contenido como una cadena.

* Parametros:
* - archivo: puntero al archivo previamente abierto.
* Retorna:
* - Un puntero a memoria dinamica con el contenido completo del archivo.
* - NULL si ocurre un error al reservar memoria.
*
* Importante:
* - La memoria devuelta se liberada con free() cuando ya no se necesite.
*/
char *leerArchivoCompleto(FILE *archivo){
    fseek(archivo, 0, SEEK_END); // Mover el cursor al final para conseguir el tamaño
    long tamano = ftell(archivo); // Obtener el tamaño total del archivo en bytes
    rewind(archivo); // Regresar el cursor al inicio del archivo

    char *contenedor = malloc(tamano + 1); //Reservar memoria dinamica para almacenar el contenido

    if(contenedor == NULL){ // Verificar que la memoria se reservocon exito
        return NULL;
    }
    fread(contenedor, 1, tamano, archivo); // Leer el contenido completo del archivo
    contenedor[tamano] = '\0'; //Agregar terminador nulo para formar una cadena válida

    return contenedor;
}

/*
 * Convierte la estructura Usuario en un objeto JSON y la guarda dentro del archivo usuarios.json.
 */
void guardarUsuariosJSON(Usuario usuario){
    FILE *archivo; //Puntero al archivo JSON
    char *contenedor = NULL; // Para almacenar el JSON
    cJSON *raiz = NULL; //Puntero a la raiz del JSON
    long tamanoArchivo = ftell(archivo);

    archivo = fopen("data/usuarios.json", "r"); //Abrir el archivo JSON en modo lectura
    if (archivo != NULL) {
        contenedor =leerArchivoCompleto(archivo);
        if(contenedor == NULL){
            fclose(archivo);
            return;
        }
        fclose(archivo); //Cerrar el archivo JSON
        
        raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
        // Liberar la memoria que se uso para leer el archivo
        free(contenedor);
        contenedor = NULL;
    }
    //Si el archivo contiene datos pero no puede convertirse a JSON, se considera corrupto.
    if(strlen(contenedor) > 0 && raiz == NULL){
        printf("Error: El archivo usuarios.json esta corrupto o fue manipulado.\n");
        return;
    }
    //Si el archivo existe pero está vacío se crea un arreglo nuevo.
    if(tamanoArchivo == 0){
        raiz = cJSON_CreateArray();
    }
    // Validar que la estructura del archivo JSON sea un arreglo
    if(!cJSON_IsArray(raiz)){
        printf("Error: La estrtura de usuarios.json no es valida.\n");
        cJSON_Delete(raiz);
        return;
    }

    cJSON *nuevoUsuario = cJSON_CreateObject(); //Crear un nuevo objeto JSON para el usuario
    cJSON_AddStringToObject(nuevoUsuario, "identificacion", usuario.identificacion); //Agregar el id del usuario al objeto JSON
    cJSON_AddStringToObject(nuevoUsuario, "nombre", usuario.nombre); //Agregar el nombre del usuario al objeto JSON
    cJSON_AddStringToObject(nuevoUsuario, "direccion", usuario.direccion); //Agregar el direccion del usuario al objeto JSON
    cJSON_AddItemToArray(raiz, nuevoUsuario); // Agregar el nuevo usuario al array de usuarios

    char *jsonString = cJSON_Print(raiz); // Convertir el objeto JSON a una cadena de caracteres

    //Guardar en el archivo
    archivo = fopen("data/usuarios.json", "w"); // Abrir el archivo JSON en modo escritura
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
    char *contenedor = leerArchivoCompleto(archivo); //Para almacenar el JSON
    fclose(archivo); //Cerrar el archivo JSON

    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    // Liberar la memoria que se uso para leer el archivo
    free(contenedor);
    contenedor = NULL;

    // Valida que el archivo JSON no este daana o manipulado
    if(raiz == NULL){
        printf("Error: El archivo usuario.json esta corrupto o fue manipulado.\n");
        return; 
    }
    // Validar que la estructura del archivo JSON sea un arreglo
    if(!cJSON_IsArray(raiz)){
        printf("Error: La estrtura de usuarios.json no es valida.\n");
        cJSON_Delete(raiz);
        return;
    }

    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuario = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id = cJSON_GetObjectItem(usuario, "identificacion");
        cJSON *nombre = cJSON_GetObjectItem(usuario, "nombre");
        cJSON *direccion = cJSON_GetObjectItem(usuario, "direccion");

        // Validar que los campos existan y si las cadenas de texto validas
        if(id == NULL || nombre == NULL || direccion == NULL || !cJSON_IsString(id) || !cJSON_IsString(nombre) ||!cJSON_IsString(direccion)){
            continue;
        } 
        printf("\n---------------------------------\n");
        printf("Identificacion: %s\n", id->valuestring); //Imprimir el id del usuario
        printf("Nombre: %s\n", nombre->valuestring); //Imprimir el nombre del usuario
        printf("Direccion: %s\n", direccion->valuestring); //Imprimir el correo del usuario
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
    char *contenedor = leerArchivoCompleto(archivo);    
    fclose(archivo); //Cerrar el archivo JSON
    
    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    // Liberar la memoria que se uso para leer el archivo
    free(contenedor);
    contenedor = NULL;

    // Valida que el archivo JSON no este daana o manipulado
    if(raiz == NULL){
        printf("Error: El archivo usuario.json esta corrupto o fue manipulado.\n");
        return 0; 
    }
    // Validar que la estructura del archivo JSON sea un arreglo
    if(!cJSON_IsArray(raiz)){
        printf("Error: La estrtura de usuarios.json no es valida.\n");
        cJSON_Delete(raiz);
        return 0;
    }

    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuarioJSON = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id = cJSON_GetObjectItem(usuarioJSON, "identificacion"); // Obtener la identifación del usuario
        
        // Validar que el campo de identificacion exitas y sea una cadena de texto valida
        if(id == NULL || !cJSON_IsString(id)){
            continue;
        }

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
    char *contenedor = leerArchivoCompleto(archivo); //Para almacenar el JSON
    size_t largoArchivo = fread(contenedor, 1, sizeof(contenedor) -1, archivo); //Leer el contenido del archivo JSON
    contenedor[largoArchivo] = '\0'; //Agregar un caracter nulo al final del buffer
    fclose(archivo); //Cerrar el archivo JSON
    
    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    // Liberar la memoria que se uso para leer el archivo
    free(contenedor);
    contenedor = NULL;

    // Valida que el archivo JSON no este daana o manipulado
    if(raiz == NULL){
        printf("Error: El archivo usuario.json esta corrupto o fue manipulado.\n");
        return 0; 
    }
    // Validar que la estructura del archivo JSON sea un arreglo
    if(!cJSON_IsArray(raiz)){
        printf("Error: La estrtura de usuarios.json no es valida.\n");
        cJSON_Delete(raiz);
        return 0;
    }
    
    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuario = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id = cJSON_GetObjectItem(usuario, "identificacion"); // Obtener la identifación del usuario
        
        // Validar que el campo de identificacion exitas y sea una cadena de texto valida
        if(id == NULL || !cJSON_IsString(id)){
            continue;
        }

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
    fseek(archivo, 0, SEEK_END);
    long largoArchivo = ftell(archivo);
    rewind(archivo);

    char *contenedor = leerArchivoCompleto(archivo); //Para almacenar el JSON
    fclose(archivo); //Cerrar el archivo JSON
    
    cJSON *raiz = cJSON_Parse(contenedor); //Convertir el contenido del contenedor a un objeto JSON
    // Liberar la memoria que se uso para leer el archivo
    free(contenedor);
    contenedor = NULL;

    // Valida que el archivo JSON no este dañado o manipulado
    if(largoArchivo == 0){
        return 0;
    }
    // Validar que la estructura del archivo JSON sea un arreglo
    if(!cJSON_IsArray(raiz)){
        printf("Error: La estrtura de usuarios.json no es valida.\n");
        cJSON_Delete(raiz);
        return 0;
    }

    int cantidadUsuarios = cJSON_GetArraySize(raiz); //Obtener la cantidad de usuarios en el array de usuarios
    for(int indice = 0; indice < cantidadUsuarios; indice++){
        cJSON *usuario = cJSON_GetArrayItem(raiz, indice); // Obtener el usuario en la posición indice del array de usuarios
        cJSON *id =cJSON_GetObjectItem(usuario, "identificacion"); // Obtener la identifación
        
        // Validar que el campo de identificacion exitas y sea una cadena de texto valida
        if(id == NULL || !cJSON_IsString(id)){
            continue;
        }
        
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