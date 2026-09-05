/*
* Lee y escribe los datos de los usuario en JSON
* convierte los structs de usuarios a JSON y viceversa
* Encapsula la persistencia de datos de usuarios
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "persistencia.h"
#include <cjson/cJSON.h>

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
    cJSON_AddItemToArray(raiz, nuevoUsuario); //Agregar el nuevo usuario al array de usuarios

    char *jsonString = cJSON_Print(raiz); //Convertir el objeto JSON a una cadena de caracteres

    //Guardar en el archivo
    archivo = fopen("data/usuarios.json", "w"); //Abrir el archivo JSON en modo escritura
    if(archivo != NULL){
        fprintf(archivo, "%s", jsonString); //Escribir la cadena de caracteres en el archivo JSON
        fclose(archivo); //Cerrar el archivo JSON
    }
    free(jsonString); //Liberar la memoria de la cadena de caracteres
    cJSON_Delete(raiz); //Liberar la memoria del objeto JSON
    printf("-----------------------------\n");
    printf("Usuario guardado con exito\n"); 
}


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

//Retorna 1 si la identifación exisite y 0 si no
int existeIdentificacionJSON(const char *identificacion){
    FILE *archivo = fopen("data/usuarios.json","r"); ////Abrir el archivo JSON en modo lectura
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
