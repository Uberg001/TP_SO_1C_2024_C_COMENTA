#ifndef MEMORIA_CONEXION_IO_H
#define MEMORIA_CONEXION_IO_H

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <global.h>
#include <./main/memoria.h>
#include <pthread.h>
#include <string.h>
void ejecutarServidorIO(); 
void* ejecutarServidor(void* socketCliente) ;
int ejecutarServidorCPU(int *socketCliente);
void hacerHandshake(int socketClienteIO);
void recibirDirYCadena(int socket, int *dir, int *pid, char* cadena);
void recibirDireccionyTamano(int socket, int *dir, int *pid, int *tamano);
#endif 