#include <main/kernel.h>
#include "../src/planificacion/planificacion.h"

int socketCliente;
t_log* logger;
t_log* loggerError;
t_config* config;
pthread_t planificadorLargoPlazo_h, planificadorCortoPlazo_h, ejecutarConsola_h, ejecutarIO_h;
Kernel_io kernel;
void escucharAlIO();
pthread_mutex_t mutex_lista_global;
t_list * lista_global_io;
int main () {
    //Inicializar variables
    logger = iniciarLogger("kernel.log", "Kernel");
	loggerError = iniciarLogger("errores.log", "Errores - Kernel"); 
    config = iniciarConfiguracion("kernel.config");
	atexit (terminarPrograma);

	inicializarSemaforos();
	atexit (destruirSemaforos);
	inicializarListasPCBs(); 
	atexit (destruirListasPCBsExit);
	inicializarStructsIO(&kernel);
	//atexit (destruirStructsIO);

	conexionMemoria(); 
	conexionCPU();
	
    char * nombre = string_duplicate("CPU-KERNEL");
	//cambiarNombre(logger, nombre);
	free (nombre);

    //ejecutarConsola();
	lista_global_io = list_create();
    pthread_mutex_init(&mutex_lista_global, NULL) ;
    //Inicializar Hilos
	int opCodes [4] = {
		pthread_create(&planificadorLargoPlazo_h, NULL, (void*) planificarALargoPlazo, NULL),
		pthread_create(&planificadorCortoPlazo_h, NULL, (void*) planificarACortoPlazoSegunAlgoritmo, NULL),
		pthread_create(&ejecutarConsola_h,NULL,(void*)ejecutarConsola,NULL),
		pthread_create(&ejecutarIO_h,NULL,(void*) escucharAlIO,NULL)
	};

    if (opCodes [0]) {
        log_error (logger,"Error al generar hilo para el planificador de largo plazo, terminando el programa.");	
	}
	if (opCodes [1]) {
        log_error (logger,"Error al generar hilo para el planificador de corto plazo, terminando el programa.");
	}
	if (opCodes [2]){
		log_error(logger,"Error al generar hilo para ejecutar la consola, terminando el programa.");
	}
	if (opCodes [3]){
		log_error(logger,"Error al generar hilo para ejecutar el IO, terminando el programa.");
	}
		
	//Hilo Planificador Largo Plazo -> Mueve procesos de NEW a READY
	pthread_join(planificadorLargoPlazo_h,NULL);
	//Hilo Planificador Corto Plazo --> Mueve procesos de READY a EXEC
	pthread_join(planificadorCortoPlazo_h,NULL);
	//Hilo para consola	
	pthread_join(ejecutarConsola_h,NULL);
	//Hilo para IO
	pthread_join(ejecutarIO_h,NULL);
	destruirStructsIO(&kernel);
    exit (0);
}