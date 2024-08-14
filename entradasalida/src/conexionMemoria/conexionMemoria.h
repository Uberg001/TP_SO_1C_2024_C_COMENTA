#ifndef IO_MEM_SER_H
#define IO_MEM_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>

#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <./main/entradasalida.h>

void conexionIOMemoria(char **argv);
void enviarHandshakeMemoria();
#endif