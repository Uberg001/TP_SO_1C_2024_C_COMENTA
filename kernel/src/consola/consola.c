#include <consola/consola.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>

void str_to_upper(char *str) {
    for(int i = 0; str[i]!= '\0'; i++) {
        if(str[i] == ' ') break; // Salta si encuentra un espacio
        str[i] = toupper((unsigned char)str[i]); // Convierte a mayúscula
    }
}

//Se reciben dos archivos:El script con las funciones de kernel y el archivo de instrucciones
int ejecutarConsola () {
    char *linea;
    //logger=cambiarNombre(logger,"Consola-Kernel");
    //log_info(logger,"Consola iniciada. Por favor ingrese un comando. Puede ingresar MENU para ver los comandos disponibles.");
    while (1) {
        linea = readline(">");
        str_to_upper(linea);
        if (!linea) {
            break;
        }
        if (!strncmp(linea, "EXIT", 4)) {
            free(linea);
            exit(EXIT_SUCCESS);
            break;
        }
        //Si escribo los comandos....
        if(!strncmp(linea, "EJECUTAR_SCRIPT", 14)){
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                char* path = token;
                if (access(path, F_OK) != -1) {
                    ejecutarScript(path);
                } else {
                    // El archivo no existe o no es accesible
                    log_error(logger, "El archivo especificado no existe o no es accesible: %s", path);
                }
            } else {
                log_info(logger, "No se proporcionó un path para EJECUTAR_SCRIPT");
                ejecutarConsola ();
            }
        }
        if(!strncmp(linea, "INICIAR_PROCESO",15)){
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                char* path = token;
                    iniciarProceso(path);
            } else {
                log_info(logger, "No se proporcionó un path para INICIAR_PROCESO");
                ejecutarConsola ();
            }
        }
        if(!strncmp(linea, "FINALIZAR_PROCESO",17)){
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int pid = atoi(token); // Convierte el token en un número entero
                finalizarProceso(pid);
            } else {
                log_info(logger, "No se proporcionó un PID para FINALIZAR_PROCESO");
                ejecutarConsola ();
            }
        }
        if(!strncmp(linea, "DETENER_PLANIFICACION",21)){
            detenerPlanificacion();
        }
        if(!strncmp(linea, "INICIAR_PLANIFICACION",21)){
            iniciarPlanificacion();
        }
        if(!strncmp(linea, "MULTIPROGRAMACION", 17)) {
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int valor = atoi(token); // Convierte el token en un número entero
                modificarGradoMultiprogramacion(valor);
            } else {
                log_info(logger, "No se proporcionó un valor para MULTIPROGRAMACION");
                ejecutarConsola ();
            }
        }
        if(!strncmp(linea, "PROCESO_ESTADO", 14)) {
            procesoEstado();
        }
        if(!strncmp(linea,"MENU",strlen("MENU"))){
            log_info(logger,"EJECUTAR_SCRIPT [PATH] - Ejecuta un script con comandos de kernel\n"
                            "INICIAR_PROCESO [PATH] - Inicia un proceso\n"
                            "FINALIZAR_PROCESO [PID] - Finaliza un proceso\n"
                            "DETENER_PLANIFICACION - Detiene la planificacion\n"
                            "INICIAR_PLANIFICACION - Inicia la planificacion\n"
                            "MULTIPROGRAMACION [VALOR] - Modifica el grado de multiprogramacion\n"
                            "PROCESO_ESTADO - Muestra el estado de los procesos\n"
                            "MENU - Muestra los comandos disponibles\n"
                            "EXIT - Sale de la consola");
        }

        free(linea);
    }

    return EXIT_SUCCESS;
}

//FUNCIONES POR CONSOLA

//EJECUTAR_SCRIPT [PATH]
void ejecutarScript(const char* path) {
    FILE* file = fopen(path, "r"); //Abro el archibo en modo lectura
    if (file == NULL) {
        log_info(logger, "No se pudo abrir el archivo %s", path);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        
        if (!strncmp(line, "INICIAR_PROCESO", strlen("INICIAR_PROCESO"))) {
            char *token = strtok(line, " ");
            token = strtok(NULL, " "); 
            if (token != NULL) {
                char* tokenModificado = malloc(strlen("src") + strlen(token) + 1); // Allocate memory for the modified token
                strcpy(tokenModificado, "src"); // Copy "src" to the modified token
                strcat(tokenModificado, token); // Concatenate the original token to the modified token
                iniciarProceso(tokenModificado);
                free(tokenModificado);
            } else {
                log_info(logger, "No se proporcionó un argumento para INICIAR_PROCESO");
                ejecutarConsola ();
            }
        } 
        else if(!strncmp(line, "FINALIZAR_PROCESO", strlen("FINALIZAR_PROCESO"))) {
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int pid = atoi(token); // Convierte el token en un número entero
                finalizarProceso(pid);
            } else {
                log_info(logger, "No se proporcionó un argumento para FINALIZAR_PROCESO");
                ejecutarConsola ();
            }
        } 
        else if(!strncmp(line, "DETENER_PLANIFICACION", strlen("DETENER_PLANIFICACION"))) {
            detenerPlanificacion();
        } 
        else if(!strncmp(line, "INICIAR_PLANIFICACION", strlen("INICIAR_PLANIFICACION"))) {
            iniciarPlanificacion();
        } 
        else if(!strncmp(line, "MULTIPROGRAMACION", strlen("MULTIPROGRAMACION"))) {
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int valor = atoi(token); // Convierte el token en un número entero
                modificarGradoMultiprogramacion(valor);
            } else {
                log_info(logger, "No se proporcionó un argumento para MULTIPROGRAMACION");
                ejecutarConsola ();
            }
        } 
        else if(!strncmp(line, "PROCESO_ESTADO", strlen("PROCESO_ESTADO"))) {
            procesoEstado();
        } 
        else {
            log_info(logger, "Comando no reconocido: %s", line);
        }
    }
    fclose(file);
}

//INICIAR_PROCESO[PATH]
void iniciarProceso(const char* path) {//Creo el pcb y lo ingreso a la cola de new
    enviarPathDeInstrucciones(path);
    pthread_mutex_lock(&mutexPCB);
    t_pcb* pcb = crearPCB();
    pthread_mutex_unlock(&mutexPCB);
    ingresarANew(pcb);
} 

//FINALIZAR_PROCESO
void finalizarProceso(int pid){
    if(buscarProceso(pcbsNEW,pid)){
        eliminarProceso(pcbsNEW,pid);
    } else if(buscarProceso(pcbsREADY,pid)){
        eliminarProceso(pcbsREADY,pid);
    } else if(buscarProceso(pcbsREADYaux,pid)){
        eliminarProceso(pcbsREADYaux,pid);
    } else if (buscarProceso(pcbsExec,pid)){
        //log_error(logger, "se envia MENSAJE a CPU para que finalice el proceso");
        enviarMensaje2("USER_INTERRUPT",conexionACPUInterrupt);
    } else if (buscarProceso(pcbsBloqueados,pid)){
        eliminarProceso(pcbsBloqueados,pid);
    } else log_info(logger, "No se encontró el proceso <%d> en ninguna lista", pid);  
}

//DETENER_PLANIFICACION
void detenerPlanificacion(){
    pthread_mutex_lock(&pausaMutex);
    pausaPlanificacion = true;
    pthread_mutex_unlock(&pausaMutex);
    //log_info(logger, "Planificacion detenida");
}

//INICIAR_PLANIFICACION
void iniciarPlanificacion(){
    pthread_mutex_lock(&pausaMutex);
    pausaPlanificacion = false;
    pthread_cond_signal(&pausaCond);
    pthread_mutex_unlock(&pausaMutex);
    //log_info(logger, "Planificaciones iniciadas");
}

//PROCESO_ESTADO
void procesoEstado(){
    log_info(logger, "Procesos en NEW");
    imprimirListaPCBs(pcbsNEW);
    log_info(logger, "Procesos en READY");
    imprimirListaPCBs(pcbsREADY);
    log_info(logger, "Procesos en EXEC");
    imprimirListaPCBs(pcbsExec);
    log_info(logger, "Procesos en BLOCKED:");
    imprimirListaPCBs(pcbsBloqueados);
    log_info(logger, "Procesos en EXIT");
    imprimirListaExit(pcbsParaExit);
}

//MULTIPROGRAMACION [VALOR]
void ajustarTamanioSemaforo(sem_t* semaforo,int valor){
    int valorActual;
    sem_getvalue(semaforo,&valorActual);

    //Si el valor actual es mayor al valor que le quiero setear, decremento el semaforo hasta llegar al valor
    if(valorActual>valor){ 
        for(int i=0;i<valorActual-valor;i++){
            sem_wait(semaforo);
        }
    } else if(valorActual<valor){ //Si el valor actual es menor al valor que le quiero setear, incremento el semaforo hasta llegar al valor
        for(int i=0;i<valor-valorActual;i++){
            sem_post(semaforo);
        }
    }
}

void modificarGradoMultiprogramacion(int valor){
    if(valor<=0){
        log_info(logger, "Multiprogramacion no permitida. Ingrese un valor entero mayor o igual a 1.");
        return;
    } else{
        gradoMultiprogramacion = valor;
        ajustarTamanioSemaforo(&semGradoMultiprogramacion,valor);
        //log_info(logger, "Grado de multiprogramacion modificado a %d", valor);
        int valorSemaforo;
        sem_getvalue(&semGradoMultiprogramacion, &valorSemaforo);
        //log_info(logger, "Valor del semaforo: %d", valorSemaforo);
    }
}