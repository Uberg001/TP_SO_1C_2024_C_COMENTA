#ifndef INICIAR_IO_H_
#define INICIAR_IO_H_
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <./main/entradasalida.h>


void iniciar_io(char** argv);
void iniciar_logs();
void iniciar_configs(char* argv);
void mostrar_configs();
#endif