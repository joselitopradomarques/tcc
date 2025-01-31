#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "capture.h"

// Definindo o tempo de captura (exemplo)
#define CAPTURE_TIME 10

// Função que simula a captura de áudio
void *capture_audio(void *arg) {
    printf("Iniciando captura de áudio...\n");
    
    // Aqui você pode colocar o código de captura real do áudio. Por enquanto, vamos simular com uma espera.
    sleep(CAPTURE_TIME);  // Simulando o tempo de captura
    
    printf("Captura de áudio finalizada.\n");
    
    return NULL;
}
