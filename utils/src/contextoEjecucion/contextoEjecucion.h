#ifndef CONTEXTO_EJECUCION_H
#define CONTEXTO_EJECUCION_H

#include <global.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <commons/temporal.h>

#include "stdlib.h"
#include "stdint.h"
#include "math.h"

typedef struct {

        t_comando motivo; 
        int parametrosLength;
        char* parametros[5]; 

}t_motivoDeDesalojo; 

typedef enum{
    FIFO,
    RR,
    VRR,
} t_algoritmo;

typedef struct {
    uint32_t pid; 
    int programCounter;
    uint32_t instruccionesLength;
    t_dictionary* registrosCPU;   
    t_motivoDeDesalojo* motivoDesalojo;
    int64_t tiempoDeUsoCPU;
    int64_t quantum;
    t_algoritmo algoritmo;
    bool fin_de_quantum;
} t_contexto;

extern t_contexto* contextoEjecucion;
extern int socketCliente;

void iniciarContexto();
void destroyContexto();
void destroyContextoUnico();
void enviarContextoBeta(int socket, t_contexto* contexto);
void recibirContextoBeta(int socket);
#endif