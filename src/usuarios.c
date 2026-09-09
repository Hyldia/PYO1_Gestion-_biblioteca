/*
* Administra los usuarios del sistema
* Se implementan todas las funciones para la gestion de los usuarios

* Para acceder a los archivos JSON se hace por medio de persistencia.h 
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h> //para valida identificacion
#include "usuarios.h"
#include "persistencia.h"
#include "tipos.h"

/*
 * Comprueba que la identificacion:
 * - Tenga exactamente 5 digitos y que este formado solo por numeros.
 *
 * Retorna:
 * 1 Si es valida.
 * 0 Si es invalida.
 */
int verificarIdentificacion(const char *identificacion){
    // No permitir espacios ni cadenas vacias
    if(identificacion == NULL || identificacion[0] == 0){
        return 0;
    }
    for(int indice = 0; identificacion[indice] != 0; indice++){ //No aceptar espacios
        if(identificacion[indice] == ' '){
            return 0;
        }
    }

    //La identifiación debe tener una longitud de 5 digitos
    if(strlen(identificacion) != 9){
        return 0;
    } 
    for(int indice = 0; indice < 9; indice++){
        if(!isdigit(identificacion[indice])){
            return 0;
        }
    }
    return 1;
}

/*
 * Verifica que un campo de texto no este vacio ni compuesto solo por espacios en blanco.
 
 * Retorna:
 * 1 Si el campo contiene informacion valida.
 * 0 Si el campo esta vacío o solo tiene espacios.
 */
int validarCampoVacio(const char *texto){
    if(texto == NULL){
        return 0;
    }
    //Verificar si todos los caractres son espacios
    while(*texto != 0) {
        if(!isspace(*texto)){ // No haya espaio vacio
            return 1; // Hay cntenido en el texto
        }
        texto++;
    }
    return 0;
}

/*
* Solicita los datos del usuario
* Verifica que la identificacion sea valida y que no exista previamente.
* Despues guarda el usuario en el JSON.
*/
void agregarUsuario() {
    Usuario usuario;

    printf("Identificacion (9 digitos): ");
    fgets(usuario.identificacion, MAX_IDENTIFICACION, stdin); //Pide la identificacion del usuario y lo guarda con fgets
    usuario.identificacion[strcspn(usuario.identificacion,"\n")] = 0; //Elimina el salto de línea al final de la identificacion
    // Comprobar identifiacion
    if(!verificarIdentificacion(usuario.identificacion)){
        printf("-----------------------------\n");
        printf("Error: la identificacion debe tener 9 numeros.\n");
        return;
    }
    if(existeIdentificacionJSON(usuario.identificacion)){
        printf("-----------------------------\n");
        printf("Error: la identificacion ya existe.\n");
        return;
    }

    printf("Nombre:");
    fgets(usuario.nombre, MAX_NOMBRE, stdin); //Pide el nombre del usuario y lo guarda con fgets
    usuario.nombre[strcspn(usuario.nombre, "\n")] = 0; //Elimina el salto de línea al final del nombre
    // Comprobar texto
    if(!validarCampoVacio(usuario.nombre)){
        printf("-----------------------------\n");
        printf("Error: El nombre no puede estar vacio.\n");
        return;
    }
    
    printf("Direccion:");
    fgets(usuario.direccion, MAX_DIRECCION, stdin); //Pide el direccion del usuario y lo guarda con fgets
    usuario.direccion[strcspn(usuario.direccion, "\n")] = 0;
    // Comprobar texto
    if(!validarCampoVacio(usuario.direccion)){
        printf("-----------------------------\n");
        printf("Error: La direccion no puede sestar vacia.\n");
        return;
    }
    
    guardarUsuariosJSON(usuario); //Llama a la función para guardar el usuario en el archivo JSON
}

/*
 * Muestra todos los usuarios registrados en el sistema.
 */
void mostrarUsuarios() {
    mostrarUsuariosJSON();
}

/*
 * Busca el usuario por su identificacion y actualiza su direccion.
 */
void modificarUsuario(){
    Usuario usuario; // Crear variable de tipo usuario para guardar los datos ingresados
    
    printf("Ingrese la identificacion del usuario que desea modifica: ");
    fgets(usuario.identificacion, MAX_IDENTIFICACION, stdin); //Pide la identificacion del usuario y lo guarda con fgets
    usuario.identificacion[strcspn(usuario.identificacion, "\n")] = 0; //Elimina el salto de línea al final de la identificacion
//
    // Verifica que la identificación tenga 9 numeros
    if(!verificarIdentificacion(usuario.identificacion)){
        printf("-----------------------------\n");
        printf("Error: La identificacion debe tener 9 numeros.\n");
        return;
    }
    printf("Ingresa la nueva direccion:");
    fgets(usuario.direccion, MAX_DIRECCION, stdin); //Pide el direccion del usuario y lo guarda con fgets
    usuario.direccion[strcspn(usuario.direccion, "\n")] = 0;//Elimina el salto de línea al final de la direccion
    // Comprobar texto
    if(!validarCampoVacio(usuario.direccion)){
        printf("-----------------------------\n");
        printf("Error: La direccion no puede estar vacia.\n");
        return;
    }
    // Llama la funcio modificar JSON para que busque el usaurio y actualice os datos 
    if(modificarUsuarioJSON(usuario)){
        printf("-----------------------------\n");
        printf("La direccion fue modificada con exito. \n");
    } else{
        printf("-----------------------------\n");
        printf("Usuario no encontrado.\n");
    }
}

/*
* Solicita la identificacion del usuario a eliminar
* Verifica si tiene registros asociados
* Si no tiene elimina el usuario y si tiene muestra un mensaje informativo
*/
void eliminarUsuario(){
    char identificacion[MAX_IDENTIFICACION]; // Guarda la identificacion
    
    printf("Ingrese la identificacion del usuario que desea eliminar: "),
    fgets(identificacion, MAX_IDENTIFICACION, stdin);  //Pide la identificacion del usuario y lo guarda con fgets
    identificacion[strcspn(identificacion, "\n")] = 0; //Elimina el salto de línea al final de la identificacion
//
    // Verifica que la identificación tenga 9 numeros
    if(!verificarIdentificacion(identificacion)){
        printf("-----------------------------\n");
        printf("Error: La identificacion debe tener 9 numeros.\n");
        return;
    }
    
    //Comrobar si el usuario tiene registros
    if(tieneRegistrosAsociados(identificacion)){
        printf("-----------------------------\n");
        printf("Error: El usaurio tiene registros asociados, no puede ser eliminado.\n");
        return;
    }
    if(eliminarUsuarioJSON(identificacion)){
        printf("-----------------------------\n");
        printf("El usuario fue eliminado con exito.\n");
    }else{
        printf("-----------------------------\n");
        printf("El usuario no fue encontrado.\n");
    }
}
