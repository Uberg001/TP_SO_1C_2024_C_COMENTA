#include <conexionKernel/conexionKernel.h>
#include "conexionKernel.h"
void envioNombreInterfaz(char *nombreInterfaz, int fd_kernel);
void envioTipoInterfaz(char *tipoInterfaz, int fd_kernel);
void conexionKernel(char **argv) {
   char *nombreInterfaz = argv[1];
    while(1){
        fd_kernel = conexion("KERNEL");
        
        if(fd_kernel != -1){
            enviarHandshake(nombreInterfaz);
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", fd_kernel);
            sleep(5);
        }
    }
}
 void enviarHandshake(char *nombreInterfaz){
    //size_t bytes;
    int32_t handshake = 1;
    int32_t result;
    log_info(logger, "Enviando handshake al servidor");
    send(fd_kernel, &handshake, sizeof(int32_t), 0);
    recv(fd_kernel, &result, sizeof(int32_t), MSG_WAITALL);
    if (result == 0){
        log_info(logger, "Handshake OK");
        envioNombreInterfaz(nombreInterfaz, fd_kernel);
        envioTipoInterfaz(TIPO_INTERFAZ,fd_kernel);
    }
    else{
        log_error(logger, "Handshake ERROR");
    }
}

void envioTipoInterfaz(char *tipoInterfaz, int fd_kernel)
{
    log_info(logger, "Tipo de interfaz es: %s", tipoInterfaz);

    // Enviar mensaje al servidor
    send(fd_kernel, tipoInterfaz, strlen(tipoInterfaz), 0);
    
}

void envioNombreInterfaz(char *nombreInterfaz, int fd_kernel)
{
    log_info(logger, "Nombre de interfaz es: %s", nombreInterfaz);

    // Enviar mensaje al servidor
    send(fd_kernel, nombreInterfaz, strlen(nombreInterfaz), 0);
    //printf("Message sent: %s\n", nombreInterfaz);
}