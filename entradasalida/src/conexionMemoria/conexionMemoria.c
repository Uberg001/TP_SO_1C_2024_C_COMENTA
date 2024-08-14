#include <conexionMemoria/conexionMemoria.h>

void conexionIOMemoria(char **argv) {
    //char* nombreInterfaz = argv[1];
    while(1){
        fd_memoria = conexion("MEMORIA");
        if(fd_memoria != -1){
            enviarHandshakeMemoria();
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", fd_memoria);
            sleep(5);
        }
    }
}

void enviarHandshakeMemoria() {
    //size_t bytes;
    int32_t handshake = 1;
    int32_t result;
    log_info(logger, "Enviando handshake a memoria");
    send(fd_memoria, &handshake, sizeof(int32_t), 0);
    recv(fd_memoria, &result, sizeof(int32_t), MSG_WAITALL);
    //log_info(logger, "Handshake recibido: %d", result);
    if (result == 0) {
        log_info(logger, "Handshake OK");
    } else {
        log_error(loggerError, "Handshake ERROR");
    }
}