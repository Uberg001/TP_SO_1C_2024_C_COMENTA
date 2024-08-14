#include <conexionMemoria/conexionMemoria.h>

int conexionAMemoria; 

int conexionMemoria() {
    while(1) {
        conexionAMemoria = conexion("MEMORIA");

        if(conexionAMemoria != -1){
            return 0;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAMemoria);
            sleep(5);
        }
    }
}
