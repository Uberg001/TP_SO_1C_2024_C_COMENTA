#include <peticiones/pcb.h>
#include "../conexiones/conexionMemoria.h"
#include <planificacion/planificacion.h>
#include <pthread.h>
char* pidsInvolucrados; 
pthread_mutex_t list_mutex;
//Básicos PCB

t_pcb *crearPCB(){
    procesosCreados++;

    t_pcb *nuevoPCB = malloc(sizeof(t_pcb));
    nuevoPCB->estado = NEW;
    nuevoPCB->pid = procesosCreados;
    nuevoPCB->programCounter = 0;
    nuevoPCB->registrosCPU = crearDiccionarioDeRegistros();
    nuevoPCB->recursosAsignados = list_create();
    nuevoPCB->fin_de_quantum=false;
    nuevoPCB->numeroInstrucciones=0;
    recibirEstructurasInicialesMemoria(nuevoPCB); 
    return nuevoPCB;
}

void destruirPCB(t_pcb *pcb){
    int pid_copia = pcb->pid;
    pthread_mutex_lock(&mutexListaExit);
    list_add(pcbsParaExit, (void*)(uintptr_t)pid_copia);
    pthread_mutex_unlock(&mutexListaExit);
    dictionary_destroy_and_destroy_elements(pcb->registrosCPU, free);
    free(pcb->recursosAsignados);
    free(pcb);
}

t_dictionary *crearDiccionarioDeRegistros(){

    t_dictionary *registros = dictionary_create();
    dictionary_put(registros,"AX", string_repeat('0', 1));
    dictionary_put(registros,"BX", string_repeat('0', 1));
    dictionary_put(registros,"CX", string_repeat('0', 1));
    dictionary_put(registros,"DX", string_repeat('0', 1));
    dictionary_put(registros,"EAX", string_repeat('0', 4));
    dictionary_put(registros,"EBX", string_repeat('0', 4));
    dictionary_put(registros,"ECX", string_repeat('0', 4));
    dictionary_put(registros,"EDX", string_repeat('0', 4));
    dictionary_put(registros,"SI",string_repeat('0',4));
    dictionary_put(registros,"DI",string_repeat('0',4));

    return registros;
}


// Lista de PCBs

void inicializarListasPCBs(){
    pcbsNEW = list_create();
    pcbsREADY = list_create();
    pcbsREADYaux=list_create();
    pcbsExec = list_create();
    pcbsBloqueados=list_create();
    pcbsParaExit=list_create();
}

void destruirListasPCBs () {
    destruirListaPCB(pcbsNEW);
    destruirListaPCB(pcbsREADY);
    destruirListaPCB(pcbsExec);
    destruirListaPCB(pcbsBloqueados);
    destruirListaPCB(pcbsREADYaux);
    destruirListaPCB(pcbsParaExit);
}

void destruirListaPCB(t_list *pcbs){
    list_destroy_and_destroy_elements(pcbs, (void *)destruirPCB);
}

void destruirListaPCBExit(t_list *pcbs){
    list_destroy_and_destroy_elements(pcbs, (void *)destruirPCBExit);
}

void destruirListasPCBsExit () {
    destruirListaPCBExit(pcbsNEW);
    destruirListaPCBExit(pcbsREADY);
    destruirListaPCBExit(pcbsExec);
    destruirListaPCBExit(pcbsBloqueados);
    destruirListaPCBExit(pcbsREADYaux);
    destruirListaPCBExit(pcbsParaExit);
}

void destruirPCBExit(t_pcb *pcb){
    dictionary_destroy_and_destroy_elements(pcb->registrosCPU, free);
    free(pcb->recursosAsignados);
    free(pcb);
}

void encolar(t_list *pcbs, t_pcb *pcb){
    list_add(pcbs, (void *)pcb);
}

t_pcb *desencolar(t_list *pcbs){
    pthread_mutex_lock(&list_mutex);
    t_pcb *pcb = (t_pcb *)list_remove(pcbs, 0);
    pthread_mutex_unlock(&list_mutex);
    return pcb;
}

void agregarPID(void *value){
    t_pcb *pcb = (t_pcb *)value;
    char *pid = string_itoa(pcb->pid);
    string_append_with_format(&pidsInvolucrados, " %s ", pid);
    free (pid);
}

void listarPIDS(t_list *pcbs) {
    pthread_mutex_lock(&list_mutex);
    list_iterate(pcbs, agregarPID);
    pthread_mutex_unlock(&list_mutex);
}

void imprimirListaPCBs(t_list *pcbs){
    //logger=cambiarNombre(logger,"Kernel-Lista PCBs");
    for(int i = 0; i < list_size(pcbs); i++){
        t_pcb *pcb = list_get(pcbs, i);
        log_info(logger, "PID: %d", pcb->pid);
    }
}

void imprimirListaExit(t_list *idsExit){
    //logger=cambiarNombre(logger,"Kernel-Lista Exit");
    for(int i = 0; i < list_size(idsExit); i++){
        log_info(logger, "PID: %d", (int)(uintptr_t)list_get(idsExit, i));
    }
}

bool buscarProceso(t_list* lista, int pid) {
    bool encontrado = false;
    pthread_mutex_lock(&list_mutex);
    for (int i = 0; i < list_size(lista); i++) {
        t_pcb* pcb = list_get(lista, i);
        if (pcb->pid == pid) {
            encontrado = true;
            break;
        }
    }
    pthread_mutex_unlock(&list_mutex);
    return encontrado;
}

void eliminarProceso(t_list* lista, int pid){
    char* interrupted_by_user="INTERRUPTED_BY_USER";

    for (int i = 0; i < list_size(lista); i++) {
        t_pcb* pcb = list_get(lista, i);
        if (pcb->pid == pid) {
            pthread_mutex_lock(&list_mutex);
            list_remove(lista, i);
            pthread_mutex_unlock(&list_mutex);
            exit_s(pcb,&interrupted_by_user);
            break;
        }
    }
    
}







