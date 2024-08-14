#ifndef PLANIFICACION_PROCESOS_H
#define PLANIFICACION_PROCESOS_H

#include <pthread.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <utilsCliente/utilsCliente.h>
#include <global.h>
#include <main/configuraciones.h>
#include <conexiones/conexionCPU.h>
#include <conexiones/conexionMemoria.h>
#include <peticiones/pcb.h>
#include <peticiones/syscalls.h>
#include <planificacion/algoritmosCortoPlazo.h>


extern t_list* pcbsNEW; 
extern t_list* pcbsREADY;
extern t_list *pcbsBloqueados;
extern t_list *pcbsParaExit;
extern t_list *pcbsREADYaux;
extern t_list *pcbsExec;

extern sem_t hayProcesosReady;
extern sem_t hayProcesosNuevos;
extern pthread_mutex_t mutexListaNew;
extern pthread_mutex_t mutexListaReady;
extern pthread_mutex_t mutexListaExit;
extern pthread_mutex_t mutexListaBloqueados;
extern pthread_mutex_t mutexPCB; 
extern sem_t semGradoMultiprogramacion; 
extern int gradoMultiprogramacion; 
extern int *instanciasRecursos;
extern bool pausaPlanificacion;
extern int flag_exit;

extern pthread_mutex_t pausaMutex;
extern pthread_cond_t pausaCond;

extern char *estadosProcesos[5];

void planificarALargoPlazo();
void planificarACortoPlazo(t_pcb* (*proximoAEjecutar)());
void inicializarSemaforos();
void destruirSemaforos();
void ingresarANew(t_pcb *pcb); 
t_pcb *obtenerSiguienteAReady();

void ingresarAReady(t_pcb *pcb);
void ingresarAReadyAux(t_pcb *pcb);

void loggearCambioDeEstado(uint32_t pid, estadoProceso anterior, estadoProceso actual); 

void crearColasBloqueo();

#endif 