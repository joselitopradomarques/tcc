#ifndef PROCESSAMENTO_SINAIS_H
#define PROCESSAMENTO_SINAIS_H

#include <stdio.h>

// Definições e constantes
#define SAMPLE_RATE 44100
#define ORDEM 64  // Ordem do filtro FIR, você pode ajustar conforme necessário

// Funções de processamento de sinais
void somar_sinais(short *sinal1, short *sinal2, short *resultado, int tamanho);
void executar_arquivo_wav(const char* arquivo);
void apply_effect_to_audio(int effect_choice, float effect_amount);
void executar_filtragem_e_soma(float frequencia_corte);
void aplicar_filtragem_e_efeito(float frequencia_corte, int fx_select, float effectAmount);

// Funções auxiliares de leitura e escrita de arquivos WAV
int ler_wav_estereo(const char *caminho_arquivo, short **sinal, int *tamanho);
int escrever_wav_estereo(const char *caminho_arquivo, short *sinal, int tamanho);

// Funções de filtragem
void gerar_filtro_FIR(float *coeficientes, int ordem, float frequencia_corte, int taxa_amostragem);
void aplicar_filtro_FIR(short *entrada, short *saida, int tamanho, float *coeficientes, int ordem);

// Funções de efeitos
void apply_delay_to_audio(const char *input_file, const char *output_file, float effect_amount);
void applyReverbEffect(const char *input_file, const char *output_file, float effect_amount);
float calcular_nova_frequencia_corte(float frequencia_corte);

#endif // PROCESSAMENTO_SINAIS_H
