#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include <global.h>
#include <configuraciones/configuraciones.h>
#include <planificacion/planificacion.h>
#include <peticiones/pcb.h>
#include <conexiones/conexionMemoria.h>

int ejecutarConsola ();
void ejecutarScript(const char*);
void iniciarProceso(const char*);
void finalizarProceso(int);
void detenerPlanificacion();
void iniciarPlanificacion();
void procesoEstado();
void modificarGradoMultiprogramacion(int);
void enviarPathDeInstrucciones(const char *path);
extern t_log* logger;

#endif /* CONSOLA_H_ */
