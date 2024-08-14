#ifndef CONEXION_MEMORIA_H
#define CONEXION_MEMORIA_H

#include <commons/config.h>
#include <commons/log.h>

#include <configuraciones/configuraciones.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>

extern t_log* logger;
extern t_log* loggerError; 
extern t_config* config;

int conexionMemoria();

#endif 