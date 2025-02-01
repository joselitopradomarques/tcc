// Definição de funções e variáveis auxiliares para o processamento de sinais

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "reverb.h"

#define PI 3.14159265358979323846
#define MAX_16BIT 32767
#define MIN_16BIT -32768


int escrever_wav_estereo(const char *filename, short *sinal, int tamanho);
// Função para ler arquivo WAV estéreo
int ler_wav_estereo(const char *filename, short **sinal, int *tamanho);

// Função para ler dois arquivos WAV estéreo
int ler_dois_wav_estereo(short **sinal1, short **sinal2, int *tamanho1, int *tamanho2);

// Função para gerar buffers circulares a partir dos sinais de áudio
int gerar_buffers_circulares(short *sinal1, short *sinal2, int tamanho, int buffer_size, short ***buffers_sinal1, short ***buffers_sinal2, int *num_buffers);

// Função exemplo de filtro simples
void filtro_exemplo(short *buffer, int buffer_size);

// Função para processar os buffers com filtro
int processar_buffers_circulares(short ***buffers_sinal1, short ***buffers_sinal2, int num_buffers, int buffer_size, float *coeficientes_filtro, int ordem_filtro, float wetness);


// Função para liberar a memória dos buffers
void liberar_buffers(short **buffers_sinal1, short **buffers_sinal2, int num_buffers);

// Função para gerar coeficientes de filtro FIR
void gerar_filtro_FIR(float *coeficientes, int ordem, float corte, float taxa_amostragem);

// Função para aplicar o filtro FIR em buffers individuais
void aplicar_filtro_FIR_buffer(short *buffer_sinal, short *buffer_sinal_filtrado, int buffer_size, float *coeficientes, int ordem);
