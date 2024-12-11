// filter_final.h
#ifndef FILTER_FINAL_H
#define FILTER_FINAL_H

// Definir constantes
#define ORDEM 1000
#define PI 3.14159265358979323846
#define SAMPLE_RATE 44100
#define MAX_16BIT 32767.0

// Declarações das funções implementadas em filter_final.c

// Função para aplicar o filtro FIR no sinal mono
void aplicar_filtro_FIR(short *sinal, short *sinal_filtrado, int tamanho, float *coeficientes, int ordem);

// Função para gerar coeficientes do filtro FIR
void gerar_filtro_FIR(float *coeficientes, int ordem, float corte, float taxa_amostragem);

// Função para ler o arquivo WAV estéreo
int ler_wav_estereo(const char *filename, short **sinal, int *tamanho);

// Função para salvar o arquivo WAV com o sinal filtrado em estéreo
int escrever_wav_estereo(const char *filename, short *sinal, int tamanho);

// Função para calcular a frequencia de corte fc2 para o segundo canal
float calcular_nova_frequencia_corte(float frequencia_corte_atual);

#endif
