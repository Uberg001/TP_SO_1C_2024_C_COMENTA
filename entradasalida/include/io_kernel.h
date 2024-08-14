#ifndef IO_KERNEL_H_
#define IO_KERNEL_H_

#include "../src/main/entradasalida.h"
#include <stdio.h>
#include <stdlib.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <configuraciones/configuraciones.h>
#include <global.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <FileSystem/filesystem.h>
void io_atender_kernel();
void recibir_mensaje_y_dormir(int socket_cliente); //lo habia puesto en utilsServidor
void manejarSTDINREAD(int socketCliente);
void manejarSTDOUTWRITE(int socketCliente);
void manejarFS_CREATE(int socketCliente);
void manejarFS_DELETE(int socketCliente);
void manejarFS_TRUNCATE(int socketCliente);
void manejarFS_WRITE(int socketCliente);
void manejarFS_READ(int socketCliente);
void enviarDireccionTamano(int direccion, int tamano, int pid, int socket); 
void enviarAImprimirAMemoria(const char *mensaje, int direccion, int socket, int pid);
void recibirEnteros3(int socket, int *tamanio, int *direccion, int *pid, int *cantidad);
#endif