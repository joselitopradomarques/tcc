#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "filter_final.h"  // Inclui o cabeçalho com as declarações das funções

// Função para aplicar o filtro FIR no sinal mono
void aplicar_filtro_FIR(short *sinal, short *sinal_filtrado, int tamanho, float *coeficientes, int ordem) {
    for (int i = 0; i < tamanho; i++) {
        float acumulador = 0.0;
        for (int j = 0; j < ordem; j++) {
            if (i - j >= 0) {
                acumulador += coeficientes[j] * sinal[i - j];
            }
        }
        // Limitar a amplitude para valores de 16 bits
        if (acumulador > MAX_16BIT) acumulador = MAX_16BIT;
        if (acumulador < -MAX_16BIT) acumulador = -MAX_16BIT;
        sinal_filtrado[i] = (short)acumulador;
    }
}

// Função para gerar coeficientes do filtro FIR
void gerar_filtro_FIR(float *coeficientes, int ordem, float corte, float taxa_amostragem) {
    int n = ordem;
    float wc = 2 * PI * corte / taxa_amostragem;  // Frequência de corte normalizada
    for (int i = 0; i < n; i++) {
        if (i == (n - 1) / 2) {
            coeficientes[i] = 1 - (wc / PI); 
        } else {
            coeficientes[i] = -sin(wc * (i - (n - 1) / 2)) / (PI * (i - (n - 1) / 2));  // Inverte a fase
        }
    }
}

// Função para ler o arquivo WAV estéreo
int ler_wav_estereo(const char *filename, short **sinal, int *tamanho) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV: %s\n", filename);
        return -1;
    }

    // Ler cabeçalho WAV (34 primeiros bytes de interesse)
    fseek(file, 22, SEEK_SET);  // Pular até o número de canais
    short num_channels;
    fread(&num_channels, sizeof(short), 1, file);

    fseek(file, 24, SEEK_SET);  // Pular até a taxa de amostragem
    int sample_rate;
    fread(&sample_rate, sizeof(int), 1, file);

    fseek(file, 34, SEEK_SET);  // Pular até os bits por amostra
    short bits_per_sample;
    fread(&bits_per_sample, sizeof(short), 1, file);

    if (num_channels != 2 || bits_per_sample != 16) {
        printf("Erro: o arquivo WAV não é estéreo ou não possui 16 bits por amostra.\n");
        fclose(file);
        return -1;
    }

    // Saltar para a parte de dados do arquivo WAV (pular o cabeçalho até a posição 44)
    fseek(file, 44, SEEK_SET);

    // Descobrir o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 44, SEEK_SET);  // Volta para a posição onde os dados de áudio começam

    // Calcular o número de amostras considerando que é estéreo (2 canais)
    int num_samples = (file_size - 44) / (sizeof(short) * num_channels);  // 2 canais
    *tamanho = num_samples;

    // Alocar memória para o sinal estéreo (2 canais)
    *sinal = (short *)malloc(num_samples * num_channels * sizeof(short));  
    if (!*sinal) {
        printf("Erro ao alocar memória para o sinal estéreo\n");
        fclose(file);
        return -1;
    }

    // Ler os dados do sinal estéreo (2 canais)
    fread(*sinal, sizeof(short), num_samples * num_channels, file);
    fclose(file);

    return 0;
}

// Função para salvar o arquivo WAV com o sinal filtrado em estéreo
int escrever_wav_estereo(const char *filename, short *sinal, int tamanho) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para escrita: %s\n", filename);
        return -1;
    }

    // Escrever cabeçalho WAV simples (44 bytes)
    unsigned int chunk_size = 36 + tamanho * 2 * sizeof(short);  // 2 canais
    unsigned short audio_format = 1; // PCM
    unsigned short num_channels = 2; // Estéreo
    unsigned int sample_rate = SAMPLE_RATE;
    unsigned int byte_rate = SAMPLE_RATE * 2 * sizeof(short);  // 2 canais
    unsigned short block_align = 2 * sizeof(short);  // 2 canais
    unsigned short bits_per_sample = 16;

    // Cabeçalho RIFF
    fwrite("RIFF", sizeof(char), 4, file);
    fwrite(&chunk_size, sizeof(unsigned int), 1, file);
    fwrite("WAVE", sizeof(char), 4, file);

    // Sub-chunk 1 "fmt "
    fwrite("fmt ", sizeof(char), 4, file);
    unsigned int sub_chunk1_size = 16;
    fwrite(&sub_chunk1_size, sizeof(unsigned int), 1, file);
    fwrite(&audio_format, sizeof(unsigned short), 1, file);
    fwrite(&num_channels, sizeof(unsigned short), 1, file);
    fwrite(&sample_rate, sizeof(unsigned int), 1, file);
    fwrite(&byte_rate, sizeof(unsigned int), 1, file);
    fwrite(&block_align, sizeof(unsigned short), 1, file);
    fwrite(&bits_per_sample, sizeof(unsigned short), 1, file);

    // Sub-chunk 2 "data"
    unsigned int sub_chunk2_size = tamanho * 2 * sizeof(short);  // 2 canais
    fwrite("data", sizeof(char), 4, file);
    fwrite(&sub_chunk2_size, sizeof(unsigned int), 1, file);
    // Escrever o sinal filtrado em estéreo
    fwrite(sinal, sizeof(short), tamanho * 2, file);

    fclose(file);
    return 0;
}
