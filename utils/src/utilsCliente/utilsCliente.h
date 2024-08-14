#ifndef UTILS_CLIENTE_H
#define UTILS_CLIENTE_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <configuraciones/configuraciones.h>

#define SEGS_ANTES_DE_REINTENTO 3

extern t_log *logger;
extern t_config *config;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef enum
{	
	MENSAJE,
	PAQUETE, 
	CONTEXTOEJECUCION,
	READ, 
	WRITE,
	NEWPCB, 
    ENDPCB,
	SUCCESS,
	MMU,
} op_code;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;

} t_paquete;

int crearConexion(char* ip, char* puerto);
void enviarMensaje(char* mensaje, int socketCliente);
t_paquete* crearPaquete(void);
void agregarAPaquete(t_paquete* paquete, void* valor, int tamanio);
void enviarPaquete(t_paquete* paquete, int socketCliente);
void eliminarPaquete(t_paquete* paquete);

void enviarCodOp (op_code codigoDeOperacion, int socket);
int conexion(char *SERVIDOR);
void crearBuffer(t_paquete *paquete);
void *serializarPaquete(t_paquete *paquete, int bytes);
void enviarMensaje2(char *mensaje, int socket);
#endif /* UTILS_H_ */