#ifndef FILT_H
#define FILT_H

#define N_FREQUENCIES 256  // Quantidade de frequências discretas
#define ORDER 121  // Ordem do filtro

// Declaração das funções
void gerar_pontos_logaritmicos(float frequencias_log[N_FREQUENCIES]);
void generate_hamming_highpass_filter(int filter_order, float cutoff_frequency, float sample_rate, float h[filter_order]);
void gerar_matriz_coeficientes(float matriz_coeficientes[N_FREQUENCIES][ORDER], float frequencias_log[N_FREQUENCIES]);

#endif
