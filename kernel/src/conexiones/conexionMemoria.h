#ifndef CONEXION_MEMORIA_H
#define CONEXION_MEMORIA_H

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include "../peticiones/pcb.h"

void conexionMemoria();
void recibirEstructurasInicialesMemoria(t_pcb* pcb); 
void liberarMemoriaPCB(t_pcb* proceso);

extern t_log* loggerError; 
extern int numeroInstrucciones;

#endif /* CONEXION_MEMORIA_H */
