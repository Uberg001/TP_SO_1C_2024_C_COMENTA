#ifndef CICLODEINSTRUCCION_H
#define CICLODEINSTRUCCION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <global.h>
#include <configuraciones/configuraciones.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <conexionMemoria/conexionMemoria.h>
#include <conexionKernel/servidorKernel.h>
#include <mmu/mmu.h>
    
extern char *listaComandos[];
extern char* instruccionAEjecutar; 
extern char** elementosInstruccion; 
extern int instruccionActual; 
extern int cantParametros;
extern int conexionAMemoria;
extern int socketClienteDispatch;
extern int socketClienteInterrupt;
extern int flag_user_interruption;
extern t_contexto* contextoEjecucion;
extern char* mensajeInterrupcion;
    
void cicloDeInstruccion();
void fetch();
void decode();
void execute();
void liberarMemoria();
t_comando check_interrupt();
int buscar(char *elemento, char **lista); 
    
char* recibirValor(int);
void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3, char * parm4, char * parm5);
int obtenerTamanioReg(char* registro);
void remove_newline(char* str);

#endif 