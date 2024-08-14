#include <conexionIO/conexionIO.h>

void ejecutarServidorIO(){
    //tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");
     //char *puertoEscucha = confGet("PUERTO_ESCUCHA");
    //int socketMemoria = alistarServidorMulti(puertoEscucha);
    while (1) {
        log_info(logger,"Esperando conexiones con IO...");
        pthread_t thread;
        
        int *socketClienteIO = malloc(sizeof(int));
        
        *socketClienteIO = esperarCliente(server_fd);
        log_info(logger, "IO conectado, en socket: %d",*socketClienteIO);

        //hacerHandshake(*socketClienteIO);
        //ejecutarServidor(*socketClienteIO);

         pthread_create(&thread,
                        NULL,
                         (void*) ejecutarServidor,
                         socketClienteIO);
         pthread_detach(thread);
    }
    //return EXIT_SUCCESS;
}

void* ejecutarServidor(void* socketCliente) {
    int sock = *(int*)socketCliente;
    free(socketCliente);
    int tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");
    // Hacer handshake
    hacerHandshake(sock);
    while (1) {
        int peticion;
        ssize_t bytes = recv(sock, &peticion, sizeof(peticion), 0);
        if (bytes <= 0) {
            if (bytes == 0) {
                log_info(logger, "El cliente cerró la conexión");
            } else {
                log_error(logger, "Error en la recepción de la petición: ");
            }
            close(sock);
            return NULL;
        }
        log_debug(logger, "Se recibió petición %d del IO", peticion);
        switch (peticion) {
            case MENSAJE:
                usleep(tiempo*1000);
   	 		    char* mensaje = recibirMensaje(sock);
                log_info(logger, "Mensaje recibido: %s", mensaje);            
                break;
            case 100: //INVENTE UN NUMERO DE OPCODE PARA CUANDO IO (STDIN O FS_READ) ENVIA EL MENSAJE A ESCRIBIR EN MEMORIA 
                usleep(tiempo*1000);
                //log_info(logger, "IO envía mensaje a escribir en memoria");
                int dir, pid;
                char cadena[2048]="";
                recibirDirYCadena(sock, &dir, &pid, cadena);
                //cadena[strlen(cadena)] = '\0';
                log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion Física: <%d> - Valor: <%s>", pid, dir, cadena); 
                memcpy((char*)mf->memoria + dir, cadena, strlen(cadena));
                /*char* datoEscrito= malloc(strlen(cadena));//ACA VERIFICO QUE SE ESCRIBIO BIEN EN MEMORIA!!!!!!!!
                memcpy(datoEscrito, (char*)mf->memoria + dir, strlen(cadena));
                datoEscrito[strlen(cadena)] = '\0';
                printf("Dato escrito: %s\n", datoEscrito);
                free(datoEscrito);*/
                char *mensaje2;
               // memset(mensaje2, 0, 3);
                mensaje2 = "OK";
                send(sock, &mensaje2, sizeof(mensaje2), 0);
                //enviarMensaje(mensaje2, sock);
                break;
            case 101: //IDEM PARA STDOUT, ACA LEO DE MEMORIA Y ENVIO A IO (STDOUT o FS_WRITE)
                usleep(tiempo*1000);
                //log_info(logger, "MEMORIA envía mensaje a IO segun direccion y tamaño");
                int dir2, tamano, pid2;
                recibirDireccionyTamano(sock, &dir2, &pid2, &tamano);
                //printf("Tamaño: %d\n", tamano);
                char* datosLeidos = malloc(256);
                memset(datosLeidos, 0, 256);
                memcpy(datosLeidos, (char*)mf->memoria + dir2, tamano);
                datosLeidos[tamano]='\0';
                log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion Física: <%d> - Valor: <%s>", pid2, dir2, datosLeidos);
                //enviarMensaje(datosLeidos, sock);
                send(sock, datosLeidos, tamano, 0);
                free(datosLeidos);
                break;
            case -1:
                log_error(logger, "IO se desconectó");
                return NULL;
                break;
            default:
                log_warning(logger, "Operación desconocida del IO. Peticion %d", peticion);
                break;
        }
    }
    close(sock);
    return NULL;
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

void recibirDirYCadena(int socket, int *dir, int *pid, char* cadena) {
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
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
    memcpy(dir, buffer + sizeof(op_code), sizeof(int));
    memcpy(pid, buffer + sizeof(op_code) + sizeof(int), sizeof(int));
    // Calcular la longitud de la cadena recibida
    int longitud_cadena = bytes_recibidos - 2*sizeof(int) - sizeof(op_code);
    // Asegurarse de no exceder el tamaño del buffer
    if (longitud_cadena > 2047) 
        longitud_cadena = 2047;
    // Copiar la cadena recibida
    memcpy(cadena, buffer + 2*sizeof(int) + sizeof(op_code), longitud_cadena);
    // Asegurarse de que la cadena esté terminada en nulo
   // cadena[longitud_cadena] = '\0';
}

void recibirDireccionyTamano(int socket, int *dir, int *pid, int *tamano) {
    char buffer[sizeof(op_code) + 3 * sizeof(int)];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(op_code) + 3 * sizeof(int)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    memcpy(dir, buffer + sizeof(op_code), sizeof(int));
    memcpy(tamano, buffer + sizeof(op_code) + sizeof(int), sizeof(int));
    memcpy(pid, buffer + sizeof(op_code) + 2*sizeof(int), sizeof(int));
}