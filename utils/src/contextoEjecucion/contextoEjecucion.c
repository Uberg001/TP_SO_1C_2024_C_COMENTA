#include <contextoEjecucion/contextoEjecucion.h>
t_contexto * contextoEjecucion = NULL;

void enviarContextoBeta(int socket, t_contexto* contexto) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = CONTEXTOEJECUCION;
    paquete->buffer = malloc(sizeof(t_buffer));

    // Calcular el tamaño del buffer necesario para la estructura
    paquete->buffer->size = sizeof(contexto->pid) + sizeof(contexto->programCounter) + sizeof(contexto->instruccionesLength) ;

     // Calcular el tamaño de los registros
    paquete->buffer->size  += 4 * (3 + 10)+2*10; // AX, BX, CX, DX (1 bytes cada uno) + EAX, EBX, ECX, EDX (4 bytes cada uno)

    //calcula tamaño del motivo
    paquete->buffer->size  += sizeof(contextoEjecucion->motivoDesalojo->motivo) +
                   sizeof(contextoEjecucion->motivoDesalojo->parametrosLength);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        paquete->buffer->size  += strlen(contextoEjecucion->motivoDesalojo->parametros[i]) + 1; // +1 para el terminador nulo
    }

    //calculo tamaño de tiempo de cpu
    paquete->buffer->size  += sizeof(contextoEjecucion->tiempoDeUsoCPU);

    //calculo para quantum, algoritmo y el flag de fin de quantum
    paquete->buffer->size+=sizeof(contexto->quantum)+sizeof(contexto->algoritmo)+sizeof(contexto->fin_de_quantum);

    paquete->buffer->stream = malloc(paquete->buffer->size);

    // Serializar los datos en el buffer
    int desplazamiento = 0;
    
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->pid), sizeof(contexto->pid));
    desplazamiento += sizeof(contexto->pid);

    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->programCounter), sizeof(contexto->programCounter));
    desplazamiento += sizeof(contexto->programCounter);
 
   memcpy(paquete->buffer->stream + desplazamiento, &(contexto->instruccionesLength), sizeof(contexto->instruccionesLength));
    desplazamiento += sizeof(contexto->instruccionesLength);

    // Serializar los registros
    char* registros[] = {"AX", "BX", "CX", "DX", "EAX", "EBX", "ECX", "EDX","SI","DI"};
    for (int i = 0; i < 10; i++) {
        char* registro = dictionary_get(contexto->registrosCPU, registros[i]);
        int registro_length = (i < 4) ? 3 : 10;
        memcpy(paquete->buffer->stream + desplazamiento, registro, registro_length);
        desplazamiento += registro_length;
    }

    //Serializar el motivo
    memcpy(paquete->buffer->stream + desplazamiento, &(contextoEjecucion->motivoDesalojo->motivo), sizeof(contextoEjecucion->motivoDesalojo->motivo));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->motivo);

    
    memcpy(paquete->buffer->stream + desplazamiento, &(contextoEjecucion->motivoDesalojo->parametrosLength), sizeof(contextoEjecucion->motivoDesalojo->parametrosLength));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->parametrosLength);

    
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        int parametro_length = strlen(contextoEjecucion->motivoDesalojo->parametros[i]) + 1;
        memcpy(paquete->buffer->stream + desplazamiento, contextoEjecucion->motivoDesalojo->parametros[i], parametro_length);
        desplazamiento += parametro_length;
    }

    //Serializacion de quantum
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->quantum), sizeof(contexto->quantum));
    desplazamiento += sizeof(contexto->quantum);

    //Serializacion de algoritmo
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->algoritmo),sizeof(contexto->algoritmo));
    desplazamiento +=sizeof(contexto->algoritmo);

    //serializo el tiempo de cpu
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->tiempoDeUsoCPU), sizeof(contexto->tiempoDeUsoCPU));
    desplazamiento += sizeof(contexto->tiempoDeUsoCPU);

    //serializo el flag de fin de quantum
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->fin_de_quantum), sizeof(contexto->fin_de_quantum));
    desplazamiento += sizeof(contexto->fin_de_quantum);

    // Calcular el tamaño total del paquete a enviar
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    
    // Serializar el paquete
    void *a_enviar = serializarPaquete(paquete, bytes);

    // Enviar el paquete a través del socket
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE);
    }

    // Liberar memoria
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(a_enviar);
    free(paquete);
}




void recibirContextoBeta(int socket) {
       char buffer[2048];
        if (contextoEjecucion != NULL) destroyContextoUnico ();
	iniciarContexto ();

    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }

    

    int desplazamiento = sizeof(op_code);

    memcpy(&(contextoEjecucion->pid), buffer + desplazamiento, sizeof(contextoEjecucion->pid));
    desplazamiento += sizeof(contextoEjecucion->pid);

    memcpy(&(contextoEjecucion->programCounter), buffer + desplazamiento, sizeof(contextoEjecucion->programCounter));
    desplazamiento += sizeof(contextoEjecucion->programCounter);

    memcpy(&(contextoEjecucion->instruccionesLength), buffer + desplazamiento, sizeof(contextoEjecucion->instruccionesLength));
    desplazamiento += sizeof(contextoEjecucion->instruccionesLength);
    
    //deserealizo los registros
    char* registro;
    char nombreRegistro[4];

    // AX, BX, CX, DX (1 byte cada uno)
    for (char nombre = 'A'; nombre <= 'D'; nombre++) {
        registro = malloc(4); // 1 byte + terminador nulo
        memcpy(registro, buffer + desplazamiento, 3);
        registro[3] = '\0';
        desplazamiento += 3;

        snprintf(nombreRegistro, 3, "%cX", nombre);
        dictionary_put(contextoEjecucion->registrosCPU, nombreRegistro, registro);
    }

    // EAX, EBX, ECX, EDX (4 bytes cada uno)
    for (char nombre = 'A'; nombre <= 'D'; nombre++) {
        registro = malloc(11); // 4 bytes + terminador nulo
        memcpy(registro, buffer + desplazamiento, 10);
        registro[10] = '\0';
        desplazamiento += 10;

        snprintf(nombreRegistro, 4, "E%cX", nombre);
        dictionary_put(contextoEjecucion->registrosCPU, nombreRegistro, registro);
    }

    //SI,DI (4 bytes cada uno)
    registro=malloc(11);
    memcpy(registro, buffer + desplazamiento, 10);
    registro[10]='\0';
    desplazamiento+=10;
    dictionary_put(contextoEjecucion->registrosCPU, "SI", registro);

    registro=malloc(11);
    memcpy(registro, buffer + desplazamiento, 10);
    registro[10]='\0';
    desplazamiento+=10;
    dictionary_put(contextoEjecucion->registrosCPU, "DI", registro);

    // Deserializar el motivo
    memcpy(&(contextoEjecucion->motivoDesalojo->motivo), buffer + desplazamiento, sizeof(contextoEjecucion->motivoDesalojo->motivo));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->motivo);

    // Deserializar la longitud de los parámetros
    memcpy(&(contextoEjecucion->motivoDesalojo->parametrosLength), buffer + desplazamiento, sizeof(contextoEjecucion->motivoDesalojo->parametrosLength));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->parametrosLength);

    // Deserializar los parámetros
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        int parametro_length = strlen(buffer + desplazamiento) + 1;
        contextoEjecucion->motivoDesalojo->parametros[i] = malloc(parametro_length);
        memcpy(contextoEjecucion->motivoDesalojo->parametros[i], buffer + desplazamiento, parametro_length);
        desplazamiento += parametro_length;
    }
    
    //Deserializar quantum
    memcpy(&(contextoEjecucion->quantum), buffer + desplazamiento, sizeof(contextoEjecucion->quantum));
    desplazamiento += sizeof(contextoEjecucion->quantum);

    //Deserializar algoritmo
    memcpy(&(contextoEjecucion->algoritmo), buffer + desplazamiento, sizeof(contextoEjecucion->algoritmo));
    desplazamiento += sizeof(contextoEjecucion->algoritmo);

    //Deserializar tiempoDeUsoCPU
    memcpy(&(contextoEjecucion->tiempoDeUsoCPU), buffer + desplazamiento, sizeof(contextoEjecucion->tiempoDeUsoCPU));
    desplazamiento += sizeof(contextoEjecucion->tiempoDeUsoCPU);

    //Deserializar flag de fin de quantum
    memcpy(&(contextoEjecucion->fin_de_quantum), buffer + desplazamiento, sizeof(contextoEjecucion->fin_de_quantum));
    desplazamiento += sizeof(contextoEjecucion->fin_de_quantum);
}

void destroyContexto() {
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) 
        if (strcmp(contextoEjecucion->motivoDesalojo->parametros[i], "")) free(contextoEjecucion->motivoDesalojo->parametros[i]);
    free(contextoEjecucion->motivoDesalojo);
    free(contextoEjecucion);
    contextoEjecucion = NULL;
}

void destroyContextoUnico () {
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    free(contextoEjecucion->motivoDesalojo);
    free(contextoEjecucion);
    contextoEjecucion = NULL;
}


void iniciarContexto(){
    contextoEjecucion = malloc(sizeof(t_contexto));
	contextoEjecucion->instruccionesLength = 0;
	contextoEjecucion->pid = 0;
	contextoEjecucion->programCounter = 0;
	contextoEjecucion->registrosCPU = dictionary_create();
    contextoEjecucion->tiempoDeUsoCPU = 0;
    contextoEjecucion->motivoDesalojo = (t_motivoDeDesalojo *)malloc(sizeof(t_motivoDeDesalojo));
    contextoEjecucion->motivoDesalojo->parametros[0] = "";
    contextoEjecucion->motivoDesalojo->parametros[1] = "";
    contextoEjecucion->motivoDesalojo->parametros[2] = "";
    contextoEjecucion->motivoDesalojo->parametros[3] = "";
    contextoEjecucion->motivoDesalojo->parametros[4] = "";
    contextoEjecucion->motivoDesalojo->parametrosLength = 0;
    contextoEjecucion->motivoDesalojo->motivo = 0;
	contextoEjecucion->quantum=0;
    contextoEjecucion->algoritmo=0;
    contextoEjecucion->fin_de_quantum=false;
}
