/*
delay.c

Esse arquivo contém as funções essenciais para a implementação do efeito delay
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "delay.h"

#define MAX_DELAY_MS 1000  // Delay máximo em milissegundos
#define SAMPLE_RATE 44100  // Taxa de amostragem (assumindo 44.1kHz)
#define MAX_DELAY_SAMPLES (SAMPLE_RATE * MAX_DELAY_MS / 1000)  // Delay máximo em número de amostras

// Defina delay_buffer e delay_buffer_pos globalmente
float *delay_buffer = NULL;  // Variável global
int delay_buffer_pos = 0;    // Variável global

void aplicar_delay(float *buffer, int buffer_size, float wetness, float feedback) {
    int delay_ms = 500;  // Definindo o delay em milissegundos (exemplo de 500ms)
    int delay_samples = (SAMPLE_RATE * delay_ms) / 1000;  // Convertendo delay de ms para amostras
    if (delay_samples > MAX_DELAY_SAMPLES) {
        delay_samples = MAX_DELAY_SAMPLES;  // Limitar o delay ao máximo permitido
    }

    // Alocar o buffer de delay se ainda não estiver alocado
    if (delay_buffer == NULL) {
        delay_buffer = (float *)calloc(MAX_DELAY_SAMPLES, sizeof(float));  // Zerar o buffer inicialmente
        if (delay_buffer == NULL) {
            printf("Erro ao alocar memória para o buffer de delay.\n");
            return;
        }
    }

    for (int i = 0; i < buffer_size; i++) {
        // Obter o valor atrasado do buffer
        float delayed_sample = delay_buffer[delay_buffer_pos];  

        // Aplicar o delay ao sinal atual e aplicar o wetness (mistura do sinal atrasado)
        float output_sample = (1.0f - wetness) * buffer[i] + wetness * delayed_sample;

        // Aplicar feedback (realimentação) ao sinal atrasado
        float feedback_sample = delayed_sample * feedback;

        // Atualizar o buffer de delay com a amostra atual somada ao feedback
        delay_buffer[delay_buffer_pos] = buffer[i] + feedback_sample;

        // Avançar a posição do buffer de delay
        delay_buffer_pos = (delay_buffer_pos + 1) % delay_samples;

        // Escrever a amostra processada no buffer de saída
        buffer[i] = output_sample;
    }
}

void liberar_delay() {
    // Liberar o buffer de delay global
    if (delay_buffer != NULL) {
        free(delay_buffer);
        delay_buffer = NULL;  // Resetando o ponteiro para evitar uso posterior inválido
    }
}
