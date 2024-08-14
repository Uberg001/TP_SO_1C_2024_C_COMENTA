#include <escuchaKernel/servidorKernel.h>
#include <pthread.h>

int socketClienteDispatch;
int socketClienteInterrupt;
t_temporal* tiempoDeUsoCPU;

void escucharAlKernel() {
    char *puertoEscuchaDispatch = confGet("PUERTO_ESCUCHA_DISPATCH");

    log_info(logger,"Esperando conexiones con KERNEL...");
    
    socketClienteDispatch = alistarServidor(puertoEscuchaDispatch);
	log_info(logger, "Kernel conectado (dispatch), en socket: %d",socketClienteDispatch);
    
    char *puertoEscuchaInterrupt = confGet("PUERTO_ESCUCHA_INTERRUPT"); 
    socketClienteInterrupt = alistarServidor(puertoEscuchaInterrupt);
	log_info(logger, "Kernel conectado (interrupt), en socket: %d", socketClienteInterrupt);

    log_info(logger,"Conexiones CPU-Kernel OK!");
    
	ejecutarServidorCPU(socketClienteDispatch);
}



//CPU recibe instrucciones del Kernel para hacer el ciclo de instruccion
int ejecutarServidorCPU(int socketCliente){
	pthread_t atenderInterrupcionKernel;
	pthread_create(&atenderInterrupcionKernel,NULL, (void*) atenderInterrupcionDeUsuario,NULL);
	while(1){
    instruccionActual = -1;
	int codOP = recibirOperacion(socketCliente);
		switch (codOP) {
				case -1:
					log_error(logger, "El Kernel se desconecto.");
					if (contextoEjecucion != NULL){
						destroyContexto ();
					}
					return EXIT_FAILURE;
				case CONTEXTOEJECUCION:
					recibirContextoBeta(socketCliente);
					tiempoDeUsoCPU=temporal_create();
					 flag_bloqueante = 0;
					 while(contextoEjecucion->programCounter < contextoEjecucion->instruccionesLength && flag_bloqueante == 0) {
					 	cicloDeInstruccion();
					 } 
					
					break;
				default:
					log_warning(loggerError,"Operacion desconocida.");
						break;
			}
	}
	pthread_join(atenderInterrupcionKernel,NULL);
	return EXIT_SUCCESS;
}

void atenderInterrupcionDeUsuario(){
	while(1){
		int peticion = recibirOperacion(socketClienteInterrupt);
		if(peticion==150){
			mensajeInterrupcion = recibirMensaje(socketClienteInterrupt);	
		}
	}
}