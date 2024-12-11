#ifndef REVERB_H
#define REVERB_H

#include <stdlib.h>
#include <math.h>  // Agora a biblioteca math.h é necessária para funções como exp() e M_PI

#define SAMPLE_RATE 44100 // Taxa de amostragem do áudio

// Estrutura para um filtro Allpass
typedef struct {
    float *buffer;        // Buffer de atraso
    int delay_samples;    // Número de amostras de atraso
    int index;            // Índice para o buffer circular
    float gain;           // Ganho do filtro
} AllpassFilter;

// Estrutura para o efeito de Reverb
typedef struct {
    AllpassFilter *filters;  // Array de filtros Allpass
    int num_filters;         // Número de filtros no efeito de reverb
    float send_level;        // Nível de send para o efeito de reverb (0 a 1)
    float low_pass_cutoff;   // Frequência de corte do filtro low-pass
    float *last_output_left; // Buffer para o último output do canal esquerdo
    float *last_output_right; // Buffer para o último output do canal direito
} ReverbEffect;

// Funções para o efeito de reverb

// Inicializa um filtro allpass
void init_allpass_filter(AllpassFilter *filter, int delay_samples, float gain);

// Processa um sample com o filtro allpass
float process_allpass(AllpassFilter *filter, float input);

// Converte um valor em dB para o ganho de feedback
float dB_to_feedback_gain(float attenuation_dB);

// Configura o efeito de reverb com um parâmetro de controle de tempo, send e filtro low-pass
void configure_reverb_with_send(ReverbEffect *reverb, float param, float low_pass_cutoff);

// Aplica o filtro low-pass ao sinal de entrada
float apply_low_pass_filter(float input, float *last_output, float cutoff);

// Aplica o efeito de reverb ao áudio de um sinal de entrada
float apply_send_to_reverb(ReverbEffect *reverb, float input);

// Aplica o reverb ao áudio de um arquivo de entrada e exporta o sinal processado
void apply_reverb_to_audio(const char *input_filename, const char *output_filename, float effect_param, float low_pass_cutoff);

#endif // REVERB_H
