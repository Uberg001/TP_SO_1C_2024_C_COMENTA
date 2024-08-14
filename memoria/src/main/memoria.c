#include <main/memoria.h>

int sockets[2];
pthread_t threadCPU, threadKernel, threadIO;

t_log* logger; 
t_log* loggerError; 
t_config* config; 
MemoriaFisica *mf;
int server_fd =0;
int main() {
    logger = iniciarLogger ("memoria.log", "Memoria");
	loggerError = iniciarLogger ("memoriaErrores.log","Memoria (Errores)"); 
	config = iniciarConfiguracion ("memoria.config");
    mf = inicializar_memoria_fisica(TAM_PAGINA);
	atexit (terminarPrograma);
	log_info (logger, "Memoria lista para recibir conexiones.");
  
	server_fd = iniciarServidor (confGet("PUERTO_ESCUCHA"));
  
	sockets[0] = esperarCliente(server_fd);
	log_info(logger, "Memoria conectada a Módulo, en socket: %d", sockets[0]);
  
	sockets[1] = esperarCliente (server_fd);
	log_info(logger, "Memoria conectada a Módulo, en socket: %d", sockets[1]);
  
    // Creación de hilos
    int opCodeCPU = pthread_create(&threadCPU, NULL, (void*)ejecutarServidorCPU, (void*)&sockets[0]);
    int opCodeIO = pthread_create(&threadIO, NULL, (void*)ejecutarServidorIO, NULL);
    int opCodeKernel = pthread_create(&threadKernel, NULL, (void*)ejecutarServidorKernel, (void*)&sockets[1]);

    // Verificación de errores en la creación de hilos
    if (opCodeCPU) {
        error("Error en iniciar el servidor a CPU");
    }
    if (opCodeIO) {
        error("Error en iniciar el servidor a IO");
    }
    if (opCodeKernel) {
        error("Error en iniciar el servidor a Kernel");
    }

    // Espera a que los hilos terminen
    pthread_join(threadCPU, NULL);
    pthread_join(threadIO, NULL);
    pthread_join(threadKernel, NULL);

    exit(0);
}