/* KERNEL- cliente | CPU - servidor*/
#include <conexiones/conexionCPU.h>
#include <pthread.h>

int conexionACPU;
int conexionACPUInterrupt;
sem_t memoriaOK;
pthread_mutex_t mutexProcesoEnEjecucion;

void conexionCPU() {
    while(1){
       
        conexionACPU = crearConexion(confGet("IP_CPU"), confGet("PUERTO_CPU_DISPATCH"));
        if(conexionACPU != -1){
           break;
        }
        else {
            log_warning(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionACPU);
            sleep(5);
        }
    }

    //CONEXION CPU INTERRUPT
    while(1){
        conexionACPUInterrupt = crearConexion(confGet("IP_CPU"), confGet("PUERTO_CPU_INTERRUPT"));
        
        if(conexionACPUInterrupt != -1){
            break;
        }
        else {
            log_warning(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionACPUInterrupt);
            sleep(5);
        }
    }
}

int recibirOperacionDeCPU(){ 
	int cod_op;
    
	if (recv(conexionACPUInterrupt, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(conexionACPUInterrupt);
		return -1;
	}
}

//Enviar proceso a CPU
t_contexto* procesarPCB(t_pcb* procesoEnEjecucion) {
    //logger= cambiarNombre(logger, "Kernel-Enviado de contexto");
    if (contextoEjecucion != NULL) destroyContextoUnico ();
	iniciarContexto ();
    
    sem_wait(&memoriaOK);
    pthread_mutex_lock(&mutexProcesoEnEjecucion);
    asignarPCBAContexto(procesoEnEjecucion);
   
    //dictionary_iterator(contextoEjecucion->registrosCPU, log_registro);

    enviarContextoBeta(conexionACPU, contextoEjecucion);

    if (recibirOperacionDeCPU() < 0) error ("Se desconecto la CPU.");
    
    recibirContextoBeta(conexionACPUInterrupt);
    
    actualizarPCB(procesoEnEjecucion);
    pthread_mutex_unlock(&mutexProcesoEnEjecucion);
    sem_post(&memoriaOK);
    return contextoEjecucion;
}

 void log_registro(char *key, void *value) {
        log_info(logger, "Registro %s: %s", key, (char*)value);
    }

void actualizarPCB(t_pcb* proceso){
    proceso->pid = contextoEjecucion->pid;
    proceso->programCounter = contextoEjecucion->programCounter;
    proceso->numeroInstrucciones=contextoEjecucion->instruccionesLength;
	dictionary_destroy_and_destroy_elements(proceso->registrosCPU, free);
    proceso->registrosCPU = registrosDelCPU(contextoEjecucion->registrosCPU);
    proceso->tiempoDeUsoCPU=contextoEjecucion->tiempoDeUsoCPU;
    proceso->algoritmo=contextoEjecucion->algoritmo;
    
    if(proceso->algoritmo != FIFO){
        proceso->quantum=contextoEjecucion->quantum;
        proceso->fin_de_quantum=contextoEjecucion->fin_de_quantum;
    }
}

void asignarPCBAContexto(t_pcb* proceso){
    contextoEjecucion->instruccionesLength = proceso->numeroInstrucciones;
    contextoEjecucion->pid = proceso->pid;
    contextoEjecucion->programCounter = proceso->programCounter;
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    contextoEjecucion->registrosCPU = registrosDelCPU(proceso->registrosCPU);
    contextoEjecucion->tiempoDeUsoCPU=proceso->tiempoDeUsoCPU;
    contextoEjecucion->algoritmo = proceso->algoritmo;
    if(contextoEjecucion->algoritmo != FIFO){
        contextoEjecucion->quantum=proceso->quantum;
        //log_info(logger, "Quantum: %ld", contextoEjecucion->quantum);
        contextoEjecucion->fin_de_quantum=proceso->fin_de_quantum;
    }
}

t_dictionary *registrosDelCPU(t_dictionary *aCopiar) {
    t_dictionary *copia = dictionary_create();
    // Allocate memory for 1-byte registers
    char* AX = malloc(4);
    char* BX = malloc(4);
    char* CX = malloc(4);
    char* DX = malloc(4);
    // Allocate memory for 4-byte registers
    char* EAX = malloc(11);
    char* EBX = malloc(11);
    char* ECX = malloc(11);
    char* EDX = malloc(11);
    char* SI=malloc(11);
    char* DI=malloc(11);
    // Copy values from the original dictionary
    strncpy(AX, (char *)dictionary_get(aCopiar, "AX"), 3);
    strncpy(BX, (char *)dictionary_get(aCopiar, "BX"), 3);
    strncpy(CX, (char *)dictionary_get(aCopiar, "CX"), 3);
    strncpy(DX, (char *)dictionary_get(aCopiar, "DX"), 3);
    strncpy(EAX, (char *)dictionary_get(aCopiar, "EAX"), 10);
    strncpy(EBX, (char *)dictionary_get(aCopiar, "EBX"), 10);
    strncpy(ECX, (char *)dictionary_get(aCopiar, "ECX"), 10);
    strncpy(EDX, (char *)dictionary_get(aCopiar, "EDX"), 10);
    strncpy(SI, (char *)dictionary_get(aCopiar, "SI"), 10);
    strncpy(DI, (char *)dictionary_get(aCopiar, "DI"), 10);

    // Ensure the strings are null-terminated
    AX[3] = '\0';
    BX[3] = '\0';
    CX[3] = '\0';
    DX[3] = '\0';
    EAX[10] = '\0';
    EBX[10] = '\0';
    ECX[10] = '\0';
    EDX[10] = '\0';
    SI[10]='\0';
    DI[10]='\0';

    // Put values into the new dictionary
    dictionary_put(copia, "AX", AX);
    dictionary_put(copia, "BX", BX);
    dictionary_put(copia, "CX", CX);
    dictionary_put(copia, "DX", DX);
    dictionary_put(copia, "EAX", EAX);
    dictionary_put(copia, "EBX", EBX);
    dictionary_put(copia, "ECX", ECX);
    dictionary_put(copia, "EDX", EDX);
    dictionary_put(copia, "SI", SI);
    dictionary_put(copia, "DI", DI);
    return copia;
}