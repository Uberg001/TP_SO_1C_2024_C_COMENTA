#ifndef ALGORITMOS_CORTO_PLAZO_H
#define ALGORITMOS_CORTO_PLAZO_H

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <commons/collections/queue.h>
#include <string.h>
#include <global.h>
#include <main/configuraciones.h>
#include <peticiones/pcb.h>
#include <planificacion/planificacion.h>


extern t_list* pcbsNEW; 
extern t_list* pcbsREADY;
extern t_list* pcbsREADYaux; //Cola auxiliar de mayor prioridad para VRR
extern int64_t rafagaCPU;
extern t_log* loggerError;


void planificarACortoPlazoSegunAlgoritmo();


t_pcb* proximoAEjecutarFIFO();
t_pcb* proximoAEjecutarVRR();
t_pcb *proximoAEjecutarRR();

void listarPIDS(t_list* pcbs); 
void detenerYDestruirCronometro(t_temporal* ); 


#endif 