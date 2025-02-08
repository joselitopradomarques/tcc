#ifndef FILT_H
#define FILT_H

#define N_FREQUENCIES 256  // Quantidade de frequências discretas
#define ORDER 121  // Ordem do filtro
#define SAMPLE_RATE 44100.0  // Taxa de amostragem
#define M_PI 3.14159265358979323846


// Declaração das funções
void gerar_pontos_logaritmicos(double frequencias_log[N_FREQUENCIES]);
void generate_hamming_highpass_filter(int filter_order, double cutoff_frequency, double sample_rate, double h[filter_order]);
void gerar_matriz_coeficientes(double matriz_coeficientes[N_FREQUENCIES][ORDER], double frequencias_log[N_FREQUENCIES]);

#endif
