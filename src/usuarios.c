/*
*Administra los usuarios del sistema
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h> //para valida identificación
#include "usuarios.h"
#include "persistencia.h"
#include "tipos.h"

// Comprobar la identificación que iongresa el usuario
int verificarIdentificacion(const char *identificacion){
    if(strlen(identificacion) != 5){//La identifiación debe tener una longitud de 5 digitos
        return 0;
    } 
    for(int indice = 0; indice < 5; indice++){
        if(!isdigit(identificacion[indice])){
            return 0;
        }
    }
    return 1;
}

void agregarUsuario() {
    Usuario usuario;
    printf("Identificacion (5 digitos): ");
    fgets(usuario.identificacion, MAX_IDENTIFICACION, stdin); //Pide la identificacion del usuario y lo guarda con fgets
    usuario.identificacion[strcspn(usuario.identificacion,"\n")] = 0; //Elimina el salto de línea al final de la identificacion
    // Comprobar identifiacion
    if(!verificarIdentificacion(usuario.identificacion)){
        printf("Error: la identificacion debe tener 5 numeros.\n");
        return;
    }
    if(existeIdentificacionJSON(usuario.identificacion)){
        printf("Error: la identificacion ya existe.\n");
        return;
    }

    printf("Nombre:");
    fgets(usuario.nombre, MAX_NOMBRE, stdin); //Pide el nombre del usuario y lo guarda con fgets
    usuario.nombre[strcspn(usuario.nombre, "\n")] = 0; //Elimina el salto de línea al final del nombre

    printf("Direccion:");
    fgets(usuario.direccion, MAX_DIRECCION, stdin); //Pide el direccion del usuario y lo guarda con fgets
    usuario.direccion[strcspn(usuario.direccion, "\n")] = 0;
    
    usuario.activo = 1; //Marca al usuario como activo

    guardarUsuariosJSON(usuario); //Llama a la función para guardar el usuario en el archivo JSON
}


void mostrarUsuarios() {
    mostrarUsuariosJSON();
}

