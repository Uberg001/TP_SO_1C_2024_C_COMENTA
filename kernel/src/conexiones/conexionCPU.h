#ifndef CONEXION_CPU_H
#define CONEXION_CPU_H

#include <commons/config.h>
#include <commons/string.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <global.h>
#include "../src/peticiones/pcb.h"
#include <conexiones/conexionMemoria.h>
#include <semaphore.h>

extern int conexionACPU;
extern int conexionACPUInterrupt;
extern t_log* loggerError; 
extern t_contexto* contextoEjecucion;
extern sem_t memoriaOK;
extern pthread_mutex_t mutexProcesoEnEjecucion;

void conexionCPU();
void asignarPCBAContexto(t_pcb* procesoEnEjecucion);
void actualizarPCB(t_pcb* procesoEnEjecucion);
t_contexto* procesarPCB(t_pcb* procesoEnEjecucion);
void log_registro(char *key, void *value);

t_dictionary *registrosDelCPU(t_dictionary *aCopiar);

#endif /* CONEXION_CPU_H */