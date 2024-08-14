#ifndef KERNEL_H_
#define KERNEL_H_

#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <conexiones/conexionMemoria.h>
#include <conexiones/conexionCPU.h>
#include <main/configuraciones.h>
#include <consola/consola.h>
#include <escuchaIO/servidorIO.h>
#include <configuraciones/configuraciones.h>

extern int socketCliente;
extern t_log* logger;
extern t_log* loggerError;
extern t_config* config;
extern pthread_t planificadorLargoPlazo_h, planificadorCortoPlazo_h, ejecutarConsola_h;
extern pthread_mutex_t mutex_lista_global;
extern t_list* lista_global_io;
#endif