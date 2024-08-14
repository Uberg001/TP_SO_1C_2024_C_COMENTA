#include <escuchaIO/servidorIO.h>
#include <global.h>
#define BUFFER_SIZE 1024

void ejecutarServidorKernel(Interfaz *interfaz_actual);
void hacerHandshake(int socketClienteIO);
void recibirNombreInterfaz(int socketClienteIO, Kernel_io *kernel);
void guardarNombreTipoYSocketEnStruct(Kernel_io *kernel, char *nombreInterfaz, char *tipoInterfaz, int socketClienteIO);
void dormir_IO(Interfaz *interfaz, t_pcb *proceso);

void escucharAlIO() {
    char *puertoEscucha = confGet("PUERTO_ESCUCHA");
    int socketKernel = alistarServidorMulti(puertoEscucha);
    log_info(logger,"Esperando conexiones con IO...");
    while (1) {
        //pthread_t thread;
        
        int *socketClienteIO = malloc(sizeof(int));
        *socketClienteIO = esperarCliente(socketKernel);
        log_info(logger, "IO conectado, en socket: %d",*socketClienteIO);

        hacerHandshake(*socketClienteIO);
        recibirNombreInterfaz(*socketClienteIO, &kernel);

        /*pthread_create(&thread,
                        NULL,
                        (void*) ejecutarServidorKernel,
                        socketClienteIO);
        pthread_detach(thread);*/
    }
}

void recibirNombreInterfaz(int socketClienteIO, Kernel_io *kernel){
    char nombreInterfaz[BUFFER_SIZE] = {0};    
    char tipoInterfaz[BUFFER_SIZE] = {0};  
   int valread = recv(socketClienteIO, nombreInterfaz, BUFFER_SIZE, 0);
    if (valread < 0) {
        log_error(loggerError,"se recibio mal el nombre");
    } else {
        int valreadTipo = recv(socketClienteIO, tipoInterfaz, BUFFER_SIZE, 0);
        if (valreadTipo < 0) {
            log_error(loggerError,"se recibio mal el tipo");
        }else{
           // log_info(logger, "Nombre recibido: %s\n", nombreInterfaz);
            //log_info(logger, "tipo recibido: %s\n", tipoInterfaz);
            guardarNombreTipoYSocketEnStruct(kernel, nombreInterfaz, tipoInterfaz, socketClienteIO);
        }
    }
}
void guardarNombreTipoYSocketEnStruct(Kernel_io *kernel, char *nombreInterfaz, char *tipoInterfaz, int socketClienteIO) {
    Interfaz *nuevaInterfaz = malloc(sizeof(Interfaz));
    if (kernel->interfaces == NULL) {
        log_error(loggerError, "Error al redimensionar el arreglo de interfaces");
        exit(EXIT_FAILURE);
    }
    
    // Copiar cadenas de caracteres para evitar problemas de sobrescritura
    nuevaInterfaz->nombre_interfaz = strdup(nombreInterfaz);
    nuevaInterfaz->tipo_interfaz = strdup(tipoInterfaz);
    if (nuevaInterfaz->nombre_interfaz == NULL || nuevaInterfaz->tipo_interfaz == NULL) {
        log_error(loggerError, "Error al copiar las cadenas de caracteres");
        free(nuevaInterfaz->nombre_interfaz);
        free(nuevaInterfaz->tipo_interfaz);
        free(nuevaInterfaz);
        exit(EXIT_FAILURE);
    }

    nuevaInterfaz->socket_interfaz = socketClienteIO;
    nuevaInterfaz->cola_procesos_io = queue_create();
    if (nuevaInterfaz->cola_procesos_io == NULL) {
        log_error(loggerError, "Error al crear la cola de procesos IO");
        free(nuevaInterfaz->nombre_interfaz);
        free(nuevaInterfaz->tipo_interfaz);
        free(nuevaInterfaz);
        exit(EXIT_FAILURE);
    }

    sem_init(&nuevaInterfaz->semaforo_cola_procesos, 0, 0);
    list_add(kernel->interfaces,nuevaInterfaz);

    pthread_t thread;
    pthread_create(&thread, NULL, (void*)ejecutarServidorKernel, nuevaInterfaz);
    pthread_detach(thread);
    //log_info(logger, "Hilo para ejecutar el servidor kernel creado y desatado");

    // Incrementar la cantidad de interfaces
}


void hacerHandshake(int socketClienteIO){
    //size_t bytes;
    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;
    recv(socketClienteIO, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == 1) {
        send(socketClienteIO, &resultOk, sizeof(int32_t), 0);
    } else {
        send(socketClienteIO, &resultError, sizeof(int32_t), 0);
    }
}

void inicializarStructsIO(Kernel_io *kernel) {
   kernel->interfaces= list_create();

}

void destruirStructsIO(Kernel_io *kernel) {
    free(kernel);
}

Interfaz *obtener_interfaz(const Kernel_io *kernel, const char *nombre_interfaz) {
    // log_info(logger, "nombre requerido %s",nombre_interfaz);
     // log_info(logger, "largo lista %d",list_size(kernel->interfaces));
    for (size_t i = 0; i < list_size(kernel->interfaces); i++) {
        Interfaz *interfaz= list_get(kernel->interfaces,i);
       //  log_info(logger, "nombre    que  tengo %ss", interfaz->nombre_interfaz);
         //log_info(logger, "Puntero deeeee la interfaz de kernel: %p", (void*)&interfaz);
        if (strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) {
         //   log_info(logger, "llamaaa %s", interfaz->nombre_interfaz);
            return interfaz;
        }
    }
     return NULL;// Si no se encuentra la interfaz
}

//esta funcion anda flama
int obtener_socket(const Kernel_io *kernel, const char *nombre_interfaz) {
    for (size_t i = 0; i < list_size(kernel->interfaces); i++) {
         Interfaz *interfaz= list_get(kernel->interfaces,i);
        if (strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0)
            return interfaz->socket_interfaz;
    }
    return -1; // Si no se encuentra la interfaz
}
int validarTipoInterfaz(const Kernel_io *kernel, char *nombreInterfaz, char *tipoRequerido){
    for (size_t i = 0; i < list_size(kernel->interfaces); i++) {
         Interfaz *interfaz= list_get(kernel->interfaces,i);
        /*log_info(logger, "nombre interfaz: %s", interfaz->nombre_interfaz);
        log_info(logger, "tipo interfaz: %s", interfaz->tipo_interfaz);
        log_info(logger, "tipo requerido: %s", tipoRequerido);
        log_info(logger, "nombre requerido: %s", tipoRequerido);*/
        if (strcmp(interfaz->nombre_interfaz, nombreInterfaz) == 0) {
            if (strcmp(interfaz->tipo_interfaz, tipoRequerido) == 0)
                return 1;
        }
    }
    return -1;
}

void desconectar_interfaz(Kernel_io *kernel, const char *nombre_interfaz) {
    for (size_t i = 0; i < list_size(kernel->interfaces); i++) {
        Interfaz *interfaz= list_get(kernel->interfaces,i);
        if (strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) {
            list_remove(kernel->interfaces,i);
            log_info(logger,"Interfaz %s desconectada\n", nombre_interfaz);
            return;
        }
    }
    log_info(logger,"Interfaz %s no encontrada\n", nombre_interfaz);
}

int existeLaInterfaz(char *nombreInterfaz, Kernel_io *kernel){
    //verifico que exista en la estructura
    int socket = obtener_socket(kernel, nombreInterfaz);
    if(socket == -1){
        return -1;
    }else{
        int estaConectado = verificarConexionInterfaz(kernel, nombreInterfaz);
        return estaConectado;
    }
}

int verificarConexionInterfaz(Kernel_io *kernel, const char *nombre_interfaz) {
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    int max_sd = 0;
    int interfaz_socket = -1;
    for (size_t i = 0; i < list_size(kernel->interfaces); i++) {
         Interfaz *interfaz= list_get(kernel->interfaces,i);
        if (strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) {
            interfaz_socket = interfaz->socket_interfaz;
            break;
        }
    }
    if (interfaz_socket == -1) {
        log_info(logger, "Interfaz %s no encontrada", nombre_interfaz);
        return -1;
    }
    FD_SET(interfaz_socket, &readfds);
    if (interfaz_socket > max_sd) {
        max_sd = interfaz_socket;
    }
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000; // 500ms
    int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
    if (activity < 0) {
        perror("select error");
        return -1;
    }
    if (FD_ISSET(interfaz_socket, &readfds)) {
        char buffer[1];
        int valread = recv(interfaz_socket, buffer, sizeof(buffer), MSG_PEEK);
        if (valread == 0) {
            log_info(logger, "Interfaz %s desconectada", nombre_interfaz);
            close(interfaz_socket);
            desconectar_interfaz(kernel, nombre_interfaz);
            return -1;
        }
    }
    return 1;
}

void ejecutarServidorKernel(Interfaz *interfaz_actual){
  pthread_mutex_unlock(&mutex_lista_global);
  list_add(lista_global_io, interfaz_actual);
  pthread_mutex_lock(&mutex_lista_global);
     //log_info(logger, "Puntero io_global_actual: %p", (void*)io_global_actual);
    //log_info(logger, "Puntero interfaz?actual: %p", (void*)interfaz_actual);

      //log_info(logger,"tipo de la innterfaz conectada:  %s", interfaz_actual->tipo_interfaz);
        //log_info(logger,"socket de la innterfaz conectada:  %d", interfaz_actual->socket_interfaz); 
    while(true) {
        //log_info(logger,"antes de estar en la cola");
        sem_wait(&interfaz_actual->semaforo_cola_procesos);
        //log_info(logger,"hay proceso en la cola");
        t_pcb *pcb;
        pcb = queue_pop(interfaz_actual->cola_procesos_io);

        //log_info(logger, "Puntero proceso servidor: %p", (void*)pcb);
        ejecutar_io(interfaz_actual, pcb);
        /*if (respuesta_ok) {
            ...
        } else {
            ...
            break
        }*/
    } 
    //pthread_mutex_unlock(&mutex_lista_global);

   // list_remove(lista_global_io, interfaz_actual);
    //pthread_mutex_lock(&mutex_lista_global);
   /* wait(mutex_lista_global)
    remove(lista_global, io)
    signal(mutex_lista_global)*/
}

void ejecutar_io(Interfaz *interfaz, t_pcb *proceso){
    // log_info(logger,"noombre %s", interfaz->parametro1);
    //  log_info(logger,"tiempooo %s", interfaz->parametro2);
    if (strcmp(interfaz->tipo_interfaz, "GENERICA") == 0)
    {
        dormir_IO(interfaz, proceso);
    }
    else
        if (strcmp(interfaz->tipo_interfaz, "STDIN") == 0)
        {
            ejecutar_io_stdin_read(interfaz, proceso);
        }
    else
         if (strcmp(interfaz->tipo_interfaz, "STDOUT") == 0)
        {
            ejecutar_io_stdout_write(interfaz, proceso);
        }
     else
        if (strcmp(interfaz->tipo_interfaz, "DialFS") == 0)
        {
            switch (interfaz->funcion)
            {
            case IO_FS_CREATE:
                ejecutar_io_fs_create(interfaz,proceso);
            break;
            case IO_FS_DELETE:
                 ejecutar_io_fs_delete(interfaz,proceso);
            break;
            case IO_FS_TRUNCATE:
                 ejecutar_io_fs_truncate(interfaz,proceso);
            break;
            case IO_FS_WRITE:
                 ejecutar_io_fs_write(interfaz,proceso);
            break;
            case IO_FS_READ:
                 ejecutar_io_fs_read(interfaz,proceso);
            break;
            default:
            break;
            }
        }
}

void ejecutar_io_fs_create(Interfaz *interfaz, t_pcb *proceso){
    char* nombre_interfaz=interfaz->parametro1;
    char* nombreArchivo=interfaz->parametro2;
    int socketClienteIO = interfaz->socket_interfaz;
    int pid = proceso->pid;
    t_paquete* paquete=malloc(sizeof(t_paquete));
    paquete->codigo_operacion=IO_FS_CREATE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(nombre_interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), nombre_interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    char *buffer;
    recv(socketClienteIO, &buffer, sizeof(buffer), 0);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    pasarAReady(proceso);
}

void ejecutar_io_fs_delete(Interfaz *interfaz, t_pcb *proceso){
    char* nombre_interfaz=interfaz->parametro1;
    char* nombreArchivo=interfaz->parametro2;
    int socketClienteIO = interfaz->socket_interfaz;
    int pid = proceso->pid;
    t_paquete* paquete=malloc(sizeof(t_paquete));
    paquete->codigo_operacion=IO_FS_DELETE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(nombre_interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), nombre_interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &pid, sizeof(int)); 
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    char *buffer;
    recv(socketClienteIO, &buffer, sizeof(buffer), 0);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    pasarAReady(proceso);
}

void ejecutar_io_fs_truncate(Interfaz *interfaz, t_pcb *proceso){
    char* nombre_interfaz=interfaz->parametro1;
    char* nombreArchivo=interfaz->parametro2;
    int tamanio=atoi(interfaz->parametro3);
    int socketClienteIO = interfaz->socket_interfaz;
    int pid = proceso->pid;
    t_paquete* paquete=malloc(sizeof(t_paquete));
    paquete->codigo_operacion=IO_FS_TRUNCATE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(nombre_interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 4*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), nombre_interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    char *mensaje;
    recv(socketClienteIO, &mensaje, sizeof(mensaje), 0);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    pasarAReady(proceso);
}

void ejecutar_io_fs_write(Interfaz *interfaz, t_pcb *proceso){
    char* nombre_interfaz=interfaz->nombre_interfaz;//verr si llega biiien este
    char* nombreArchivo=interfaz->parametro1;
    char* direccion=interfaz->parametro2;
    char* tamanio=interfaz->parametro3;
    char* punteroArchivo=interfaz->parametro4;
    int punterito = atoi(punteroArchivo);
    int socketClienteIO =interfaz->socket_interfaz;
    int pid = proceso->pid;
    char** direcciones = string_split(direccion, ",");
    int cantidadDirecciones = 0;
    while(direcciones[cantidadDirecciones] != NULL)
        cantidadDirecciones++;
    int* direccionesInt = malloc(cantidadDirecciones*sizeof(int));
    for(int i = 0; i < cantidadDirecciones; i++)
        direccionesInt[i] = atoi(direcciones[i]);
    t_paquete* paquete=malloc(sizeof(t_paquete));
    // tamanio contiene todos los tamannios separados por una coma, necesito separarlos
    char** tamanios = string_split(tamanio, ",");
    int cantidadTamanios = 0;
    while(tamanios[cantidadTamanios] != NULL)
        cantidadTamanios++;
    int* tamaniosInt = malloc(cantidadTamanios*sizeof(int));
    for(int i = 0; i < cantidadTamanios; i++)
        tamaniosInt[i] = atoi(tamanios[i]);
    paquete->codigo_operacion=IO_FS_WRITE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(nombre_interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = 6*sizeof(int) + interfaz_len + archivo_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), nombre_interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &cantidadDirecciones, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, direccionesInt, cantidadDirecciones* sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int), &cantidadTamanios, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int), tamaniosInt, cantidadTamanios* sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), &punterito, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 5*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 6*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    char *mensaje;
    recv(socketClienteIO, &mensaje, sizeof(mensaje), 0);
    //free(direcciones);
    //free(tamanios);
    string_array_destroy(direcciones);
    string_array_destroy(tamanios);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    free(direccionesInt);
    free(tamaniosInt);
    pasarAReady(proceso);
}


void ejecutar_io_fs_read(Interfaz *interfaz, t_pcb *proceso){
    char* nombre_interfaz=interfaz->nombre_interfaz; //verr si llega biiien este
    char* nombreArchivo=interfaz->parametro1;
    char* direccion=interfaz->parametro2;
    char* tamanio=interfaz->parametro3;
    char* punteroArchivo=interfaz->parametro4;
    int punterito = atoi(punteroArchivo);
    int socketClienteIO = obtener_socket(&kernel, interfaz->nombre_interfaz);
    int pid = proceso->pid;
    char** direcciones = string_split(direccion, ",");
    int cantidadDirecciones = 0;
    while(direcciones[cantidadDirecciones] != NULL)
        cantidadDirecciones++;
    int* direccionesInt = malloc(cantidadDirecciones*sizeof(int));
    for(int i = 0; i < cantidadDirecciones; i++)
        direccionesInt[i] = atoi(direcciones[i]);
    t_paquete* paquete=malloc(sizeof(t_paquete));
    // tamanio contiene todos los tamannios separados por una coma, necesito separarlos
    char** tamanios = string_split(tamanio, ",");
    int cantidadTamanios = 0;
    while(tamanios[cantidadTamanios] != NULL)
        cantidadTamanios++;
    int* tamaniosInt = malloc(cantidadTamanios*sizeof(int));
    for(int i = 0; i < cantidadTamanios; i++)
        tamaniosInt[i] = atoi(tamanios[i]);
    paquete->codigo_operacion=IO_FS_READ;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(nombre_interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = 6*sizeof(int) + interfaz_len + archivo_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), nombre_interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &cantidadDirecciones, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, direccionesInt, cantidadDirecciones* sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int), &cantidadTamanios, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int), tamaniosInt, cantidadTamanios* sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), &punterito, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 5*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 6*sizeof(int)+ interfaz_len + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int), nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    char *mensaje;
    recv(socketClienteIO, &mensaje, sizeof(mensaje), 0);
    //free(direcciones);
    //free(tamanios);
    string_array_destroy(direcciones);
    string_array_destroy(tamanios);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    free(direccionesInt);
    free(tamaniosInt);
    pasarAReady(proceso); 
}

void ejecutar_io_stdout_write(Interfaz *interfaz, t_pcb *proceso){
    char* direccionFisica=interfaz->parametro1;
    char* tamanio=interfaz->parametro2;
    int socketClienteIO = interfaz->socket_interfaz;
    int pid = proceso->pid;
    char** direcciones = string_split(direccionFisica, ",");
    int cantidadDirecciones = 0;
    while(direcciones[cantidadDirecciones] != NULL)
        cantidadDirecciones++;
    int* direccionesInt = malloc(cantidadDirecciones*sizeof(int));
    for(int i = 0; i < cantidadDirecciones; i++)
        direccionesInt[i] = atoi(direcciones[i]);
    t_paquete* paquete=crearPaquete();
    // tamanio contiene todos los tamannios separados por una coma, necesito separarlos
    char** tamanios = string_split(tamanio, ",");
    int cantidadTamanios = 0;
    while(tamanios[cantidadTamanios] != NULL)
        cantidadTamanios++;
    int* tamaniosInt = malloc(cantidadTamanios*sizeof(int));
    for(int i = 0; i < cantidadTamanios; i++)
        tamaniosInt[i] = atoi(tamanios[i]);
    paquete->codigo_operacion=IO_STDOUT_WRITE;
    paquete->buffer->size = cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int) + sizeof(int)*3;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    //enviar la cantidad de direcciones, las direcciones, la cantidad de tamanios, los tamanios y el pid
    memcpy(paquete->buffer->stream, &cantidadDirecciones, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), direccionesInt, cantidadDirecciones*sizeof(int));
    memcpy(paquete->buffer->stream + cantidadDirecciones*sizeof(int)+sizeof(int), &cantidadTamanios, sizeof(int));
    memcpy(paquete->buffer->stream + cantidadDirecciones*sizeof(int)+2*sizeof(int), tamaniosInt, cantidadTamanios*sizeof(int));
    memcpy(paquete->buffer->stream + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int)+2*sizeof(int), &pid, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    char *buffer;
    recv(socketClienteIO, &buffer, sizeof(buffer), 0);
    //free(direcciones);
    //free(tamanios);
    string_array_destroy(direcciones);
    string_array_destroy(tamanios);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    free(direccionesInt);
    free(tamaniosInt);
    pasarAReady(proceso);
}

void ejecutar_io_stdin_read(Interfaz *interfaz, t_pcb *proceso){
    char* direccionFisica=interfaz->parametro1;
    char* tamanio=interfaz->parametro2;
    int socketClienteIO = interfaz->socket_interfaz;
    int pid = proceso->pid;
    // direccionFisica contiene todas las direcciones separadas por una coma, necesito separarlas
    char** direcciones = string_split(direccionFisica, ",");
    int cantidadDirecciones = 0;
    while(direcciones[cantidadDirecciones] != NULL)
        cantidadDirecciones++;
    int* direccionesInt = malloc(cantidadDirecciones*sizeof(int));
    for(int i = 0; i < cantidadDirecciones; i++)
        direccionesInt[i] = atoi(direcciones[i]);
    t_paquete* paquete=malloc(sizeof(t_paquete)); //VER DE CAMBIAR ESTO POR CREARPAQUETE()
    // tamanio contiene todos los tamannios separados por una coma, necesito separarlos
    char** tamanios = string_split(tamanio, ",");
    int cantidadTamanios = 0;
    while(tamanios[cantidadTamanios] != NULL)
        cantidadTamanios++;
    int* tamaniosInt = malloc(cantidadTamanios*sizeof(int));
    for(int i = 0; i < cantidadTamanios; i++)
        tamaniosInt[i] = atoi(tamanios[i]);
    paquete->codigo_operacion=IO_STDIN_READ;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int) + sizeof(int)*3;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    //enviar la cantidad de direcciones, las direcciones, la cantidad de tamanios, los tamanios y el pid
    memcpy(paquete->buffer->stream, &cantidadDirecciones, sizeof(int));
    memcpy(paquete->buffer->stream+sizeof(int), direccionesInt, cantidadDirecciones*sizeof(int));
    memcpy(paquete->buffer->stream + cantidadDirecciones*sizeof(int)+sizeof(int), &cantidadTamanios, sizeof(int));
    memcpy(paquete->buffer->stream + cantidadDirecciones*sizeof(int)+2*sizeof(int), tamaniosInt, cantidadTamanios*sizeof(int));
    memcpy(paquete->buffer->stream + cantidadDirecciones*sizeof(int) + cantidadTamanios*sizeof(int)+2*sizeof(int), &pid, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    char *buffer;
    recv(socketClienteIO, &buffer, sizeof(buffer), 0);
    //free(direcciones);
    //free(tamanios);
    string_array_destroy(direcciones);
    string_array_destroy(tamanios);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    free(direccionesInt);
    free(tamaniosInt);
    pasarAReady(proceso);
     //sem_post(&io_global_actual->liberacion_semaforo);
}

//IO_GEN_SLEEP [Interfaz, UnidadesDeTrabajo]
void dormir_IO(Interfaz *interfaz, t_pcb *proceso){  
    //char* interfaz=interfaz->parametro1;
    char* tiempo=interfaz->parametro2;
    int pid = proceso->pid;
    //log_info(logger, "tiempo recibido %s", tiempo);
    //log_info(logger, "interfaz recibida %s", interfaz->nombre_interfaz);
    int socketClienteIO = interfaz->socket_interfaz;
    //log_info(logger, "se recibio el socket %d", socketClienteIO);
    enviarMensajeGen(socketClienteIO, interfaz->nombre_interfaz, tiempo, pid);
    //log_info(logger, "antes de recibir msj");
    //Recibir mensaje de confirmacion de IO
    recibirMsjIO( socketClienteIO);
    //log_info(logger, "luego e recobor msj");
    //log_warning(logger, "Proceso <%d> desbloqueado por IO_GEN_SLEEP", proceso->pid);
    pasarAReady(proceso);
}