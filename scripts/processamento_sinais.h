// processamento_sinais.h

#ifndef PROCESSAMENTO_SINAIS_H
#define PROCESSAMENTO_SINAIS_H

#include "filter_final.h" // Inclui o cabeçalho que define a constante ORDEM

// Definições e protótipos das funções

// Função que soma dois sinais de entrada
void somar_sinais(short *sinal1, short *sinal2, short *resultado, int tamanho);

// Função para aplicar os efeitos (delay ou reverb)
void apply_effect_to_audio(int effect_choice, float effect_amount);

// Função para execução final
void aplicar_filtragem_e_efeito(float frequencia_corte, int fx_select, float effectAmount);

// Função para executar a filtragem dos dois sinais e somá-los
void executar_filtragem_e_soma(float frequencia_corte);

// Arquivos de entrada e saída definidos aqui (podem ser modificados conforme necessidade)
#define INPUT_FILE_1 "/home/joselito/git/tcc/datas/audio01.wav"
#define INPUT_FILE_2 "/home/joselito/git/tcc/datas/audio02.wav"
#define OUTPUT_FILE_DELAY "/home/joselito/git/tcc/scripts/saida_delay.wav"
#define OUTPUT_FILE_REVERB "/home/joselito/git/tcc/scripts/saida_reverb.wav"
#define OUTPUT_FILE_FILTRADA_1 "/home/joselito/git/tcc/scripts/saida_filtrada_1.wav"
#define OUTPUT_FILE_FILTRADA_2 "/home/joselito/git/tcc/scripts/saida_filtrada_2.wav"
#define OUTPUT_FILE_FILTRADA_SOMA "/home/joselito/git/tcc/scripts/saida_filtrada_soma.wav"

#endif // PROCESSAMENTO_SINAIS_H
