#include "filt.h"
#include <stdio.h>
#include <math.h>
#include "proc.h"

// Função para gerar pontos logarítmicos
void gerar_pontos_logaritmicos(float frequencias_log[N_FREQUENCIES]) {
    float fc1_min = 20.0f;  // Especifica como float
    float fc1_max = 22050.0f;  // Especifica como float

    printf("Iniciando a geração de pontos logarítmicos...\n");

    for (int i = 0; i < N_FREQUENCIES; i++) {
        frequencias_log[i] = fc1_min * powf(10.0f, (log10f(fc1_max / fc1_min)) * i / (N_FREQUENCIES - 1));
    }

    printf("Geração de pontos logarítmicos concluída!\n");
}

// Função para gerar os coeficientes de Hamming para um filtro passa-altas
void generate_hamming_highpass_filter(int filter_order, float cutoff_frequency, float sample_rate, float h[filter_order]) {
    int M = (filter_order - 1) / 2;  // Centro da janela (M)
    float nyquist = sample_rate / 2.0f;
    float normalized_cutoff = cutoff_frequency / nyquist;  // Normalizar a frequência de corte

    // Gerar a resposta ideal do filtro passa-altas
    for (int n = 0; n < filter_order; n++) {
        if (n == M) {
            h[n] = 1 - normalized_cutoff;  // Resposta para o centro (n = M) passa-altas
        } else {
            h[n] = -sinf(PI * (n - M) * normalized_cutoff) / (PI * (n - M));  // Inverte a resposta ideal
        }
    }

    // Aplicar a janela de Hamming
    for (int n = 0; n < filter_order; n++) {
        float window = 0.54f - 0.46f * cosf(2 * PI * n / (filter_order - 1));  // Janela de Hamming
        h[n] *= window;
    }
}

// Função para gerar a matriz de coeficientes para diferentes frequências
void gerar_matriz_coeficientes(float matriz_coeficientes[N_FREQUENCIES][ORDER], float frequencias_log[N_FREQUENCIES]) {
    for (int i = 0; i < N_FREQUENCIES; i++) {
        // Certifique-se de que generate_hamming_highpass_filter aceite float
        generate_hamming_highpass_filter(ORDER, frequencias_log[i], SAMPLE_RATE, matriz_coeficientes[i]);
    }
    
    // Mensagem de sucesso após a geração da matriz de coeficientes
    printf("Matriz de coeficientes gerada com sucesso para %d frequências.\n", N_FREQUENCIES);
}

