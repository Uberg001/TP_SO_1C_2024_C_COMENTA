#ifndef PCB_H_
#define PCB_H_

#include <stdlib.h>
#include <string.h> 
#include <stdint.h> 
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <global.h>
#include <main/configuraciones.h>
#include <contextoEjecucion/contextoEjecucion.h>

typedef enum estadoProceso{
    NEW, 
    READY,
    EXEC,
    BLOCKED,
    SALIDA
} estadoProceso; 

typedef struct {
    uint32_t pid; 
    uint32_t programCounter;  
    estadoProceso estado; 
    t_dictionary* registrosCPU;
    int64_t tiempoDeUsoCPU; 
    t_list* recursosAsignados;
    int64_t quantum;
    t_algoritmo algoritmo;
    bool fin_de_quantum;
    int numeroInstrucciones;
} t_pcb; 

extern t_list *pcbsNEW;
extern t_list *pcbsREADY;
extern t_list *pcbsREADYaux;
extern t_list *pcbsExec;
extern t_list *pcbsBloqueados;
extern t_list *pcbsParaExit;
extern t_log* logger;
extern int32_t procesosCreados;
extern char* pidsInvolucrados; 
extern pthread_mutex_t list_mutex;

t_pcb* crearPCB();

void destruirPCB(t_pcb* pcb);
void destruirPCBExit(t_pcb* pcb);

t_dictionary* crearDiccionarioDeRegistros(); 

void inicializarListasPCBs(); 

void destruirListasPCBs();
void destruirListasPCBsExit();
void destruirListaPCB(t_list* pcbs);
void encolar(t_list* pcbs,t_pcb* pcb);
t_pcb* desencolar(t_list* pcbs);

void agregarPID(void *value); 
void listarPIDS(t_list *pcbs);
void imprimirListaPCBs(t_list *pcbs);
void imprimirListaExit(t_list *idsExit);
bool buscarProceso(t_list* lista, int pid);
void eliminarProceso(t_list* lista, int pid);


#endif