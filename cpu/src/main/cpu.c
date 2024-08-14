#include <main/cpu.h>
int flag_bloqueante;

int main(void){
	
	logger = iniciarLogger("cpu.log", "CPU");
	loggerError = iniciarLogger("errores.log", "Errores CPU");

	config = iniciarConfiguracion("cpu.config");

	atexit(terminarPrograma);

	conexionMemoria(); 

	char * nombre = string_duplicate("CPU-KERNEL");

    char *algoritmoTLB = obtenerAlgoritmoTLB();
    int cantidadEntradasTLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    if (cantidadEntradasTLB > 0) inicializar_tlb(algoritmoTLB); 

    escucharAlKernel();

	free (nombre);

	return EXIT_SUCCESS;
}