#include <cicloDeInstruccion/cicloDeInstruccion.h>
#include <inttypes.h>
#include <limits.h>
#include <commons/memory.h>
char *listaComandos[] = {
    [SET] = "SET",
    [MOV_IN] = "MOV_IN",
    [MOV_OUT] = "MOV_OUT",
    [SUM]="SUM",
    [SUB]="SUB",
    [JNZ]="JNZ",
    [RESIZE]="RESIZE",
    [COPY_STRING]="COPY_STRING",
    [IO_GEN_SLEEP]="IO_GEN_SLEEP",
    [IO_STDIN_READ]="IO_STDIN_READ",
    [IO_STDOUT_WRITE]="IO_STDOUT_WRITE",
    [IO_FS_CREATE]="IO_FS_CREATE",
    [IO_FS_DELETE]="IO_FS_DELETE",
    [IO_FS_TRUNCATE]="IO_FS_TRUNCATE",
    [IO_FS_WRITE]="IO_FS_WRITE",
    [IO_FS_READ]="IO_FS_READ",
    [WAIT] = "WAIT",
    [SIGNAL] = "SIGNAL",
    [EXIT] = "EXIT"
};

char* instruccionAEjecutar; 
char** elementosInstruccion; 
int instruccionActual; 
int cantParametros;
int flag_user_interruption=0;
char* mensajeInterrupcion="";  

void cicloDeInstruccion(){
    fetch();
    decode();
    execute();
    liberarMemoria();
}
void solicitarInstruccion(int pid, int indice, int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 2*sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &indice, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); // Manejo de error, puedes ajustarlo según tu aplicación
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
	free(a_enviar);
	free(paquete);
}

void solicitudResize(int pid, int tamanio, int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = RESIZE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 2*sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &tamanio, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE);
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
	free(a_enviar);
	free(paquete);
}

// ------- Funciones del ciclo ------- //
void fetch() { 
    log_info(logger, "PID: <%d> - FETCH - Program Counter: <%d>", contextoEjecucion->pid, contextoEjecucion->programCounter);
    //Mando la linea que voy a leer de la lista de instrucciones de memoria
    int pid=contextoEjecucion->pid;
    int numInstruccionABuscar = contextoEjecucion->programCounter;
    solicitarInstruccion(pid, numInstruccionABuscar, conexionAMemoria);
    //Recibo la instruccion
    int peticion = recibirOperacion(conexionAMemoria);
	switch (peticion) {
		case MENSAJE:
			instruccionAEjecutar=recibirMensaje(conexionAMemoria);
            //log_info(logger, "Instruccion recibida: %s", instruccionAEjecutar);
			break;
        default:
            log_warning(logger,"Operacion desconocida.");
			break;
	}
    contextoEjecucion->programCounter += 1;
}

void decode(){
    // Eliminar el salto de línea al final de instruccionAEjecutar
    remove_newline(instruccionAEjecutar);
    //log_info(logger, "inicio decode con instruccionAEjecutar: %s", instruccionAEjecutar);
    elementosInstruccion = string_n_split(instruccionAEjecutar, 6, " ");
    //log_info(logger, "instruccion a ejecutar: %s", elementosInstruccion[0]); 
    cantParametros = string_array_size(elementosInstruccion) - 1; 
    //log_info(logger, "cantParametros: %d", cantParametros);
    instruccionActual = buscar(elementosInstruccion[0], listaComandos); 
    //log_info(logger, "instruccion Actual: %d", instruccionActual);
    //free(instruccionAEjecutar); // creo que va aca
}

// Función para eliminar el salto de línea al final de la cadena
void remove_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int buscar(char *elemento, char **lista) {
    int i = 0;
    while (i <= string_array_size (lista)) {
        if (i < string_array_size (lista))
            if (!strcmp (elemento, lista[i]))
                return i;
        i++;
    }
    return (i > string_array_size(lista)) ? -1 : i;
}
 
t_comando check_interrupt(){
    if(strcmp(mensajeInterrupcion,"USER_INTERRUPT")==0){
        //log_warning(logger,"INTERRUPCION ASINCRONICA");
        return USER_INTERRUPTION;
    }
    if(contextoEjecucion->algoritmo != FIFO){
        //log_info(logger, "inicio check_interrupt");
        int64_t quantum=contextoEjecucion->quantum;
        //log_info(logger,"Tiempo %" PRId64 ,temporal_gettime(tiempoDeUsoCPU));
        //log_info(logger,"Quantum %" PRId64 ,quantum);
        //Si el cronometro marca un tiempo superior al quantum, desalojo el proceso
        if(temporal_gettime(tiempoDeUsoCPU)>=quantum){
            //log_warning(logger,"FIN DE QUANTUM");
            return FIN_DE_QUANTUM;
        } 
    }
    return VOID;
}
// Función para calcular la cantidad de páginas a leer
int calcularPaginasALeer(int first_addr, int last_addr, int page_size){ 
    int start_page = first_addr / page_size;
    int end_page = last_addr / page_size;
    return end_page - start_page + 1;
}
// Función para calcular la cantidad de bytes a escribir/leer en cada página
void calcularBytesPorPagina2(int first_addr, int last_addr, int page_size, int *bytes_por_pagina, int *num_pages) {
    int total_bytes = last_addr - first_addr+1; // Calcula el total de bytes a leer
    // Calcula el número de página inicial y el desplazamiento dentro de la página
    int start_page = first_addr / page_size;
    int start_offset = first_addr % page_size;
    // Calcula el número de página final
    int end_page = last_addr / page_size;
    // Calcula el número total de páginas
    *num_pages = end_page - start_page + 1;
    // Calcula los bytes a leer en la primera página
    if (start_page == end_page) {
        // Caso especial: first_addr y last_addr están en la misma página
        bytes_por_pagina[0] = total_bytes;
    } else {
        bytes_por_pagina[0] = page_size - start_offset;
    // Llena las páginas intermedias con tamaño completo de página
    for (int i = 1; i < *num_pages - 1; i++) {
        bytes_por_pagina[i] = page_size;
    }
    // Calcula los bytes a leer en la última página
    bytes_por_pagina[*num_pages - 1] = total_bytes - (bytes_por_pagina[0] + (*num_pages - 2) * page_size); 
    }
}

int obtenerTamanoPagina(char* path){
    char *pathConfig=malloc(PATH_MAX);
    pathConfig[0]='\0';
    realpath(path, pathConfig);
    strcat(pathConfig,"/memoria.config");
  // printf("Path: %s\n", pathConfig);
    t_config* config = config_create(pathConfig);
    int tamPagina = config_get_int_value(config, "TAM_PAGINA");
    config_destroy(config);
    free(pathConfig);
    return tamPagina;
}
// Instrucciones
void io_fs_delete(char* interfaz,char* nombreArchivo){
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_DELETE, 2, interfaz, nombreArchivo, "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
        } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_DELETE, 2, interfaz, nombreArchivo, "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

void io_stdout_write(char* interfaz, char* registroDireccion, char* RegistroTamanio){
    char* regDireccion=dictionary_get(contextoEjecucion->registrosCPU, registroDireccion);
    char* regTamanio=dictionary_get(contextoEjecucion->registrosCPU, RegistroTamanio);
    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = atoi(regDireccion);
    int last_addr = first_addr + atoi(regTamanio)-1;
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //convierto la variable bytes_por_pagina a un string
    char* bytes_por_pagina_str = malloc(sizeof(char) * 100);
    bytes_por_pagina_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* bytes = malloc(sizeof(char) * 10);
        sprintf(bytes, "%d", bytes_por_pagina[i]);
        strcat(bytes_por_pagina_str, bytes);
        free(bytes);
        if (i < num_pages - 1)
            strcat(bytes_por_pagina_str, ",");
    }
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++) {
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];
    }
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++) 
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    //convierto a char* las direcciones fisicas
    char* direccionesFisicas_str = malloc(sizeof(char) * 100);
    direccionesFisicas_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* direccion = malloc(sizeof(char) * 10);
        sprintf(direccion, "%d", direccionesFisicas[i]);
        strcat(direccionesFisicas_str, direccion);
        free(direccion);
        if (i < num_pages - 1)
            strcat(direccionesFisicas_str, ",");
    }
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_STDOUT_WRITE, 3, interfaz, direccionesFisicas_str, bytes_por_pagina_str, "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
        } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_STDOUT_WRITE, 3, interfaz, direccionesFisicas_str, bytes_por_pagina_str, "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
    free(bytes_por_pagina_str);
    free(direccionesFisicas_str);
}

void io_fs_truncate(char* interfaz, char* nombreArchivo, char* RegistroTamanio){
    char* regTamanio=dictionary_get(contextoEjecucion->registrosCPU, RegistroTamanio);
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_TRUNCATE, 3, interfaz, nombreArchivo, regTamanio, "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
        } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_TRUNCATE, 3, interfaz, nombreArchivo, regTamanio, "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

void io_fs_create(char* interfaz, char* nombreArchivo){
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_CREATE, 2, interfaz, nombreArchivo, "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
        } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_CREATE, 2, interfaz, nombreArchivo, "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

void io_fs_write(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    char* regDireccion=dictionary_get(contextoEjecucion->registrosCPU, registroDireccion);
    char* regTamanio=dictionary_get(contextoEjecucion->registrosCPU, registroTamanio);
    char* regPunteroArchivo=dictionary_get(contextoEjecucion->registrosCPU, registroPunteroArchivo);

    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = atoi(regDireccion);
    int last_addr = first_addr + atoi(regTamanio)-1;
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //convierto la variable bytes_por_pagina a un string
    char* bytes_por_pagina_str = malloc(sizeof(char) * 100);
    bytes_por_pagina_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* bytes = malloc(sizeof(char) * 10);
        sprintf(bytes, "%d", bytes_por_pagina[i]);
        strcat(bytes_por_pagina_str, bytes);
        free(bytes);
        if (i < num_pages - 1)
            strcat(bytes_por_pagina_str, ",");
    }
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++) 
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];

   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++)
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    //convierto a char* las direcciones fisicas
    char* direccionesFisicas_str = malloc(sizeof(char) * 100);
    direccionesFisicas_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* direccion = malloc(sizeof(char) * 10);
        sprintf(direccion, "%d", direccionesFisicas[i]);
        strcat(direccionesFisicas_str, direccion);
        free(direccion);
        if (i < num_pages - 1)
            strcat(direccionesFisicas_str, ",");
    }

    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_WRITE, 5, interfaz, nombreArchivo, direccionesFisicas_str, bytes_por_pagina_str, regPunteroArchivo);
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_WRITE, 5, interfaz, nombreArchivo, direccionesFisicas_str, bytes_por_pagina_str, regPunteroArchivo);
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
    free(bytes_por_pagina_str);
    free(direccionesFisicas_str);
}

void io_fs_read(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    char* regDireccion=dictionary_get(contextoEjecucion->registrosCPU, registroDireccion);
    char* regTamanio=dictionary_get(contextoEjecucion->registrosCPU, registroTamanio);
    char* regPunteroArchivo=dictionary_get(contextoEjecucion->registrosCPU, registroPunteroArchivo);
    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = atoi(regDireccion);
    int last_addr = first_addr + atoi(regTamanio)-1;
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //convierto la variable bytes_por_pagina a un string
    char* bytes_por_pagina_str = malloc(sizeof(char) * 100);
    bytes_por_pagina_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* bytes = malloc(sizeof(char) * 10);
        sprintf(bytes, "%d", bytes_por_pagina[i]);
        strcat(bytes_por_pagina_str, bytes);
        free(bytes);
        if (i < num_pages - 1)
            strcat(bytes_por_pagina_str, ",");
    }
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++)
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++) 
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    //convierto a char* las direcciones fisicas
    char* direccionesFisicas_str = malloc(sizeof(char) * 100);
    direccionesFisicas_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* direccion = malloc(sizeof(char) * 10);
        sprintf(direccion, "%d", direccionesFisicas[i]);
        strcat(direccionesFisicas_str, direccion);
        free(direccion);
        if (i < num_pages - 1)
            strcat(direccionesFisicas_str, ",");
    }
    
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_READ, 5, interfaz, nombreArchivo, direccionesFisicas_str, bytes_por_pagina_str, regPunteroArchivo);
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_FS_READ, 5, interfaz, nombreArchivo, direccionesFisicas_str, bytes_por_pagina_str, regPunteroArchivo);
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
    free(bytes_por_pagina_str);
    free(direccionesFisicas_str);
}

void io_stdin_read(char* interfaz, char* registroDireccion, char* registroTamanio){
    char* regDireccion=dictionary_get(contextoEjecucion->registrosCPU, registroDireccion);
    char* regTamanio=dictionary_get(contextoEjecucion->registrosCPU, registroTamanio);
    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = atoi(regDireccion);
    int last_addr = first_addr + atoi(regTamanio)-1; 
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //convierto la variable bytes_por_pagina a un string
    char* bytes_por_pagina_str = malloc(sizeof(char) * 100);
    bytes_por_pagina_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* bytes = malloc(sizeof(char) * 10);
        sprintf(bytes, "%d", bytes_por_pagina[i]);
        strcat(bytes_por_pagina_str, bytes);
        free(bytes);
        if (i < num_pages - 1)
            strcat(bytes_por_pagina_str, ",");
    }
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++) {
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];
    }
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++) 
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    //convierto a char* las direcciones fisicas
    char* direccionesFisicas_str = malloc(sizeof(char) * 100);
    direccionesFisicas_str[0] = '\0';
    for (int i = 0; i < num_pages; i++) {
        char* direccion = malloc(sizeof(char) * 10);
        sprintf(direccion, "%d", direccionesFisicas[i]);
        strcat(direccionesFisicas_str, direccion);
        free(direccion);
        if (i < num_pages - 1)
            strcat(direccionesFisicas_str, ",");
    }
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_STDIN_READ, 3, interfaz, direccionesFisicas_str, bytes_por_pagina_str, "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_STDIN_READ, 3, interfaz, direccionesFisicas_str, bytes_por_pagina_str, "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
    free(bytes_por_pagina_str);
    free(direccionesFisicas_str);
}
void enviarDireccionTamano2(int direccion,int tamano, int pid, int socket) {
   t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 104;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 3 * sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &tamano, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int), &pid, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); // Manejo de error, puedes ajustarlo según tu aplicación
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(a_enviar);
    free(paquete);
}
void enviarAImprimirAMemoria(const char *mensaje, int direccion, int socket, int pid) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 105;
    paquete->buffer = malloc(sizeof(t_buffer));
    size_t mensaje_len = strlen(mensaje) ; // +1 para el terminador nulo??????
    paquete->buffer->size = 2*sizeof(int) + mensaje_len;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int), mensaje, mensaje_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); // Manejo de error
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(a_enviar);
    free(paquete);
}

/*Copio la cantidad de bytes indicada del string apuntado por SI a DI*/
void copy_string(char* tamanio){
    int tamanioInt = atoi(tamanio); //ESTE ES EL TAMAÑO DE LA PALABRA A LEER Y ESCRIBIR EN MEMORIA
    int pointer = atoi(dictionary_get(contextoEjecucion->registrosCPU, "SI"));
    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = pointer;
    int last_addr = first_addr + tamanioInt-1; 
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++) 
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++) 
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    char *cadenacompleta = malloc(tamanioInt);
    for(int i=0; i<num_pages; i++){
        if (i==0)
            cadenacompleta[0]='\0';
        enviarDireccionTamano2(direccionesFisicas[i],bytes_por_pagina[i],contextoEjecucion->pid,conexionAMemoria);
        //usleep(1000*1200);
        char recibido[100];
		int bytes=recv(conexionAMemoria, recibido, sizeof(recibido), 0);
        recibido[bytes] = '\0';
        //log_info(logger, "Recibido: %s", recibido);
        strncat(cadenacompleta, recibido, bytes_por_pagina[i]);
        log_info(logger, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", contextoEjecucion->pid, direccionesFisicas[i], recibido);
    }
    log_info(logger, "Cadena completa: %s", cadenacompleta);
    //ACA MANDA PARA ESCRIBIR EN MEMORIA
    int pointer2 = atoi(dictionary_get(contextoEjecucion->registrosCPU, "DI"));
    int first_addr2 = pointer2;
    int last_addr2 = first_addr2 + tamanioInt-1; 
    int paginasALeer2 = calcularPaginasALeer(first_addr2, last_addr2, tamPagina);
    int bytes_por_pagina2[paginasALeer2];
    int num_pages2;
    calcularBytesPorPagina2(first_addr2, last_addr2, tamPagina, bytes_por_pagina2, &num_pages2);
    //calcular las direcciones logicas para cada página
    int direccionesLogicas2[num_pages2];
    direccionesLogicas2[0] = first_addr2;
    for (int i = 1; i < num_pages2; i++) 
        direccionesLogicas2[i] = direccionesLogicas2[i - 1] + bytes_por_pagina2[i - 1];
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas2[num_pages2];
    for (int i = 0; i < num_pages2; i++) 
        direccionesFisicas2[i] =  mmu(contextoEjecucion->pid, direccionesLogicas2[i],0);
    char *datosLeidos2 = malloc(100);
    int tamanotexto=0;
    //divido el texto en partes
    for(int i=0; i<num_pages2; i++){
        memcpy(datosLeidos2, cadenacompleta + tamanotexto, bytes_por_pagina2[i]);
        datosLeidos2[bytes_por_pagina2[i]] = '\0';
       // printf("Texto a enviar a memoria: %s\n", datosLeidos2);
        log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", contextoEjecucion->pid, direccionesFisicas2[i], datosLeidos2);
        tamanotexto += strlen(datosLeidos2);
        enviarAImprimirAMemoria(datosLeidos2,direccionesFisicas2[i], conexionAMemoria, contextoEjecucion->pid);//estos datos se deben escribir en la direccion de memoria
        //recibir un mensaje de confirmacion de que se escribio en memoria
        char recibido[100];
        recv(conexionAMemoria, recibido, sizeof(recibido), 0);
        //log_info(logger, "OK DE MEMORIA");
    }
    free(datosLeidos2);
    free(cadenacompleta);
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}
/*Le pido a memoria ajustar el tamanio del proceso*/
void resize(char* tamanio){
    solicitudResize(contextoEjecucion->pid,atoi(tamanio), conexionAMemoria);
    
    int peticion = recibirOperacion(conexionAMemoria);
	switch (peticion) {
		case MENSAJE:
			char* mensaje=recibirMensaje(conexionAMemoria);
            //log_info(logger, "Mensaje recibido: %s", mensaje);
            if(strcmp(mensaje,"OUT_OF_MEMORY")==0){
                contextoEjecucion->fin_de_quantum=false;
                temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
                contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
                temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
                modificarMotivoDesalojo (RESIZE, 0, "","", "", "","");
                enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
                flag_bloqueante = 1;
                return;  
            }
            free(mensaje);
			break;
		default:
            log_warning(logger,"Operacion desconocida.");
			break;
	}
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*Le asigno al registro el valor que se indica*/
void set_c(char* registro, char* valor){ 
    if(strcmp(registro, "PC") == 0){
        contextoEjecucion->programCounter = atoi(valor);
        return;
    }else{
        dictionary_put(contextoEjecucion->registrosCPU, registro, string_duplicate(valor)); //TODO: FIX
    }
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
        //free(tiempoDeUsoCPU);
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*RD=RD+RO*/
void sum_c(char* registro_destino, char* registro_origen){ 
    // Verificar que los registros existen en el diccionario
    char* valorDestino = dictionary_get(contextoEjecucion->registrosCPU, registro_destino);
    char* valorOrigen = dictionary_get(contextoEjecucion->registrosCPU, registro_origen);
    if (!valorDestino || !valorOrigen) {
        log_error(logger, "Registro no encontrado: %s o %s", registro_destino, registro_origen);
        return;
    }
    int primerValor = atoi(valorDestino);
    int segundoValor = atoi(valorOrigen);
    int resultado = primerValor + segundoValor;
    char* resultadoStr = malloc(sizeof(char) * 10);
    sprintf(resultadoStr,"%d", resultado);
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro_destino, free);
    dictionary_put(contextoEjecucion->registrosCPU, registro_destino, resultadoStr);
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*RD=RD-RO*/
void sub_c(char* registro_destino, char* registro_origen){ 
    // Verificar que los registros existen en el diccionario
    char* valorDestino = dictionary_get(contextoEjecucion->registrosCPU, registro_destino);
    char* valorOrigen = dictionary_get(contextoEjecucion->registrosCPU, registro_origen);
    if (!valorDestino || !valorOrigen) {
        log_error(logger, "Registro no encontrado: %s o %s", registro_destino, registro_origen);
        return;
    }
    int primerValor = atoi(valorDestino);
    int segundoValor = atoi(valorOrigen);
    int resultado = primerValor - segundoValor;
    char* resultadoStr = malloc(sizeof(char) * 10);
    sprintf(resultadoStr, "%d", resultado);
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro_destino, free);
    dictionary_put(contextoEjecucion->registrosCPU, registro_destino, resultadoStr);
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*Me fijo que el registro sea distinto de 0. Si lo es, seteo el PC con la instruccion indicada*/
void jnz(char* registro, char* instruccion){ 
    log_info(logger,"En jnz con: %s y %s", registro, instruccion);
    // Obtener el valor del registro desde el diccionario
    char* valorRegistro = dictionary_get(contextoEjecucion->registrosCPU, registro);
    if (!valorRegistro) {
        log_error(logger, "Registro no encontrado: %s", registro);
        return;
    }
    // Convertir el valor del registro a un entero
    int valor = atoi(valorRegistro);
    // Si el valor no es 0, actualizar el contador de programa (programCounter)
    if (valor != 0) {
        log_info(logger, "contextoEjecucion->programCounter: %d", contextoEjecucion->programCounter);
        log_info(logger, "Valor de registro: %d", atoi(instruccion));
        contextoEjecucion->programCounter = atoi(instruccion);
    }
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}
/*Desalojo el proceso y kernel le indica a IO que haga un sleep en una interfaz indicada y un tiempo indicado*/
void io_gen_sleep(char* interfaz, char* unidades_trabajo){
    //INTERRUPT STAGE 
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_GEN_SLEEP, 3, interfaz, unidades_trabajo, "GENERICA", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else {
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (IO_GEN_SLEEP, 3, interfaz, unidades_trabajo, "GENERICA", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*Desalojo el proceso y le pido a kernel que asigne una instancia del recurso indicado*/
void wait_c(char* recurso){
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (WAIT, 1, recurso, "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (WAIT, 1, recurso, "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*Desalojo el proceso y le pido a kernel que libere una instancia del recurso indicado*/
void signal_c(char* recurso){
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (SIGNAL, 1, recurso, "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else{
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (SIGNAL, 1, recurso, "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*Desalojo el proceso y kernel se encarga de mover el proceso a EXIT*/
void exit_c () {
    temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
    contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
    temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
    char * terminado = string_duplicate ("SUCCESS");
    modificarMotivoDesalojo (EXIT, 1, terminado, "", "", "", "");
    //log_info(logger, "Pasa modificarMotivoDesalojo");
    enviarContextoBeta(socketClienteInterrupt, contextoEjecucion); //TODO: HACER CON INTERRUPT
    free (terminado);
    //log_info(logger, "fin exit_c");
    flag_bloqueante = 1;
}

/*Leo el valor almacenado en la direccion fisica de memoria y lo almaceno en el registro*/
void mov_in(char* registro, char* direccionLogica){
    char* regDireccion=dictionary_get(contextoEjecucion->registrosCPU, direccionLogica);
    uint32_t tamRegistro =(uint32_t) obtenerTamanioReg(registro);
    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = atoi(regDireccion);
    int last_addr = first_addr + tamRegistro-1;
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++) 
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++) 
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    char *cadenacompleta = malloc(tamRegistro+1);
    for(int i=0; i<num_pages; i++){
        if (i==0)
            cadenacompleta[0]='\0';
        enviarDireccionTamano2(direccionesFisicas[i],bytes_por_pagina[i],contextoEjecucion->pid,conexionAMemoria);
        char recibido[100];
		int bytes=recv(conexionAMemoria, recibido, sizeof(recibido)-1, 0);
        recibido[bytes] = '\0';
        //log_info(logger, "Recibido: %s", recibido);
        strncat(cadenacompleta, recibido, bytes_por_pagina[i]);
        int numeros=0;
        memcpy(&numeros, recibido,bytes_por_pagina[i]);
        log_info(logger, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>", contextoEjecucion->pid, direccionesFisicas[i], numeros);
        // mem_hexdump(&numeros, bytes_por_pagina[i]);
    }
    //log_info(logger, "Cadena completa: %s", cadenacompleta);
    int numTotal=0;
    memcpy(&numTotal, cadenacompleta, strlen(cadenacompleta));
    //log_info(logger, "Valor total: %d", numTotal);
    //paso numTotal a string
    char* numTotalStr = malloc(sizeof(char) * 10);
    sprintf(numTotalStr, "%d", numTotal);
    free(cadenacompleta);
    dictionary_put(contextoEjecucion->registrosCPU, registro, string_duplicate(numTotalStr));
    free(numTotalStr);
    //INTERUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

/*Escribo en la direccion fisica de memoria el valor almacenado en el registro*/
void mov_out(char* direccionLogica, char* registro){
    char* regDireccion=dictionary_get(contextoEjecucion->registrosCPU, direccionLogica);
    char* valorRegistro=dictionary_get(contextoEjecucion->registrosCPU, registro);
    uint32_t tamRegistro =(uint32_t) obtenerTamanioReg(registro);
    uint32_t valorReg= atoi(valorRegistro);
   // log_warning(logger, "Valor del registro: %u", valorReg);
    int tamPagina = obtenerTamanoPagina("../memoria"); 
    int first_addr = atoi(regDireccion);
    int last_addr = first_addr + tamRegistro-1;
    int paginasALeer = calcularPaginasALeer(first_addr, last_addr, tamPagina);
    int bytes_por_pagina[paginasALeer];
    int num_pages;
    calcularBytesPorPagina2(first_addr, last_addr, tamPagina, bytes_por_pagina, &num_pages);
    //calcular las direcciones logicas para cada página
    int direccionesLogicas[num_pages];
    direccionesLogicas[0] = first_addr;
    for (int i = 1; i < num_pages; i++) 
        direccionesLogicas[i] = direccionesLogicas[i - 1] + bytes_por_pagina[i - 1];
   //hago las traducciones a direcciones fisicas con la mmu
    uint32_t direccionesFisicas[num_pages];
    for (int i = 0; i < num_pages; i++) 
        direccionesFisicas[i] =  mmu(contextoEjecucion->pid, direccionesLogicas[i],0);
    if (tamRegistro == 1){
        uint8_t registro_ext = valorReg;
        void* registro_ext_puntero = &registro_ext;
        char *palabra=malloc(1+1);
        int tamanotexto=0;
        for(int i=0; i<num_pages; i++){
            palabra=realloc(palabra, bytes_por_pagina[i]+1);
            //printf("Tamaño de la palabra: %d\n", bytes_por_pagina[i]);
            memcpy(palabra, registro_ext_puntero + tamanotexto, bytes_por_pagina[i]);
            palabra[bytes_por_pagina[i]] = '\0';
            //printf("Palabra: %s\n", palabra); 
            int numeros=0;
            memcpy(&numeros, palabra,bytes_por_pagina[i]);
            log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", contextoEjecucion->pid, direccionesFisicas[i], numeros);
            tamanotexto += strlen(palabra);
            enviarAImprimirAMemoria(palabra,direccionesFisicas[i], conexionAMemoria, contextoEjecucion->pid);//estos datos se deben escribir en la direccion de memoria
            //recibir un mensaje de confirmacion de que se escribio en memoria
            char recibido[100];
            recv(conexionAMemoria, recibido, sizeof(recibido), 0);
            //log_info(logger, "OK DE MEMORIA");
        }
        free(palabra);
    }
    else{
        uint32_t registro_ext = valorReg;
        void* registro_ext_puntero = &registro_ext;
        char *palabra=malloc(4+1);
        int tamanotexto=0;
        for(int i=0; i<num_pages; i++){
            //printf("Tamaño de la palabra: %d\n", bytes_por_pagina[i]);
            memcpy(palabra, registro_ext_puntero + tamanotexto, bytes_por_pagina[i]);
            palabra[bytes_por_pagina[i]] = '\0';
            //printf("Palabra: %s\n", palabra); 
            int numeros=0;
            memcpy(&numeros, palabra, bytes_por_pagina[i]);
            log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", contextoEjecucion->pid, direccionesFisicas[i], numeros);
            tamanotexto += strlen(palabra);
            enviarAImprimirAMemoria(palabra,direccionesFisicas[i], conexionAMemoria, contextoEjecucion->pid);//estos datos se deben escribir en la direccion de memoria
            //recibir un mensaje de confirmacion de que se escribio en memoria
            char recibido[100];
            recv(conexionAMemoria, recibido, sizeof(recibido), 0);
            //log_info(logger, "OK DE MEMORIA");
        }
        free(palabra);
    }
    //INTERRUPT STAGE
    if(check_interrupt()==FIN_DE_QUANTUM){
        contextoEjecucion->fin_de_quantum=true;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    } else if(check_interrupt()==USER_INTERRUPTION){
        contextoEjecucion->fin_de_quantum=false;
        temporal_stop(tiempoDeUsoCPU); //Detengo el cronometro
        contextoEjecucion->tiempoDeUsoCPU=temporal_gettime(tiempoDeUsoCPU); //Asigno el tiempo al contexto
        temporal_destroy(tiempoDeUsoCPU); //Destruyo el cronometro
        modificarMotivoDesalojo (USER_INTERRUPTION, 0, "", "", "", "", "");
        enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        flag_bloqueante = 1;
    }
}

// Funciones grales
void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3, char * parm4, char * parm5) {
    char * (parametros[5]) = { parm1, parm2, parm3, parm4, parm5};
    contextoEjecucion->motivoDesalojo->motivo = comando;
    contextoEjecucion->motivoDesalojo->parametrosLength = numParametros;
    for (int i = 0; i < numParametros; i++){
        contextoEjecucion->motivoDesalojo->parametros[i] = parametros[i];
    }
}

void liberarMemoria() {
    mensajeInterrupcion="";
    for (int i = 0; i <= cantParametros; i++) free(elementosInstruccion[i]);
    free(elementosInstruccion);
    free(instruccionAEjecutar);
}

char* recibirValor(int socket) {
    char* valor;
    int tamanio; 
	int size, desplazamiento = 0;
	void * buffer;
	buffer = recibirBuffer(socket, &size);
    memcpy(&(tamanio), buffer, sizeof(int32_t));
    desplazamiento += sizeof(int32_t);
    memcpy(&(valor),buffer+desplazamiento,sizeof(char)*tamanio); 
    desplazamiento+=sizeof(char)*tamanio; 
	free(buffer);
    return valor;
}

int obtenerTamanioReg(char* registro){
    if(string_starts_with(registro, "E") || strcmp(registro,"SI")==0 || strcmp(registro,"DI")==0) return 4;
    else return 1;
}

void execute() {
    //log_info(logger, "inicio execute");
    switch(cantParametros) {
        case 0:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> ", contextoEjecucion->pid, listaComandos[instruccionActual]);
            break;
        case 1:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> -  <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1]);
            break;
        case 2:   
            log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case 3:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3]);
            break; 
        case 4:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4]);
            break;
        case 5:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4], elementosInstruccion[5]);
            break;
    }
    //log_info(logger, "instruccionActual: %d", instruccionActual);
    switch(instruccionActual){//TODO: Completar con instrucciones restantes
        case SET: 
            set_c(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case SUM:
            sum_c(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case SUB:
            sub_c(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case JNZ:
            jnz(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case IO_GEN_SLEEP:
            io_gen_sleep(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case MOV_IN:
            mov_in(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case MOV_OUT:
            mov_out(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case WAIT:
            wait_c(elementosInstruccion[1]);
            break;
        case SIGNAL:
            signal_c(elementosInstruccion[1]);
            break;
        case EXIT:
            exit_c();
            break;
        case IO_STDIN_READ:
            io_stdin_read(elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3]);
            break;
        case IO_STDOUT_WRITE:
            io_stdout_write(elementosInstruccion[1],elementosInstruccion[2], elementosInstruccion[3]);
            break;
        case IO_FS_CREATE:
            io_fs_create(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case IO_FS_DELETE:
            io_fs_delete(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case IO_FS_TRUNCATE:
            io_fs_truncate(elementosInstruccion[1], elementosInstruccion[2],elementosInstruccion[3]);
            break;
        case IO_FS_WRITE:
            io_fs_write(elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4], elementosInstruccion[5]);
            break;
        case IO_FS_READ:
            io_fs_read(elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4], elementosInstruccion[5]);
            break;
        case RESIZE:
            resize(elementosInstruccion[1]);
            break;
        case COPY_STRING:
            copy_string(elementosInstruccion[1]);
            break;
        default:
            break;
    }
}