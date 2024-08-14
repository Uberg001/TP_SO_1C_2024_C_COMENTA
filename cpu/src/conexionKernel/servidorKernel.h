#ifndef CPU_KERN_SER_H
#define CPU_KERN_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/temporal.h>

#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <cicloDeInstruccion/cicloDeInstruccion.h>

extern t_log* loggerError; 

void escucharAlKernel(); 
int ejecutarServidorCPU(int socketCliente);
extern int socketClienteDispatch;
extern int socketClienteInterrupt;
extern t_contexto* contextoEjecucion;
extern int flag_bloqueante;
extern t_temporal* tiempoDeUsoCPU;
void atenderInterrupcionDeUsuario();
#endif