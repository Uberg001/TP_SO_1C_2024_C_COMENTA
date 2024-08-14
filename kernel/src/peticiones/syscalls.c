#include <peticiones/syscalls.h>

t_list *recursos;
char **nombresRecursos;
char* invalidResource = "INVALID_RESOURCE";
char* invalidInterface = "INVALID_INTERFACE";
char* outOfMemory = "OUT_OF_MEMORY";
char* interrupted_by_user="INTERRUPTED_BY_USER";
estadoProceso estadoAnterior; 
void eliminarProcesoAsociado(t_pcb *proceso){
    for (size_t i = 0; i < list_size(pcbsBloqueados); i++){
        t_pcb *procesoABuscar = list_get(pcbsBloqueados,i);
        if (proceso->pid == procesoABuscar->pid){
            pthread_mutex_lock(&mutexListaBloqueados);
            list_remove(pcbsBloqueados,i);              
            pthread_mutex_unlock(&mutexListaBloqueados);
        }
    }
}
//Seria para las funciones de IO
void pasarAReady(t_pcb *proceso){
    //log_warning(logger, "Proceso <%d> - fin_de_quantum: %d", proceso->pid, proceso->fin_de_quantum);
    eliminarProcesoAsociado(proceso);
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    if(proceso->algoritmo==FIFO){ //Si es fifo, voy a ready de cualquier forma
        ingresarAReady(proceso);
    }
    //Si estoy en RR, no importa si hay o no fin de q; paso el proceso a ready siempre
    if(proceso->algoritmo==RR){
        ingresarAReady(proceso);
    }
    //Si estoy en VRR y hubo fin de q, el proceso vuelve a ready con el q reseteado
    if(proceso->algoritmo==VRR && proceso->fin_de_quantum==true){ 
        ingresarAReady(proceso);
    }
    //Si estoy en VRR, pero no hubo fin de q, el proceso va a la cola prioritaria con el q modificado
    if(proceso->algoritmo==VRR && proceso->fin_de_quantum==false){
        ingresarAReadyAux(proceso);
    } 
}

void recibirMsjIO(int socketClienteIO){
    char buffer[1024];
    int bytes_recibidos = recv(socketClienteIO, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0)
        perror("Error al recibir el mensaje");
    buffer[bytes_recibidos] = '\0'; // Asegurar el carácter nulo al final del mensaje
    //log_info(logger, "valor recibido: %s", buffer);
}

//FUNCIONES GENERALES
void retornoContexto(t_pcb *proceso, t_contexto *contextoEjecucion){
    //logger=cambiarNombre(logger,"Kernel-Retorno Contexto");
    //Aca trato las instrucciones bloqueantes
    switch (contextoEjecucion->motivoDesalojo->motivo){
        case WAIT:
            wait_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case SIGNAL:
            signal_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_GEN_SLEEP:
            io_gen_sleep(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_STDIN_READ:
            io_stdin_read(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_STDOUT_WRITE:
            io_stdout_write(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_CREATE:
            io_fs_create(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_DELETE:
            io_fs_delete(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_TRUNCATE:
            io_fs_truncate(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_WRITE:
            io_fs_write(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_READ:
            io_fs_read(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case EXIT:
            exit_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case RESIZE:
            exit_s(proceso, &outOfMemory);
            break;
        case FIN_DE_QUANTUM:
            finDeQuantum(proceso);
            break;
        case USER_INTERRUPTION:
            exit_s(proceso,&interrupted_by_user);
            break;
        default:
            log_error(loggerError, "Comando incorrecto");
            break;
    }
}

void loggearBloqueoDeProcesos(t_pcb* proceso, char* motivo) {
    log_info(logger,"PID: <%d> - Bloqueado por: <%s>", proceso->pid, motivo); //Log obligatorio
}

void loggearSalidaDeProceso(t_pcb* proceso, char* motivo) {
    log_info(logger,"Finaliza el proceso <%d> - Motivo: <%s>", proceso->pid, motivo); //log obligatorio
}

//FUNCIONES RETORNO CONTEXTO
//WAIT [Recurso]
void wait_s(t_pcb *proceso,char **parametros){
    char* recurso=parametros[0];
    int indexRecurso = indiceRecurso(recurso);
    
    if(indexRecurso==-1){ //Verifico que exista el recurso
        exit_s(proceso,&invalidResource); //Si no existe, va a EXIT
        return;
    }

    //Resto 1 a la instancias del recurso indicado
    int instanciaRecurso=instanciasRecursos[indexRecurso];
    instanciaRecurso--; 
    instanciasRecursos[indexRecurso]=instanciaRecurso;

    //log_info(logger,"PID:<%d>-WAIT:<%s>-Instancias:<%d>",proceso->pid,recurso,instanciaRecurso);

    //Si el numero de instancias es menor a 0 el proceso se bloquea
    if(instanciaRecurso<0){
        t_list *colaBloqueadosRecurso=list_get(recursos,indexRecurso);

        estadoAnterior = proceso->estado;
        proceso->estado = BLOCKED;
    
        pthread_mutex_lock(&mutexListaBloqueados);
        list_add(pcbsBloqueados,proceso);
        pthread_mutex_unlock(&mutexListaBloqueados);

        list_add(colaBloqueadosRecurso,proceso);

        loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
        loggearBloqueoDeProcesos(proceso, recurso);
    } else {//Si no, vuelve a cpu
        list_add(proceso->recursosAsignados, (void*)string_duplicate(recurso));
        estadoAnterior = proceso->estado;
        proceso->estado = READY;
        loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
        ingresarAReady(proceso);
    }
}

//SIGNAL [Recurso]
void signal_s(t_pcb *proceso,char **parametros){
    char *recurso = parametros[0];
    int indexRecurso = indiceRecurso(recurso);

    //Verifico que exista el recurso. Si no existe, va a EXIT
    if (indexRecurso == -1){
        if(!strncmp(parametros[2],"EXIT",4)){
        return;
        } else{
            exit_s(proceso, &invalidResource); 
            return;
        } 
    }

    //Sumo 1 instancia del recurso que se especifica
    int instancRecurso = instanciasRecursos[indexRecurso];
    instancRecurso++;

    //log_info(logger,"PID: <%d> - Signal: <%s> - Instancias: <%d>",proceso->pid, recurso, instancRecurso); 
    eliminarRecursoLista(proceso->recursosAsignados,recurso); 

    instanciasRecursos[indexRecurso]=instancRecurso;
    //log_warning(logger,"Instancias del recurso %s: %d",recurso,instancRecurso);
    if(instancRecurso <= 0){
        //instancRecurso++;
        instanciasRecursos[indexRecurso]=instancRecurso;
        
        pthread_mutex_lock(&mutexListaBloqueados);
        t_list *colaBloqueadosRecurso = list_get(recursos, indexRecurso);
        t_pcb* pcbDesbloqueado = desencolar(colaBloqueadosRecurso);

        list_add(pcbDesbloqueado->recursosAsignados, (void*)string_duplicate (recurso));
        pthread_mutex_unlock(&mutexListaBloqueados);
        estadoAnterior = pcbDesbloqueado->estado;
        pcbDesbloqueado->estado = READY;
        loggearCambioDeEstado(pcbDesbloqueado->pid,estadoAnterior,pcbDesbloqueado->estado);
        eliminarProcesoAsociado(pcbDesbloqueado);
        ingresarAReady(pcbDesbloqueado);
    }
    
    //Si invoco signal para liberar los recursos, termino la funcion. Si no, paso el proceso a ready
    if(!strncmp(parametros[2],"EXIT",4)){ 
        return;
    } else{
        //list_add(proceso->recursosAsignados, (void*)string_duplicate(recurso));
        estadoAnterior = proceso->estado;
        proceso->estado = READY;
        loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
        ingresarAReady(proceso);
    }
}


void io_gen_sleep(t_pcb *proceso, char **parametros){
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);

    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "GENERICA");
        if (esValida == 1 ){
            //log_warning(logger, "Proceso <%d> encolado en cola de bloqueados", proceso->pid);
            // en caso de validar() sea 1, hacemos io_gen_sleep
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            
            //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            loggearBloqueoDeProcesos(proceso, "IO_GEN_SLEEP");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_GEN_SLEEP por <%s> unidades de trabajo", proceso->pid, parametros[1]);

            io_global->parametro1 = parametros[0];
            io_global->parametro2 = parametros[1];

            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)

void io_stdin_read(t_pcb *proceso,char **parametros){
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDIN");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_STDIN_READ");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_STDIN_READ",proceso->pid);

           //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = contextoEjecucion->motivoDesalojo->parametros[1];
            io_global->parametro2 = contextoEjecucion->motivoDesalojo->parametros[2];
            
            //log_info(logger, "parametro 1 %s", io_global->parametro1);
            //   log_info(logger, "parametro 2222 %s", io_global->parametro2);
            // log_info(logger, "Puntero io_global: %p", (void*)io_global);
             ///////////pthread_mutex_lock(&io_global->mutex_cola_procesos);
            queue_push( io_global->cola_procesos_io,proceso);
             //pthread_mutex_unlock(&io_global->mutex_cola_procesos);
            sem_post(&io_global->semaforo_cola_procesos);
           // sem_wait(&io_global->liberacion_semaforo);
            //log_info(logger, "luego de liberar el seeeeemaforo");
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)

void io_stdout_write(t_pcb *proceso,char **parametros){
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDOUT");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_STDOUT_WRITE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_STDOUT_WRITE",proceso->pid);

            //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = parametros[1];
            io_global->parametro2 = parametros[2];

            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//IO_FS_CREATE (Interfaz, Nombre Archivo)
void io_fs_create(t_pcb *proceso,char **parametros){
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_FS_CREATE",proceso->pid);
            //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = parametros[0];
            io_global->parametro2 = parametros[1];
            io_global->funcion= IO_FS_CREATE;
            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}
//IO_FS_DELETE (Interfaz, Nombre Archivo)
void io_fs_delete(t_pcb *proceso,char **parametros){
Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_DELETE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_FS_DELETE",proceso->pid);

            //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = parametros[0];
            io_global->parametro2 = parametros[1];
            io_global->funcion= IO_FS_DELETE;
            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño)
void io_fs_truncate(t_pcb *proceso,char **parametros){
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_TRUNCATE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_FS_TRUNCATE",proceso->pid);

            //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = parametros[0];
            io_global->parametro2 = parametros[1];
            io_global->parametro3 = parametros[2];
            io_global->funcion= IO_FS_TRUNCATE;
            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
void io_fs_write(t_pcb *proceso,char **parametros){
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_WRITE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_FS_WRITE",proceso->pid);

            //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = parametros[1];
            io_global->parametro2 = parametros[2];
            io_global->parametro3 = parametros[3];
            io_global->parametro4 = parametros[4];
            io_global->funcion= IO_FS_WRITE;
            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
void io_fs_read(t_pcb *proceso,char **parametros){
    Interfaz *io_global = obtener_interfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0]);
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1 && io_global){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_READ");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            //log_info(logger, "PID <%d>-Ejecuta IO_FS_READ",proceso->pid);

             //agrego proceso a lista de bloqueados
            pthread_mutex_lock(&mutexListaBloqueados);
            list_add(pcbsBloqueados,proceso);
            pthread_mutex_unlock(&mutexListaBloqueados);

            io_global->parametro1 = parametros[1];
            io_global->parametro2 = parametros[2];
            io_global->parametro3 = parametros[3];
            io_global->parametro4 = parametros[4];
            io_global->funcion= IO_FS_READ;
            queue_push( io_global->cola_procesos_io,proceso);
            sem_post(&io_global->semaforo_cola_procesos);
        }
        else{
            // mandar proceso a exit porque devuelve -1
            exit_s(proceso,&invalidInterface);
        }
    }
    else{
        // mandar proceso a exit
        exit_s(proceso,&invalidInterface);
    }
}

//EXIT
void exit_s(t_pcb *proceso,char **parametros){
    estadoAnterior = proceso->estado;
    proceso->estado = SALIDA;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearSalidaDeProceso(proceso, parametros[0]);
    if(!list_is_empty(proceso->recursosAsignados))
        liberarRecursosAsignados(proceso);
    flag_exit=1;
    liberarMemoriaPCB(proceso);
    destruirPCB(proceso);
    if(contextoEjecucion!=NULL)
        destroyContexto();
    sem_post(&semGradoMultiprogramacion);
}

//FIN_DE_QUANTUM
void finDeQuantum(t_pcb *proceso){
    log_info(logger,"PID: <%d> - Desalojado por fin de Quantum",proceso->pid); //Log obligatorio
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    //No importa si es RR o VRR, siempre se encola en READY
    ingresarAReady(proceso); 
}

void enviarMensajeGen(int socket_cliente, char *mensaje, char *entero_str, int pid){
    int entero = atoi(entero_str);
    t_paquete* paquete=crearPaquete();
    int length = strlen(mensaje);
    paquete->codigo_operacion=IO_GEN_SLEEP;
    paquete->buffer->size = 2*sizeof(int) + length;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &entero, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int), mensaje, length);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket_cliente, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
}