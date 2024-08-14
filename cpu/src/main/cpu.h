#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

#include <configuraciones/configuraciones.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>

#include <configuraciones/configuraciones.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <conexionMemoria/conexionMemoria.h>
#include <conexionKernel/servidorKernel.h>
#include <mmu/mmu.h>


t_log* logger;
t_log* loggerError; 
t_config* config;

extern int conexionAMemoria;
#endif 