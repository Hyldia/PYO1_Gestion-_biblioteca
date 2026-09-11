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
    long tamanoArchivo = 0;

    archivo = fopen("data/usuarios.json", "r"); //Abrir el archivo JSON en modo lectura
    if (archivo != NULL) {
        //Actualizar tamaño archivo
        fseek(archivo, 0, SEEK_END);
        tamanoArchivo = ftell(archivo);
        rewind(archivo);

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
    if(tamanoArchivo > 0 && raiz == NULL){
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
 * ---------------------------Catalogo---------------------------
 */

/*
 * Verifica si una produccion por su nombre ya existe en catalogo.json.
 * Retorna: 1 si existe, 0 si no existe o falla la lectura.
 */
int existeProduccionJSON(const char *nombre) {
    FILE *archivo = fopen("data/catalogo.json", "r");// Abrir el archivo JSON en modo lectura
    if (archivo == NULL) return 0;

    char *contenedor = (char *)malloc(MAX_CONTENEDOR_CATALOGO * sizeof(char));
    if (contenedor == NULL) return 0;
    
    size_t largoArchivo = fread(contenedor, 1, MAX_CONTENEDOR_CATALOGO - 1, archivo);
    contenedor[largoArchivo] = '\0';
    fclose(archivo);

    cJSON *raiz = cJSON_Parse(contenedor);
    if (raiz == NULL) {
        free(contenedor);
        return 0;
    }

    int cantidad = cJSON_GetArraySize(raiz);// Obtener la cantidad de producciones en el array de producciones
    for (int i = 0; i < cantidad; i++) {
        cJSON *prod = cJSON_GetArrayItem(raiz, i);
        cJSON *nom = cJSON_GetObjectItem(prod, "nombre");
        if (nom && nom->valuestring && strcmp(nom->valuestring, nombre) == 0) {//Compara el nombre de la produccion con el nombre buscado
            cJSON_Delete(raiz);
            free(contenedor);
            return 1;
        }
    }

    cJSON_Delete(raiz);
    free(contenedor);
    return 0;
}

/*
 * Guarda una nueva produccion en catalogo.json.
 */
void guardarProduccionJSON(Produccion prod) {
    FILE *archivo;
    char *contenedor = (char *)malloc(MAX_CONTENEDOR_CATALOGO * sizeof(char));
    if (contenedor == NULL) {
        printf("Error: No se pudo asignar memoria para el contenedor.\n");
        return;
    }
    
    cJSON *raiz = NULL;

    archivo = fopen("data/catalogo.json", "r");// Abrir el archivo JSON en modo lectura
    if (archivo != NULL) {
        size_t largoArchivo = fread(contenedor, 1, MAX_CONTENEDOR_CATALOGO - 1, archivo);
        contenedor[largoArchivo] = '\0';
        fclose(archivo);
        raiz = cJSON_Parse(contenedor);
    }

    if (raiz == NULL) {// Si el archivo JSON no existe, crear un nuevo objeto JSON
        raiz = cJSON_CreateArray();
    }

    cJSON *nuevaProd = cJSON_CreateObject();// Crear un nuevo objeto JSON para la produccion
    cJSON_AddStringToObject(nuevaProd, "nombre", prod.nombre);// Agregar el nombre de la produccion al objeto JSON
    cJSON_AddStringToObject(nuevaProd, "autor", prod.autor);// Agregar el autor de la produccion al objeto JSON
    cJSON_AddNumberToObject(nuevaProd, "anio_publicacion", prod.anio_publicacion);// Agregar el año de publicacion de la produccion al objeto JSON
    cJSON_AddStringToObject(nuevaProd, "genero", prod.genero);// Agregar el genero de la produccion al objeto JSON
    cJSON_AddStringToObject(nuevaProd, "resumen", prod.resumen);// Agregar el resumen de la produccion al objeto JSON
    cJSON_AddNumberToObject(nuevaProd, "cantidad", prod.cantidad);// Agregar la cantidad de libros de la produccion al objeto JSON

    cJSON_AddItemToArray(raiz, nuevaProd);

    char *jsonString = cJSON_Print(raiz);
    archivo = fopen("data/catalogo.json", "w");
    if (archivo != NULL) {
        fprintf(archivo, "%s", jsonString);
        fclose(archivo);
    }

    free(jsonString);
    cJSON_Delete(raiz);
    free(contenedor);
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
    char *contenedor = (char *)malloc(MAX_CONTENEDOR_CATALOGO * sizeof(char));
    if (contenedor == NULL) {
        printf("Error: No se pudo asignar memoria para el contenedor.\n");
        return;
    }
    
    cJSON *raiz = NULL;

    archivo = fopen("data/ejemplares.json", "r");// Abrir el archivo JSON en modo lectura
    if (archivo != NULL) {
        size_t largoArchivo = fread(contenedor, 1, MAX_CONTENEDOR_CATALOGO - 1, archivo);
        contenedor[largoArchivo] = '\0';
        fclose(archivo);
        raiz = cJSON_Parse(contenedor);
    }

    if (raiz == NULL) {// Si el archivo JSON no existe, crear un nuevo objeto JSON
        raiz = cJSON_CreateArray();
    }

    for (int i = 1; i <= cantidad; i++) {// Crear un nuevo objeto JSON para cada ejemplar
        cJSON *ejemplar = cJSON_CreateObject();

        char *idEjemplar = (char *)malloc(256 * sizeof(char));
        if (idEjemplar == NULL) {
            cJSON_Delete(ejemplar);
            continue;
        }
        snprintf(idEjemplar, 256, "%s- %d", nombre, i);

        cJSON_AddStringToObject(ejemplar, "id", idEjemplar);// Agregar el ID del ejemplar al objeto JSON
        cJSON_AddStringToObject(ejemplar, "produccion", nombre);// Agregar el nombre de la produccion al objeto JSON
        cJSON_AddStringToObject(ejemplar, "estado", "Disponible");// Agregar el estado del ejemplar al objeto JSON

        cJSON_AddItemToArray(raiz, ejemplar);// Agregar el nuevo ejemplar al array de ejemplares
    }

    char *jsonString = cJSON_Print(raiz);// Convertir el objeto JSON a una cadena de caracteres
    archivo = fopen("data/ejemplares.json", "w");// Abrir el archivo JSON en modo escritura
    if (archivo != NULL) {
        fprintf(archivo, "%s", jsonString);// Escribir la cadena de caracteres en el archivo JSON
        fclose(archivo);
    }

    free(jsonString);
    cJSON_Delete(raiz);
    free(contenedor);
}

/*nota: raiz es el objeto JSON principal */

/* Persistencia de Prestamos */
/*
* Objetivo:abrir un archivo JSON, leerlo completo y convertirlo a objeto cJSON.
* Entradas: ruta - ruta del archivo.
* Salidas:puntero cJSON con el contenido, o NULL si el archivo no existe o esta vacio/corrupto.
*/
static cJSON *cargarArchivoJSON(const char *ruta) {
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        return NULL;
    }

    char *contenido = leerArchivoCompleto(archivo);
    fclose(archivo);
    if (contenido == NULL) {
        return NULL;
    }

    cJSON *raiz = cJSON_Parse(contenido);
    free(contenido);
    return raiz;
}

/*
* Objetivo:escribir un objeto cJSON en un archivo (lo reemplaza completo).
* Entradas:ruta - archivo destino; raiz - objeto cJSON a guardar.
* Salidas:ninguna
*/
static void escribirArchivoJSON(const char *ruta, cJSON *raiz) {
    char *texto = cJSON_Print(raiz);
    if (texto == NULL) return;

    FILE *archivo = fopen(ruta, "w");
    if (archivo != NULL) {
        fprintf(archivo, "%s", texto);
        fclose(archivo);
    }
    free(texto);
}

/*
* Objetivo:indicar si dos rangos de fechas se cruzan (formato YYYY-MM-DD).
* Entradas:aIni, aFin - primer rango; bIni, bFin - segundo rango.
* Salidas:1 si se cruzan, 0 si no.
* Restricciones: compara las fechas como texto (solo sirve con formato ISO).
*/
static int fechasSeCruzan(const char *aIni, const char *aFin, const char *bIni, const char *bFin) {
    return (strcmp(aIni, bFin) <= 0 && strcmp(bIni, aFin) <= 0);
}

/*
* Objetivo:indicar si un arreglo cJSON de cadenas contiene un valor dado.
* Entradas:arr - arreglo cJSON; valor - cadena a buscar.
* Salidas:1 si lo contiene, 0 si no.
*/
static int arregloContieneCadena(cJSON *arr, const char *valor) {
    if (!cJSON_IsArray(arr)) return 0;
    int cantidad = cJSON_GetArraySize(arr);
    for (int i = 0; i < cantidad; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        if (cJSON_IsString(item) && strcmp(item->valuestring, valor) == 0) {
            return 1;
        }
    }
    return 0;
}
/*
* Objetivo:  calcular el id que le corresponde al proximo prestamo.
* Entradas:  ninguna (lee data/prestamos.json).
* Salidas:   el id mas alto encontrado + 1; 1 si no hay prestamos.
*/
int siguienteIdPrestamoJSON(void) {
    cJSON *raiz = cargarArchivoJSON("data/prestamos.json");
    if (!cJSON_IsArray(raiz)) {
        cJSON_Delete(raiz);
        return 1;
    }

    int maximo = 0;
    int cantidad = cJSON_GetArraySize(raiz);
    for (int i = 0; i < cantidad; i++) {
        cJSON *p  = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(p, "id");
        if (cJSON_IsNumber(id) && id->valueint > maximo) {
            maximo = id->valueint;
        }
    }

    cJSON_Delete(raiz);
    return maximo + 1;
}

/*
* Objetivo:  agregar un prestamo al archivo data/prestamos.json.
* Entradas:  prestamo - estructura con todos los datos (paso por valor).
* Salidas:   ninguna (modifica el archivo).
*/
void guardarPrestamoJSON(Prestamo prestamo) {
    cJSON *raiz = cargarArchivoJSON("data/prestamos.json");
    if (!cJSON_IsArray(raiz)) {
        cJSON_Delete(raiz);
        raiz = cJSON_CreateArray();
    }

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", prestamo.id);
    cJSON_AddStringToObject(obj, "usuario", prestamo.usuario);
    cJSON_AddStringToObject(obj, "fecha_inicio", prestamo.fecha_inicio);
    cJSON_AddStringToObject(obj, "fecha_entrega", prestamo.fecha_entrega);
    cJSON_AddStringToObject(obj, "estado", prestamo.estado);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < prestamo.cantidad_ejemplares; i++) {
        cJSON_AddItemToArray(arr, cJSON_CreateString(prestamo.ejemplares[i]));
    }
    cJSON_AddItemToObject(obj, "ejemplares", arr);

    cJSON_AddItemToArray(raiz, obj);

    escribirArchivoJSON("data/prestamos.json", raiz);
    cJSON_Delete(raiz);
}

/*
* Objetivo:indicar si un ejemplar existe en data/ejemplares.json.
* Entradas:idEjemplar - identificador a buscar.
* Salidas:1 si existe, 0 si no.
*/
int existeEjemplarJSON(const char *idEjemplar) {
    cJSON *raiz = cargarArchivoJSON("data/ejemplares.json");
    if (!cJSON_IsArray(raiz)) {
        cJSON_Delete(raiz);
        return 0;
    }

    int encontrado = 0;
    int cantidad = cJSON_GetArraySize(raiz);
    for (int i = 0; i < cantidad && !encontrado; i++) {
        cJSON *ej = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(ej, "id");
        if (cJSON_IsString(id) && strcmp(id->valuestring, idEjemplar) == 0) {
            encontrado = 1;
        }
    }

    cJSON_Delete(raiz);
    return encontrado;
}

/*
* Objetivo:indicar si un ejemplar esta disponible en un rango de fechas.
* Entradas:idEjemplar - ejemplar a revisar; fInicio, fFin - rango pedido.
* Salidas:1 si esta disponible, 0 si ya esta prestado en fechas que se cruzan.
* Restricciones: solo considera prestamos con estado "activo".
*/
int ejemplarDisponibleJSON(const char *idEjemplar,
                           const char *fInicio, const char *fFin) {
    cJSON *raiz = cargarArchivoJSON("data/prestamos.json");
    if (!cJSON_IsArray(raiz)) {
        cJSON_Delete(raiz);
        return 1; // no hay prestamos
    }

    int disponible = 1;
    int cantidad = cJSON_GetArraySize(raiz);
    for (int i = 0; i < cantidad && disponible; i++) {
        cJSON *p = cJSON_GetArrayItem(raiz, i);

        cJSON *estado = cJSON_GetObjectItem(p, "estado");
        if (!cJSON_IsString(estado) || strcmp(estado->valuestring, "activo") != 0) {
            continue; // solo interesan los activos
        }

        cJSON *arr  = cJSON_GetObjectItem(p, "ejemplares");
        cJSON *pIni = cJSON_GetObjectItem(p, "fecha_inicio");
        cJSON *pFin = cJSON_GetObjectItem(p, "fecha_entrega");
        if (!cJSON_IsString(pIni) || !cJSON_IsString(pFin)) continue;

        if (arregloContieneCadena(arr, idEjemplar) &&
            fechasSeCruzan(fInicio, fFin, pIni->valuestring, pFin->valuestring)) {
            disponible = 0;
        }
    }

    cJSON_Delete(raiz);
    return disponible;
}
/*
* Objetivo:  cambiar el estado de un ejemplar en data/ejemplares.json.
* Entradas:  idEjemplar - ejemplar a modificar; nuevoEstado - "Prestado"/"Disponible".
* Salidas:   ninguna (modifica el archivo si encuentra el ejemplar).
*/
void cambiarEstadoEjemplarJSON(const char *idEjemplar, const char *nuevoEstado) {
    cJSON *raiz = cargarArchivoJSON("data/ejemplares.json");
    if (!cJSON_IsArray(raiz)) {
        cJSON_Delete(raiz);
        return;
    }

    int cantidad = cJSON_GetArraySize(raiz);
    for (int i = 0; i < cantidad; i++) {
        cJSON *ej = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(ej, "id");
        if (cJSON_IsString(id) && strcmp(id->valuestring, idEjemplar) == 0) {
            cJSON_ReplaceItemInObject(ej, "estado", cJSON_CreateString(nuevoEstado));
            break;
        }
    }

    escribirArchivoJSON("data/ejemplares.json", raiz);
    cJSON_Delete(raiz);
}
/*
* Objetivo: obtener el nombre de la produccion a la que pertenece un ejemplar.
* Entradas: idEjemplar - identificador del ejemplar.
* Salidas:puntero malloc con el nombre (el que llama lo libera con free) o NULL si el ejemplar no existe.
*/
char *obtenerProduccionEjemplarJSON(const char *idEjemplar) {
    cJSON *raiz = cargarArchivoJSON("data/ejemplares.json");
    if (!cJSON_IsArray(raiz)) {
        cJSON_Delete(raiz);
        return NULL;
    }

    char *nombre = NULL;
    int cantidad = cJSON_GetArraySize(raiz);
    for (int i = 0; i < cantidad && nombre == NULL; i++) {
        cJSON *ej = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(ej, "id");
        if (cJSON_IsString(id) && strcmp(id->valuestring, idEjemplar) == 0) {
            cJSON *prod = cJSON_GetObjectItem(ej, "produccion");
            if (cJSON_IsString(prod)) {
                nombre = malloc(strlen(prod->valuestring) + 1);
                if (nombre != NULL) strcpy(nombre, prod->valuestring);
            }
        }
    }

    cJSON_Delete(raiz);
    return nombre;
}

/*--------------------Devolucoiones-------------------------------*/

/*
* Objetivo: Buscar un prestamo activo segun su ID.
* Entradas: Identificador del prestamo.
* Salidas:
    - 1 si el prestamo existe y esta activo.
    - 0 si no existe o esta finalizado.
* Restricciones: Lee la informacion desde prestamos.json.
*/
int existePrestamoActivoJSON(int idPrestamo){
    cJSON *raiz = cargarArchivoJSON("data/prestamos.json");
    if(!cJSON_IsArray(raiz)){
        cJSON_Delete(raiz);
        return 0;
    }
    int cantidad = cJSON_GetArraySize(raiz);
    for(int i = 0; i < cantidad; i++){
        cJSON *prestamo = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(prestamo, "id");
        cJSON *estado = cJSON_GetObjectItem(prestamo, "estado");

        if(!cJSON_IsNumber(id) || !cJSON_IsString(estado)){
            continue;
        }
        if(id->valueint == idPrestamo && strcmp(estado->valuestring, "activo") == 0){
            cJSON_Delete(raiz);
            return 1;
        }
    }
    cJSON_Delete(raiz);
    return 0;
}

/*
* Objetivo: Obtener las fechas asociadas a un préstamo.
* Entradas: Identificador del préstamo.
* Salidas: FechaInicio y fechaEntrega.
* Retorna:
    - 1 si encuentra el préstamo.
    - 0 si no existe.
*/
int obtenerFechasPrestamoJSON(int idPrestamo, char *fechaInicio, char *fechaEntrega){
    cJSON *raiz = cargarArchivoJSON("data/prestamos.json");
    if(!cJSON_IsArray(raiz)){
        cJSON_Delete(raiz);
        return 0;
    }
    int cantidad = cJSON_GetArraySize(raiz);
    for(int i = 0; i < cantidad; i++){
        cJSON *prestamo = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(prestamo, "id");

        if(!cJSON_IsNumber(id)){
            continue;
        }
        if(id->valueint == idPrestamo){
            cJSON *inicio = cJSON_GetObjectItem(prestamo, "fecha_inicio");
            cJSON *entrega = cJSON_GetObjectItem(prestamo, "fecha_entrega");
            if(cJSON_IsString(inicio) && cJSON_IsString(entrega)){
                strcpy(fechaInicio, inicio->valuestring);
                strcpy(fechaEntrega, entrega->valuestring);
                cJSON_Delete(raiz);
                return 1;
            }
        }
    }
    cJSON_Delete(raiz);
    return 0;
}

/*
* Objetivo:  Registrar la devolucion de un prestamo.
* Entradas: Identificador del prestamo y fechaDevolucion - fecha en formato YYYY-MM-DD.
* Salidas:
    - Cambia el estado a "finalizado".
    - Agrega la fecha de devolucion.
    - Actualiza los ejemplares asociados a "Disponible".
* Retorna:
    - 1 si la operacion tuvo exito.
    - 0 si el prestamo no existe o ya fue finalizado.
*/
int finalizarPrestamoJSON(int idPrestamo, const char *fechaDevolucion){
    cJSON *raiz = cargarArchivoJSON("data/prestamos.json");
    if(!cJSON_IsArray(raiz)){
        cJSON_Delete(raiz);
        return 0;
    }
    int cantidad = cJSON_GetArraySize(raiz);
    for(int i = 0; i < cantidad; i++){
        cJSON *prestamo = cJSON_GetArrayItem(raiz, i);
        cJSON *id = cJSON_GetObjectItem(prestamo, "id");

        if(!cJSON_IsNumber(id)){
            continue;
        }
        if(id->valueint != idPrestamo){
            continue;
        }
        cJSON_ReplaceItemInObject(prestamo, "estado", cJSON_CreateString("finalizado")); // cambia el estado del prestamo de activo a finalizado
        cJSON_AddStringToObject(prestamo, "fecha_devolucion", fechaDevolucion); // registra la fecha real en que se realizo la devolucion
        cJSON *ejemplares = cJSON_GetObjectItem(prestamo, "ejemplares"); // obtiene la lista de ejemplares asociados al prestamo

        if(cJSON_IsArray(ejemplares)){
            int total = cJSON_GetArraySize(ejemplares);
            for(int j = 0; j < total; j++){
                cJSON *ejemplar = cJSON_GetArrayItem(ejemplares, j);
                if(cJSON_IsString(ejemplar)){
                    cambiarEstadoEjemplarJSON(ejemplar->valuestring, "Disponible"); // libera el ejemplar para que pueda volver a prestarse
                }
            }
        }
        escribirArchivoJSON("data/prestamos.json", raiz);
        cJSON_Delete(raiz);
        return 1;
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
    cJSON *raiz= cargarArchivoJSON("data/prestamos.json");
    if(!cJSON_IsArray(raiz)){
        cJSON_Delete(raiz);
        return 0;
    }
    int cantidad = cJSON_GetArraySize(raiz);
    for(int i = 0; i < cantidad; i++){
        cJSON *prestamo = cJSON_GetArrayItem(raiz, i);
        cJSON *usuario = cJSON_GetObjectItem(prestamo, "usuario");
        cJSON *estado = cJSON_GetObjectItem(prestamo, "estado");

        if(!cJSON_IsString(usuario) || !cJSON_IsString(estado)){
            continue;
        }
        // un usuario con prestamos activos o vencidos no puede eliminarse del sistema
        if(strcmp(usuario->valuestring, identificacion) == 0 && strcmp(estado->valuestring, "activo") == 0){ 
            cJSON_Delete(raiz);
            return 1;
        }
    }
    cJSON_Delete(raiz);
    return 0;
}