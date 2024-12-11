#include "reverb2.h"
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES  // Define a macro antes de incluir math.h
#include <math.h>
#include <stdio.h>
#include <sndfile.h>
#include "filter_final.h"

// Função para inicializar o filtro allpass
void init_allpass_filter(AllpassFilter *filter, int delay_samples, float gain) {
    filter->buffer = (float *)malloc(delay_samples * sizeof(float));
    memset(filter->buffer, 0, delay_samples * sizeof(float));
    filter->delay_samples = delay_samples;
    filter->index = 0;
    filter->gain = gain;
}

// Função para processar um sample com filtro allpass
float process_allpass(AllpassFilter *filter, float input) {
    float delayed_sample = filter->buffer[filter->index];
    float output = input + filter->gain * delayed_sample;
    filter->buffer[filter->index] = output;

    // Avança o índice no buffer circular
    filter->index = (filter->index + 1) % filter->delay_samples;

    return output;
}

// Função para calcular o ganho de feedback a partir do dB (usando a fórmula de atenuação)
float dB_to_feedback_gain(float attenuation_dB) {
    // Limitar o ganho para evitar feedback excessivo
    attenuation_dB = fmax(-30.0f, fmin(attenuation_dB, 0.0f)); // Limitar entre -30dB e 0dB
    return pow(10.0f, attenuation_dB / 20.0f);
}

// Função para configurar o tempo do reverb e o nível de send com base em um único parâmetro (0 a 1)
void configure_reverb_with_send(ReverbEffect *reverb, float param, float low_pass_cutoff) {
    // Definir o intervalo de tempo do reverb (em segundos)
    float min_reverb_time = 0.1f; // Tempo mínimo de reverb (em segundos)
    float max_reverb_time = 2.0f; // Tempo máximo de reverb (em segundos)

    // Calcular o tempo de reverb proporcionalmente ao parâmetro
    float reverb_time = min_reverb_time + (max_reverb_time - min_reverb_time) * param;

    // Inicializa o reverb com o tempo de reverb calculado
    reverb->num_filters = 5; // Número de filtros allpass
    reverb->filters = (AllpassFilter *)malloc(reverb->num_filters * sizeof(AllpassFilter));

    for (int i = 0; i < reverb->num_filters; i++) {
        int delay_samples = (int)(SAMPLE_RATE * pow(1.05, i)); // Ajustar o incremento do atraso
        init_allpass_filter(&reverb->filters[i], delay_samples, 0.7f);
    }

    // Calcula o ganho de feedback com base no dB de reverb após 1 segundo
    float feedback_gain = dB_to_feedback_gain(reverb_time * 10); // Reduzir a intensidade do feedback

    // Ajusta o ganho de feedback no último filtro (para controle de reverb)
    reverb->filters[reverb->num_filters - 1].gain = feedback_gain;

    // Nível de send, onde 0 é sem send e 1 é o send máximo
    float send_level = param;
    reverb->send_level = send_level;
}

// Função para aplicar um filtro low-pass simples
float apply_low_pass_filter(float input, float *last_output, float cutoff) {
    float alpha = exp(-2.0f * PI * cutoff / SAMPLE_RATE);  // Cálculo do fator de suavização
    float output = alpha * (*last_output) + (1.0f - alpha) * input;
    *last_output = output;
    return output;
}

// Função para aplicar o send ao reverb
float apply_send_to_reverb(ReverbEffect *reverb, float input) {
    // Calcula o sinal enviado para o reverb com base no nível de send
    float send_signal = input * reverb->send_level;

    // Aplica o reverb ao sinal enviado
    float reverb_output = send_signal; // Aqui, o sinal é enviado diretamente para o reverb
    for (int i = 0; i < reverb->num_filters; i++) {
        reverb_output = process_allpass(&reverb->filters[i], reverb_output); // Aplica o reverb através dos filtros allpass
    }

    // Retorna o sinal com o reverb aplicado
    return reverb_output;
}

// Função para aplicar o reverb ao áudio de um arquivo de entrada e exportar apenas o sinal de reverb para um arquivo de saída
void apply_reverb_to_audio(const char *input_filename, const char *output_filename, float effect_param, float low_pass_cutoff) {
    // Abre o arquivo de entrada para leitura
    SF_INFO sf_info;
    SNDFILE *infile = sf_open(input_filename, SFM_READ, &sf_info);
    if (!infile) {
        printf("Erro ao abrir o arquivo de entrada: %s\n", input_filename);
        exit(1);
    }

    // Verifica se o formato do arquivo de áudio é estéreo
    if (sf_info.channels != 2) {
        printf("Somente áudio estéreo é suportado neste exemplo.\n");
        sf_close(infile);
        exit(1);
    }

    // Inicializa o reverb com o parâmetro de efeito (controlando tanto o tempo de reverb quanto o send)
    ReverbEffect reverb;
    configure_reverb_with_send(&reverb, effect_param, low_pass_cutoff);

    // Abre o arquivo de saída para escrita (somente o sinal do reverb será exportado)
    SNDFILE *outfile = sf_open(output_filename, SFM_WRITE, &sf_info);
    if (!outfile) {
        printf("Erro ao abrir o arquivo de saída: %s\n", output_filename);
        sf_close(infile);
        exit(1);
    }

    // Processa o áudio e aplica o reverb
    float buffer[2 * SAMPLE_RATE];  // Buffer para os dois canais (estéreo)
    sf_count_t read_count;

    // Variáveis para o filtro low-pass
    float last_output_left = 0.0f;
    float last_output_right = 0.0f;

    while ((read_count = sf_readf_float(infile, buffer, SAMPLE_RATE)) > 0) {
        for (sf_count_t i = 0; i < read_count; i++) {
            // Aplica o reverb
            buffer[2 * i] = apply_send_to_reverb(&reverb, buffer[2 * i]);
            buffer[2 * i + 1] = apply_send_to_reverb(&reverb, buffer[2 * i + 1]);

            // Aplica o filtro low-pass após o reverb
            buffer[2 * i] = apply_low_pass_filter(buffer[2 * i], &last_output_left, low_pass_cutoff);
            buffer[2 * i + 1] = apply_low_pass_filter(buffer[2 * i + 1], &last_output_right, low_pass_cutoff);
        }

        // Grava o sinal processado no arquivo de saída
        sf_writef_float(outfile, buffer, read_count);
    }

    // Fecha os arquivos e libera a memória
    sf_close(infile);
    sf_close(outfile);
    free(reverb.filters);
}

int main() {
    // Parâmetro de controle de efeito (de 0 a 1)
    float effect_param = 1.0f;  // Você pode variar esse valor para testar diferentes intensidades de reverb
    float low_pass_cutoff = 50.0f; // Frequência de corte para o filtro low-pass (5kHz)

    // Chama a função para processar o áudio e aplicar o reverb
    apply_reverb_to_audio("/home/joselito/git/tcc/datas/audio01.wav", "/home/joselito/git/tcc/scripts/saida_soma_reverb2.wav", effect_param, low_pass_cutoff);

    return 0;
}
