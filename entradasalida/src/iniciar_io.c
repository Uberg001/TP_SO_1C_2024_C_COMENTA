#include <../include/iniciar_io.h>

void iniciar_io(char** argv){
    iniciar_logs();
    iniciar_configs(argv[2]);
    mostrar_configs();
}

void iniciar_logs(){
    logger = log_create("entradasalida.log", "IO", true, LOG_LEVEL_INFO);
	if( logger == NULL){
		perror ("error al crear log");
		exit(EXIT_FAILURE);
	}else{
        //log_info (logger, "[IO] Log iniciado correctamente");
    }

    loggerError = log_create("errores.log", "ERROR_IO", true, LOG_LEVEL_INFO);
	if( loggerError == NULL){
		perror ("error al crear log error");
		exit(EXIT_FAILURE);
	}else{
        //log_info (loggerError, "[IO] Error Log inciado correctamente");
    }
}

void iniciar_configs(char* arg){
    config = config_create(arg);
    
    if (config == NULL) {
        // No se pudo crear el config, terminemos el programa
		perror("Error al intentar cargar el config.");
        exit(EXIT_FAILURE);
    }else{
        //log_info (logger, "[IO] Configuracion obtenida correctamente");
    }
    TIPO_INTERFAZ = config_get_string_value(config, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
    IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(config, "PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(config, "BLOCK_COUNT");
    RETRASO_COMPACTACION = config_get_int_value(config, "RETRASO_COMPACTACION");
}

void mostrar_configs(){
    log_info(logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);
    /*log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", TIEMPO_UNIDAD_TRABAJO);
    log_info(logger, "IP_KERNEL: %s", IP_KERNEL);
    log_info(logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    log_info(logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    log_info(logger, "BLOCK_SIZE: %d", BLOCK_SIZE);
    log_info(logger, "BLOCK_COUNT: %d", BLOCK_COUNT);*/
}