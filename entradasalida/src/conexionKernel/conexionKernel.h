#ifndef IO_KERN_SER_H
#define IO_KERN_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>

#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <./main/entradasalida.h>

void conexionKernel(char **argv); 
void enviarHandshake(char *nombreInterfaz);
#endif