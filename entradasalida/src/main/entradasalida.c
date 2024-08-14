#include <main/entradasalida.h>

t_log* logger;
t_log* loggerError ;
t_config* config ;
int fd_kernel = 0;
int fd_memoria = 0;

char* TIPO_INTERFAZ ;
int TIEMPO_UNIDAD_TRABAJO ;
char* IP_KERNEL;
char* PUERTO_KERNEL ;
char* IP_MEMORIA ;
char* PUERTO_MEMORIA ;
char* PATH_BASE_DIALFS ;
int BLOCK_SIZE ;
int BLOCK_COUNT ;
int RETRASO_COMPACTACION;
// yo lo corro con: ./bin/entradasalida IntX entradasalida.config
int main(int argc, char** argv) {

    if (argc < 3) {
    	fprintf(stderr, "Usar: %s IntX </home/utnso/tp-2024-1c-Grupo-SO-/entradasalida/io.config>\n", argv[0]);
    	return EXIT_FAILURE;
	}

    iniciar_io(argv); //abrir modulo con: ./bin/entradasalida /home/utnso/tp-2024-1c-Silver-Crime/entradasalida/entradasalida.config

	atexit(terminarPrograma); //cuando se haga exit, se llamara a terminarPrograma. esto es lo que hace atexit

	conexionIOMemoria(argv);
	conexionKernel(argv);
	if (strcmp(TIPO_INTERFAZ, "DialFS") == 0){
		crearCarpetaSiNoExiste(PATH_BASE_DIALFS);
    	create_bitmap_file("bitmap.dat", BLOCK_COUNT/8);
    	create_bloques_file("bloques.dat", BLOCK_COUNT*BLOCK_SIZE);
	}
	pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) io_atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);	
	//pthread_t hilo_memoria;
	//pthread_create(&hilo_memoria, NULL, (void*) io_atender_memoria, NULL);
	//pthread_join(hilo_memoria, NULL);
	//io_atender_kernel();
	return EXIT_SUCCESS;
	 
}

void crearCarpetaSiNoExiste(const char *path) {
    struct stat st = {0};

    // Verificar si la carpeta existe
    if (stat(path, &st) == -1) {
        // Crear la carpeta con permisos 0755
        if (mkdir(path, 0755) == 0) {
            log_info(logger,"Carpeta creada; %s", path);
        } else {
            perror("Error al crear la carpeta");
        }
    } else {
        log_info(logger,"La carpeta existe: %s", path);
    }
}