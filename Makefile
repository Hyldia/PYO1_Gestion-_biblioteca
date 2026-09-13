# Compila el programa

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/inicio.c src/usuarios.c src/persistencia.c src/catalogo.c src/prestamos.c src/devoluciones.c src/busquedas.c src/estadisticas.c
all: 
	$(CC) $(CFLAGS) $(SRC) -o biblioteca -lcjson
clean: 
	rm -f biblioteca