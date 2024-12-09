/*
delay.c

Esse arquivo contém as funções essenciais para a implementação do efeito delay
*/

#include <stdio.h>
#include <stdlib.h>
#include "delay.h"

// Inicializa o buffer de delay
void init_delay_buffer(DelayBuffer *db, int delay_ms) {
    db->buffer_size = (SAMPLE_RATE * delay_ms) / 1000;
    db->buffer = (float *)calloc(db->buffer_size, sizeof(float));
    if (db->buffer == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o buffer de delay.\n");
        exit(1);
    }
    db->write_index = 0;
}

// Função para aplicar o delay
float process_delay(DelayBuffer *db, float input, int delay_ms) {
    int delay_samples = (SAMPLE_RATE * delay_ms) / 1000;
    int read_index = (db->write_index - delay_samples + db->buffer_size) % db->buffer_size;
    float delayed_sample = db->buffer[read_index];

    // Escreve a nova amostra no buffer
    db->buffer[db->write_index] = input;
    db->write_index = (db->write_index + 1) % db->buffer_size;

    return delayed_sample;
}

// Função para processar o áudio com delay isolado
void apply_delay_to_audio(const char *input_filename, const char *output_filename, int delay_time) {
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
            // Substitui o sinal original pelo sinal com delay no canal esquerdo
            buffer[2 * i] = process_delay(&db_left, buffer[2 * i], delay_time);       // Canal esquerdo

            // Substitui o sinal original pelo sinal com delay no canal direito
            buffer[2 * i + 1] = process_delay(&db_right, buffer[2 * i + 1], delay_time);  // Canal direito
        }

        // Grava os dados processados no arquivo de saída (somente o sinal com delay)
        sf_writef_float(outfile, buffer, read_count);
    }

    // Fecha os arquivos e libera a memória
    sf_close(infile);
    sf_close(outfile);
    free(db_left.buffer);
    free(db_right.buffer);
}

