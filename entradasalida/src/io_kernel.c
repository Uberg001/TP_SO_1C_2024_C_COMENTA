#include "../include/io_kernel.h"
#define BUFFER_SIZE 1024

void io_atender_kernel(){
	bool control=1;
	while (control) {
    	int cod_op = recibirOperacion(fd_kernel);
    	switch (cod_op) {
    	case MENSAJE:
   	 		recibirMensaje(fd_kernel);
   	 		break;
   		case IO_GEN_SLEEP:
   			//log_info(logger, "GEN SLEEP recibido");
			recibir_mensaje_y_dormir(fd_kernel);
   			break;
		case IO_STDIN_READ:
   			//log_info(logger, "STDIN READ recibido");
			manejarSTDINREAD(fd_kernel);
   			break;
		case IO_STDOUT_WRITE:
   			//log_info(logger, "STDOUT WRITE recibido");
			manejarSTDOUTWRITE(fd_kernel);
   			break;
		case IO_FS_CREATE:
            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            //log_info(logger, "FS CREATE recibido");
			manejarFS_CREATE(fd_kernel);
			break;
		case IO_FS_DELETE:
            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
			//log_info(logger, "FS DELETE recibido");
            manejarFS_DELETE(fd_kernel);   
			break;
		case IO_FS_READ:
            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
			//log_info(logger, "FS READ recibido");
            manejarFS_READ(fd_kernel);
			break;
		case IO_FS_TRUNCATE:
            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
			//log_info(logger, "FS TRUNCATE recibido");
            manejarFS_TRUNCATE(fd_kernel);
			break;
		case IO_FS_WRITE:
            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
			//log_info(logger, "FS WRITE recibido");
            manejarFS_WRITE(fd_kernel);
			break;
    	case -1:
   			log_error(logger, "Kernel se desconectó. Terminando servidor");
   			control = 0;
   			break;
            exit(0);
    	default:
   		 	log_warning(logger,"Operacion desconocida. No quieras meter la pata");
   	 		break;
    	}
	}
}

void manejarFS_CREATE(int socketCliente) {
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2,pid;    
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    //printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    //printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&longitud2, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    //printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&pid, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int));
    // Copiar la cadena recibida
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 3*sizeof(int)+longitud1, longitud2);
    // Asegurarse de que la cadena esté terminada en nulo
    nombrearchivo[longitud2] = '\0';
    //printf("Nombre de archivo: %s\n", nombrearchivo);
    log_info(logger, "PID: <%d> - Crear Archivo: <%s>", pid, nombrearchivo);
    crearArchivo2(nombrearchivo);
    //enviarMensaje("OK", socketCliente);
    char *mensje="ok";
    send(socketCliente, &mensje, sizeof(mensje), 0);
    //free(mensje);
}

void manejarFS_DELETE(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2, pid;
    char nombreinterfaz[2048], nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    //printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    //printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&longitud2, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    //printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&pid, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int));
    //printf("PID: %d\n", pid);
    // Copiar la cadena recibida
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 3*sizeof(int)+longitud1, longitud2);
    // Asegurarse de que la cadena esté terminada en nulo
    nombrearchivo[longitud2] = '\0';
    //printf("Nombre de archivo: %s\n", nombrearchivo);
    log_info(logger, "PID: <%d> - Eliminar Archivo: <%s>", pid, nombrearchivo);
    delete_file(nombrearchivo);
    char *mensje="ok";
    send(socketCliente, &mensje, sizeof(mensje), 0);

}

void manejarFS_TRUNCATE(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2, nuevoTamanio, pid;
    char nombreinterfaz[2048], nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    //printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    //printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&nuevoTamanio, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    //printf("Nuevo tamanio: %d\n", nuevoTamanio);
    memcpy(&longitud2, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int));
    //printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&pid, buffer + sizeof(op_code)+3*sizeof(int)+longitud1, sizeof(int));
    //printf("PID: %d\n", pid);
    // Copiar la cadena recibida
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 4*sizeof(int)+longitud1, longitud2);
    // Asegurarse de que la cadena esté terminada en nulo
    nombrearchivo[longitud2] = '\0';
    //printf("Nombre de archivo: %s\n", nombrearchivo);
    log_info(logger, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%d>", pid, nombrearchivo, nuevoTamanio);
    truncarArchivo2(nombrearchivo,nuevoTamanio, pid);
    //enviarMensaje("OK", socketCliente);
    char *mensje="ok";
    send(socketCliente, &mensje, sizeof(mensje), 0);
}

void manejarFS_READ(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2, punteroArchivo, pid;
    int *direccion=malloc(sizeof(int)*20);
    int *tamanio=malloc(sizeof(int)*20);
    char nombreinterfaz[2048], nombrearchivo[2048];
    int cantidaddirec, cantidadtamanio;
    memset(direccion, 0, 20 * sizeof(int));
    memset(tamanio, 0, 20 * sizeof(int));
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    // printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    // printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&cantidaddirec, buffer + sizeof(op_code) + sizeof(int)+longitud1, sizeof(int));
    // printf("cantidad de direcciones: %d\n", cantidaddirec);
    memcpy(direccion, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int)*cantidaddirec);
    memcpy(&cantidadtamanio, buffer + sizeof(op_code)+2*sizeof(int)+longitud1+sizeof(int)*cantidaddirec, sizeof(int));
    // printf("cantidad de tamanios: %d\n", cantidadtamanio);
    memcpy(tamanio, buffer + sizeof(op_code)+3*sizeof(int)+longitud1+sizeof(int)*cantidaddirec, sizeof(int)*cantidadtamanio);
    //printf("tamanio: %d\n", tamanio);
    memcpy(&punteroArchivo, buffer + sizeof(op_code)+3*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, sizeof(int));
    // printf("pointer: %d\n", punteroArchivo);
    memcpy(&longitud2, buffer + sizeof(op_code) + 4*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, sizeof(int));
    // printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&pid, buffer + sizeof(op_code) + 5*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, sizeof(int));
    //printf("PID: %d\n", pid);
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 6*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, longitud2);
    nombrearchivo[longitud2] = '\0';
    //printf("Nombre de archivo: %s\n", nombrearchivo);
    int cantidadtotal=cantidaddirec;
    int tamanioTotal=0;
    int puntero2=0;
    for (int i = 0; i < cantidadtotal; i++)
        tamanioTotal += tamanio[i];
    log_info(logger, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>", pid, nombrearchivo, tamanioTotal, punteroArchivo);
    for(int i=0; i<cantidadtotal; i++){
        //log_info(logger, "Tamanio recibido: %d", tamanio[i]);
        //log_info(logger, "Direccion recibida: %d", direccion[i]);
        char *datosLeidos= leerDatosDesdeArchivo(nombrearchivo, punteroArchivo+puntero2, tamanio[i]);
        //printf("Datos leidos: %s\n", datosLeidos);
        puntero2+=tamanio[i];
        enviarAImprimirAMemoria(datosLeidos,direccion[i], fd_memoria, pid);
        //usleep(1000*1000);
        char *recibido="";
        recv(fd_memoria, &recibido, sizeof(recibido), 0);
        //log_info(logger, "OK DE MEMORIA");
        free(datosLeidos);
    }
    free(direccion);
    free(tamanio);
    char *mensje="ok";
    send(fd_kernel, &mensje, sizeof(mensje), 0);
}

void manejarFS_WRITE(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2, punteroArchivo, pid;
    int* direccion=malloc(sizeof(int)*20);
    int* tamanio=malloc(sizeof(int)*20);
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    int cantidaddirec, cantidadtamanio;
    memset(direccion, 0, 20 * sizeof(int));
    memset(tamanio, 0, 20 * sizeof(int));
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    // printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    //printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&cantidaddirec, buffer + sizeof(op_code) + sizeof(int)+longitud1, sizeof(int));
    //printf("cantidad de direcciones: %d\n", cantidaddirec);
    memcpy(direccion, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int)*cantidaddirec);
    memcpy(&cantidadtamanio, buffer + sizeof(op_code)+2*sizeof(int)+longitud1+sizeof(int)*cantidaddirec, sizeof(int));
    //printf("cantidad de tamanios: %d\n", cantidadtamanio);
    memcpy(tamanio, buffer + sizeof(op_code)+3*sizeof(int)+longitud1+sizeof(int)*cantidaddirec, sizeof(int)*cantidadtamanio);
    //printf("tamanio: %d\n", tamanio);
    memcpy(&punteroArchivo, buffer + sizeof(op_code)+3*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, sizeof(int));
    // printf("pointer: %d\n", punteroArchivo);
    memcpy(&longitud2, buffer + sizeof(op_code) + 4*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, sizeof(int));
    // printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&pid, buffer + sizeof(op_code) + 5*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, sizeof(int));
    // printf("PID: %d\n", pid);
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 6*sizeof(int)+longitud1+sizeof(int)*cantidaddirec+sizeof(int)*cantidadtamanio, longitud2);
    nombrearchivo[longitud2] = '\0';
    // printf("Nombre de archivo: %s\n", nombrearchivo);
    int cantidadtotal=cantidaddirec;
    int tamanioTotal=0;
    char *cadenaCompleta=malloc(126);
    memset(cadenaCompleta, 0, 126);
    for(int i=0; i<cantidadtotal; i++)
        tamanioTotal+=tamanio[i];
    log_info(logger, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Escribir: <%d> - Puntero Archivo: <%d>", pid, nombrearchivo, tamanioTotal, punteroArchivo);
    for(int i=0; i<cantidadtotal; i++){
        //log_info(logger, "Tamanio recibido: %d", tamanio[i]);
        //log_info(logger, "Direccion recibida: %d", direccion[i]);
        enviarDireccionTamano(direccion[i],tamanio[i],pid,fd_memoria);
       // usleep(1000*1000);
        char* recibido=malloc(256);
        memset(recibido, 0, 256);
	    //if(i==0)
	    //	cadenaCompleta[0]='\0';
	    recv(fd_memoria, recibido, tamanio[i], 0);
	    //printf("Mensaje recibido de memoria:%s\n", recibido);
	    //ir concatenando los mensajes
	    strncat(cadenaCompleta, recibido, tamanio[i]);
        free(recibido);
    }
    //printf("Mensaje completo:%s\n", cadenaCompleta);
	escribirCadenaEnArchivo(nombrearchivo, cadenaCompleta, punteroArchivo);
    char *mensje="ok";
    send(fd_kernel, &mensje, sizeof(mensje), 0);
    if(cadenaCompleta!=NULL)
        free(cadenaCompleta);
    free(direccion);
    free(tamanio);
}

//IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
void manejarSTDINREAD(int socketCliente) {
    int  pid;
    //voy a recibir un array de tamanios y otro array de direcciones
    int *tamanios = malloc(sizeof(int)*20);
    int *direcciones = malloc(sizeof(int)*20);
    int cantidad;
    recibirEnteros3(socketCliente, tamanios, direcciones, &pid, &cantidad);
    // Loguear los parámetros recibidos
    /*for(int i=0; i<cantidad; i++){
        log_info(logger, "Tamanio recibido: %d", tamanios[i]);
        log_info(logger, "Direccion recibida: %d", direcciones[i]);
    }*/
    log_info(logger, "PID: <%d> - Operacion: <IO_STDIN_READ>", pid);
    // Leer una línea de texto usando readline
    char* texto = readline(">");
    //tengo que dividir el texto ingresado en partes de tamanio maximo tamanios[0]
    //y enviar cada parte a memoria en la direccion direcciones[i]
    // Copiar los datos desde el archivo mapeado al buffer de datos leídos
    char *datosLeidos = malloc(126);
    memset(datosLeidos, 0, 126);
    int tamanotexto=0; 
    //divido el texto en partes
    for(int i=0; i<cantidad; i++){
        memcpy(datosLeidos, texto + tamanotexto, tamanios[i]);
        datosLeidos[tamanios[i]] = '\0';
        //printf("Texto a enviar a memoria: %s\n", datosLeidos);
        tamanotexto += strlen(datosLeidos);
        enviarAImprimirAMemoria(datosLeidos,direcciones[i], fd_memoria, pid);//estos datos se deben escribir en la direccion de memoria
        //recibir un mensaje de confirmacion de que se escribio en memoria
        char *recibido="";
        recv(fd_memoria, &recibido, sizeof(recibido), 0);
        //log_info(logger, "OK DE MEMORIA");
        //usleep(1000*1000);
    }
    //recv(fd_memoria, &recibido, sizeof(recibido), 0);
    // Liberar la memoria reservada
    free(texto);
    free(datosLeidos);
    free(tamanios);
    free(direcciones);
    //printf("Todo liberado!\n");
    char *mensje="ok";
    send(fd_kernel, &mensje, sizeof(mensje), 0);
}

void manejarSTDOUTWRITE(int socketCliente) {
    int *tamanios=malloc(sizeof(int)*20);
    int *direcciones=malloc(sizeof(int)*20);
    int pid, cantidad;
    recibirEnteros3(socketCliente, tamanios, direcciones, &pid, &cantidad);
    // Loguear los parámetros recibidos
    /*for(int i=0; i<cantidad; i++){
        log_info(logger, "Tamanio recibido: %d", tamanios[i]);
        log_info(logger, "Direccion recibida: %d", direcciones[i]);
    }*/
    log_info(logger, "PID: <%d> - Operacion: <IO_STDOUT_WRITE>", pid);
    char *cadenaCompleta=malloc(126);
    memset(cadenaCompleta, 0, 126);
    //divido el texto en partes
    for(int i=0; i<cantidad; i++){
        enviarDireccionTamano(direcciones[i],tamanios[i],pid,fd_memoria);
        char* recibido=malloc(256);
        memset(recibido, 0, 256);
	    //if(i==0)
	    //	cadenaCompleta[0]='\0';
	    recv(fd_memoria, recibido, tamanios[i], 0);
	    //printf("Mensaje recibido de memoria:%s\n", recibido);
	    //ir concatenando los mensajes
	    strncat(cadenaCompleta, recibido, tamanios[i]);
        //log_error(logger, "mensaje concatenado: %s", cadenaCompleta);
        free(recibido);
    }
    printf("%s\n", cadenaCompleta);
	char *mensje="ok";
    send(fd_kernel, &mensje, sizeof(mensje), 0);
    if(cadenaCompleta!=NULL)
        free(cadenaCompleta);
    free(tamanios);
    free(direcciones);
}

void recibir_mensaje_y_dormir(int socket_cliente) {
    // Buffer para almacenar el mensaje recibido
    char buffer[1024];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket_cliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    int unidades, pid;
    memcpy(&unidades, buffer+sizeof(op_code), sizeof(int));
    memcpy(&pid, buffer+sizeof(op_code)+sizeof(int), sizeof(int));
    char nombre[100];
    int longitud=bytes_recibidos-sizeof(op_code)-2*sizeof(int);
    memcpy(nombre, buffer+sizeof(op_code)+2*sizeof(int), longitud);
    nombre[longitud] = '\0';
    log_info(logger, "PID: <%d> - Operacion: <IO_GEN_SLEEP>", pid);
    //log_info(logger, "Nombre recibido: %s", nombre);
	//log_info(logger, "Tiempo a dormir recibido: %d", unidades); 
	//log_info(logger, "Tiempo a dormir calculado: %f", unidades*TIEMPO_UNIDAD_TRABAJO/1000.0); // ejemplo: 10*250/1000 = 2.5seg
	//log_info(logger, "Antes de dormir");
	sleep(unidades* TIEMPO_UNIDAD_TRABAJO/1000.0);
	//log_info(logger, "Despues de dormir");
	//mandar mensaje luego de dormir a kernel
    send(socket_cliente, "OK", 2, 0);
	//enviarMensaje("OK", socket_cliente);
	return;
}

void enviarAImprimirAMemoria(const char *mensaje, int direccion, int socket, int pid) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 100;
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

void enviarDireccionTamano(int direccion,int tamano, int pid, int socket) {
   t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 101;
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

void recibirEnteros3(int socket, int *tamanio, int *direccion, int *pid, int *cantidad) {
    char buffer[2048];
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    int cantidaddireciones,cantidadtamanios;
    memcpy(&cantidaddireciones, buffer+sizeof(op_code), sizeof(int));
    //printf("Cantidad de direcciones: %d\n", cantidaddireciones);
    memcpy(direccion, buffer+sizeof(op_code)+sizeof(int), sizeof(int)*cantidaddireciones);
    memcpy(&cantidadtamanios, buffer+sizeof(op_code)+sizeof(int)+sizeof(int)*cantidaddireciones, sizeof(int));
    memcpy(tamanio, buffer+sizeof(op_code)+sizeof(int)+sizeof(int)*cantidaddireciones+sizeof(int), sizeof(int)*cantidadtamanios);
    memcpy(pid, buffer+sizeof(op_code)+sizeof(int)+sizeof(int)*cantidaddireciones+sizeof(int)+sizeof(int)*cantidadtamanios, sizeof(int));
    *cantidad = cantidaddireciones;
}