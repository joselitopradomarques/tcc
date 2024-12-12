/*
delay.c

Esse arquivo contém as funções essenciais para a implementação do efeito delay
*/

#include <stdio.h>
#include <stdlib.h>
#include "delay.h"

// Inicializa o buffer de delay
void init_delay_buffer(DelayBuffer *db, int delay_ms) {
    if (delay_ms == 0) {
        // Se o delay for 0, o buffer não é necessário
        db->buffer_size = 0;
        db->buffer = NULL;
        db->write_index = 0;
        return;
    }

    // Inicialização normal para delay > 0
    db->buffer_size = (SAMPLE_RATE * delay_ms) / 1000;
    db->buffer = (float *)calloc(db->buffer_size, sizeof(float));
    if (db->buffer == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o buffer de delay.\n");
        exit(1);
    }
    db->write_index = 0;
}

// Função para prevenir o clipping, limitando os valores de áudio entre -1.0 e 1.0
float prevent_clipping(float sample) {
    if (sample > 1.0f) {
        return 1.0f;
    } else if (sample < -1.0f) {
        return -1.0f;
    }
    return sample;
}

// Função para processar o delay com atenuação
float process_delay(DelayBuffer *db, float input, int delay_ms, float attenuation) {
    if (delay_ms == 0 || db->buffer == NULL) {
        // Se o delay for 0, retorne a amostra original sem processar
        return input;
    }

    // Calcula o número de amostras do delay
    int delay_samples = (SAMPLE_RATE * delay_ms) / 1000;
    int read_index = (db->write_index - delay_samples + db->buffer_size) % db->buffer_size;
    float delayed_sample = db->buffer[read_index];

    // Aplica a atenuação no sinal de delay
    delayed_sample *= attenuation;

    // Escreve a nova amostra no buffer
    db->buffer[db->write_index] = input;
    db->write_index = (db->write_index + 1) % db->buffer_size;

    // Retorna o sinal de delay somado com o original, aplicado a prevenção de clipping
    return prevent_clipping(input + delayed_sample);
}



void apply_delay_to_audio(const char *input_filename, const char *output_filename, float delay_factor) {
    // Verifica se o fator de delay está dentro do intervalo [0, 1]
    if (delay_factor < 0.0f || delay_factor > 1.0f) {
        printf("Erro: O fator de delay deve estar entre 0 e 1.\n");
        exit(1);
    }

    // Converte o fator de delay (0 a 1) para o valor em milissegundos (0 a 1000 ms)
    int delay_time = (int)(delay_factor * 1000);

    // Definir a atenuação fixa para suavizar o efeito de delay
    float attenuation = 0.6;  // 30% da intensidade do delay

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

    // Inicializa os buffers de delay para os canais esquerdo e direito
    DelayBuffer db_left, db_right;
    init_delay_buffer(&db_left, delay_time);
    init_delay_buffer(&db_right, delay_time);

    // Abre o arquivo de saída para escrita
    SNDFILE *outfile = sf_open(output_filename, SFM_WRITE, &sf_info);
    if (!outfile) {
        printf("Erro ao abrir o arquivo de saída: %s\n", output_filename);
        sf_close(infile);
        exit(1);
    }

    // Processa o áudio e aplica o delay
    float buffer[2 * SAMPLE_RATE];  // Buffer para os dois canais (estéreo)
    sf_count_t read_count;
    while ((read_count = sf_readf_float(infile, buffer, SAMPLE_RATE)) > 0) {
        for (sf_count_t i = 0; i < read_count; i++) {
            // Aplica o efeito de delay com atenuação no canal esquerdo
            buffer[2 * i] = process_delay(&db_left, buffer[2 * i], delay_time, attenuation);  // Canal esquerdo

            // Aplica o efeito de delay com atenuação no canal direito
            buffer[2 * i + 1] = process_delay(&db_right, buffer[2 * i + 1], delay_time, attenuation);  // Canal direito
        }

        // Grava os dados processados no arquivo de saída
        sf_writef_float(outfile, buffer, read_count);
    }

    // Fecha os arquivos e libera a memória
    sf_close(infile);
    sf_close(outfile);
    if (db_left.buffer != NULL) free(db_left.buffer);
    if (db_right.buffer != NULL) free(db_right.buffer);
}
