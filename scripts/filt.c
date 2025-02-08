#include "filt.h"
#include <stdio.h>
#include <math.h>

// Função para gerar pontos logarítmicos
void gerar_pontos_logaritmicos(double frequencias_log[N_FREQUENCIES]) {
    float fc1_min = 20.0;
    float fc1_max = 24000.0;

    for (int i = 0; i < N_FREQUENCIES; i++) {
        frequencias_log[i] = fc1_min * pow(10.0, (log10(fc1_max / fc1_min)) * i / (N_FREQUENCIES - 1));
    }
}

// Função para gerar os coeficientes de Hamming para um filtro passa-altas
void generate_hamming_highpass_filter(int filter_order, double cutoff_frequency, double sample_rate, double h[filter_order]) {
    int M = (filter_order - 1) / 2;  // Centro da janela (M)
    double nyquist = sample_rate / 2.0;
    double normalized_cutoff = cutoff_frequency / nyquist;  // Normalizar a frequência de corte

    // Gerar a resposta ideal do filtro passa-altas
    for (int n = 0; n < filter_order; n++) {
        if (n == M) {
            h[n] = 2 * normalized_cutoff;  // Resposta para o centro (n = M)
        } else {
            h[n] = sin(M_PI * (n - M) * normalized_cutoff) / (M_PI * (n - M));  // Resposta ideal
        }
    }

    // Aplicar a janela de Hamming
    for (int n = 0; n < filter_order; n++) {
        double window = 0.54 - 0.46 * cos(2 * M_PI * n / (filter_order - 1));  // Janela de Hamming
        h[n] *= window;
    }
}

// Função para gerar a matriz de coeficientes para diferentes frequências
void gerar_matriz_coeficientes(double matriz_coeficientes[N_FREQUENCIES][ORDER], double frequencias_log[N_FREQUENCIES]) {
    for (int i = 0; i < N_FREQUENCIES; i++) {
        generate_hamming_highpass_filter(ORDER, frequencias_log[i], SAMPLE_RATE, matriz_coeficientes[i]);
    }
}
