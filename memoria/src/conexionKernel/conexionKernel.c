#include <conexionKernel/conexionKernel.h>
#include "conexionKernel.h"

int PID;
int cantidadMaximaPaginas;
uint32_t direccionBasePagina;
uint32_t tamanioPagina;
char* pathInstrucciones;

uint32_t recibirPID(int socketCliente) {

	int size, desplazamiento=0; 
	uint32_t pid; 

	void* buffer = recibirBuffer(socketCliente, &size);
	desplazamiento += sizeof(int);
	memcpy(&(pid), buffer + desplazamiento, sizeof(uint32_t));

	free (buffer);
	return pid; 

}

int ejecutarServidorKernel(int *socketCliente) {
    //logger=cambiarNombre(logger,"conexion con kernel - Memoria");
    int tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");
    cantidadMaximaPaginas = confGetInt("TAM_PAGINA");
    while (1) {
        int peticionRealizada = recibirOperacion(*socketCliente);
        switch (peticionRealizada) {
            case NEWPCB: {
                usleep(tiempo*1000);
                PID = recibirPID(*socketCliente);
                Proceso *proceso = inicializar_proceso(PID, pathInstrucciones);
                //Mando el numero de instrucciones a kernel
                int n=proceso->numero_instrucciones;
                //log_info(logger,"Cantidad de instrucciones: %d",n);
                send(*socketCliente, &n, sizeof(int), 0);
                //log_info(logger,"Creacion de Proceso PID: <%d>", PID);
                break;
            }
            case ENDPCB: {
                usleep(tiempo*1000);
                PID = recibirPID(*socketCliente);
                //log_info(logger, "Eliminación de Proceso PID: <%d>", PID);
                log_info(logger, "Destrucción de Tabla de Páginas PID: <%d> - Tamaño: <%d> Páginas", PID, buscar_proceso_por_pid(PID)->tabla_paginas->paginas_asignadas);
                eliminarProcesoDeMemoria(PID);
                //free(pathInstrucciones); //ver si va o no
                break;
            }
            case MENSAJE:{
                usleep(tiempo*1000);
                pathInstrucciones=recibirMensaje(*socketCliente); //Recibo el path
                //log_info(logger,"Path de instrucciones recibido: %s",pathInstrucciones);
                break;
            }
            case -1:
                log_error(logger, "El Kernel se desconectó");
                exit(0);
                break;
            default:
                log_warning(logger, "Operación desconocida: %d", peticionRealizada);
                break;
        }
    }
    return EXIT_SUCCESS;
}

void eliminarProcesoDeMemoria(int pid) {
   Proceso *proceso = buscar_proceso_por_pid(pid);
    if (proceso != NULL) {
        //liberar los marcos de la tabla de paginas
        for (int i = proceso->tabla_paginas->paginas_asignadas - 1; i >= 0; i--) {
             EntradaTablaPaginas *entrada =list_get(proceso->tabla_paginas->entradas, i);
             int marco = entrada->numero_marco;

            // Liberar la página i
            list_remove(proceso->tabla_paginas->entradas,i);
            list_replace(mf->listaMarcosLibres, marco,false); //creo que se pone - 1 porque es el indice dee una lista y no existe el marco 0
            //reemmplazaaaa vaalor marcando false como  disponible
            free(entrada);
        }
    /*int marcosLibres=0;
    for (int i = 0; i < list_size(mf->listaMarcosLibres); i++) {
        if (list_get(mf->listaMarcosLibres,i) == false)
            marcosLibres++;
    }
    log_info(logger,"Marcos libres: %d",marcosLibres);*/
            // Elimina todas las páginas del proceso
        list_destroy(proceso->tabla_paginas->entradas);
        liberar_tabla_paginas(proceso->tabla_paginas);
        
        // Elimina todas las instrucciones del proceso
        for (int i = 0; i < proceso->numero_instrucciones; i++) {
            free(proceso->instrucciones[i]);
        }
        free(proceso->instrucciones);

        //bbusco procesoo en la lisssstaa y lo eliminoo
        for (int i = 0; i < list_size(mf->listaProcesos); i++) {
            proceso = list_get(mf->listaProcesos,i);
            if (proceso->pid == pid) {
                list_remove(mf->listaProcesos,i);
            }
        }
        // Libera la memoria del proceso
        //free(proceso); //ver si va aca o no

        //log_info(logger, "Proceso PID: <%d> eliminado correctamente", pid);
    } else {
        log_warning(logger, "Proceso PID: <%d> no encontrado para eliminación", pid);
    }
}

Proceso *buscar_proceso_por_pid(int pid) {
    pthread_mutex_lock(&mutex);
    //log_info(logger, "buscar_proceso_por_pid: %d",pid);
    Proceso *proceso = NULL;
    for (int i = 0; i < list_size(mf->listaProcesos); i++) {
        proceso = list_get(mf->listaProcesos,i);
        if (proceso->pid == pid) {
            pthread_mutex_unlock(&mutex);
            return proceso;
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}