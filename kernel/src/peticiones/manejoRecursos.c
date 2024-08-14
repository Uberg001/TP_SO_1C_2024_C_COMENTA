#include <peticiones/manejoRecursos.h>
#include <peticiones/syscalls.h>


int cantidadRecursos;

void crearColasBloqueo()
{
	instanciasRecursos = NULL; 
	recursos = list_create();
    nombresRecursos = obtenerRecursos(); //[RA,RB,RC...]
    char **cantInstanciasRecursos = obtenerInstanciasRecursos();//[1,2,1...]
    int tamanio = tamanioArrayCharDoble(cantInstanciasRecursos);

    cantidadRecursos=tamanio;
    for (int i = 0; i < tamanio; i++)
    {
        int instanciasConvertEntero = atoi(cantInstanciasRecursos[i]);
        instanciasRecursos=realloc(instanciasRecursos,(i+1)*sizeof(int));
        instanciasRecursos[i]=instanciasConvertEntero;

        t_list *colaBloqueo = list_create();
        list_add(recursos, colaBloqueo);
    }

    destruirArrayCharDoble(cantInstanciasRecursos);
}


int indiceRecurso (char * recurso){
    int tamanio = tamanioArrayCharDoble(nombresRecursos);

    for (int i = 0; i < tamanio; i++)
        if (!strcmp(recurso, nombresRecursos[i]))
            return i;
    return -1;
}

void liberarColasBloqueo(){
	destruirInstanciasRecursos();
	destruirArrayCharDoble(nombresRecursos);
	destruirRecursos();
}


void destruirInstanciasRecursos(){
	free(instanciasRecursos);
}


void destruirArrayCharDoble (char ** array){
	int tamanio=tamanioArrayCharDoble(array);
	for (int i = 0; i<tamanio; i++) free(array[i]);
	free(array);
}


void colaBloqueadosDestroyer(void* colaBloqueados){
	list_destroy_and_destroy_elements(colaBloqueados,(void*)destruirPCB);
}

void destruirRecursos(){
	list_destroy_and_destroy_elements(recursos, colaBloqueadosDestroyer);
}

int tamanioArrayCharDoble (char **arreglo){  
	    return string_array_size(arreglo);
}

void liberarRecursosAsignados(t_pcb* proceso){
    int cantRecursos = list_size(proceso->recursosAsignados);;
    int i;
    if(cantRecursos!=0){
        for(i=0; i<cantRecursos;cantRecursos--){
            debug ("%d", i);
            char * parametros[3] = {(char *)list_get(proceso->recursosAsignados, i), "", "EXIT"}; 
            signal_s(proceso, parametros);
        }
    }

    for(i=0;i<list_size(recursos);i++){
        t_list* colaBloquadosRecurso=list_get(recursos,i);
        for(int j=0;j<list_size(colaBloquadosRecurso);j++){
            t_pcb* pcbBloqueado=list_get(colaBloquadosRecurso,j);
            if(pcbBloqueado->pid==proceso->pid){
                instanciasRecursos[i]++;
                list_remove(colaBloquadosRecurso,j);
            }
        }
    }
}

void eliminarRecursoLista(t_list* recursos, char* recurso){
    int cantRecursos = list_size(recursos);
    int i;
    
    for(i=0;i<cantRecursos;i++){ 
        if(!strcmp((char*)list_get(recursos,i), recurso)){
            list_remove(recursos,i);
            return;  
        }
    }
}