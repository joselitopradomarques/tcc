#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // Necessário para usar a função sin() e PI
#include "proc.h"

#define PI 3.14159265358979323846  // Definindo PI
// Função para aplicar o filtro FIR em cada buffer circular
void aplicar_filtro_FIR_buffer(short *buffer_sinal, short *buffer_sinal_filtrado, int buffer_size, float *coeficientes, int ordem) {
    // Aplicar o filtro FIR em um único buffer
    for (int j = 0; j < buffer_size; j++) {
        float acumulador = 0.0;
        // Aplicar FIR para cada amostra no buffer
        for (int k = 0; k < ordem; k++) {
            if (j - k >= 0) {
                acumulador += coeficientes[k] * buffer_sinal[j - k];
            }
        }
        // Limitar a amplitude para valores de 16 bits
        if (acumulador > MAX_16BIT) acumulador = MAX_16BIT;
        if (acumulador < -MAX_16BIT) acumulador = -MAX_16BIT;
        buffer_sinal_filtrado[j] = (short)acumulador;
    }
}

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

int ler_dois_wav_estereo(short **sinal1, short **sinal2, int *tamanho1, int *tamanho2) {
    // Definindo os caminhos dos arquivos WAV diretamente dentro da função
    const char *filename1 = "/home/joselito/git/tcc/datas/audio01.wav";
    const char *filename2 = "/home/joselito/git/tcc/datas/audio02.wav";

    // Ler o primeiro arquivo WAV
    short *sinal_temp1 = NULL;
    if (ler_wav_estereo(filename1, &sinal_temp1, tamanho1) != 0) {
        return -1; // Erro ao ler o primeiro arquivo
    }

    // Ler o segundo arquivo WAV
    short *sinal_temp2 = NULL;
    if (ler_wav_estereo(filename2, &sinal_temp2, tamanho2) != 0) {
        // Se ocorrer erro ao ler o segundo arquivo, liberar a memória do primeiro arquivo
        free(sinal_temp1);
        return -1; // Erro ao ler o segundo arquivo
    }

    // Verificar se as propriedades dos dois arquivos são compatíveis
    if (*tamanho1 != *tamanho2) {
        // Ajuste os sinais para o tamanho do menor
        int min_tamanho = (*tamanho1 < *tamanho2) ? *tamanho1 : *tamanho2;
        *tamanho1 = min_tamanho;
        *tamanho2 = min_tamanho;

        // Redimensionar os sinais para o tamanho compatível
        sinal_temp1 = (short *)realloc(sinal_temp1, min_tamanho * 2 * sizeof(short));  // 2 canais
        sinal_temp2 = (short *)realloc(sinal_temp2, min_tamanho * 2 * sizeof(short));  // 2 canais
    }

    // Se tudo estiver correto, copiar os dados de áudio para os arrays finais
    *sinal1 = sinal_temp1;
    *sinal2 = sinal_temp2;

    return 0; // Sucesso na leitura dos dois arquivos
}

// Função para gerar buffers a partir dos sinais de áudio sinal1 e sinal2
// Os sinais são divididos em buffers de tamanho definido pelo usuário
int gerar_buffers_circulares(short *sinal1, short *sinal2, int tamanho, int buffer_size, short ***buffers_sinal1, short ***buffers_sinal2, int *num_buffers) {
    *num_buffers = (tamanho + buffer_size - 1) / buffer_size;  // Calculando o número de buffers necessários

    // Alocando memória para os buffers circulares
    *buffers_sinal1 = (short **)malloc(*num_buffers * sizeof(short *));
    *buffers_sinal2 = (short **)malloc(*num_buffers * sizeof(short *));
    if (*buffers_sinal1 == NULL || *buffers_sinal2 == NULL) {
        printf("Erro ao alocar memória para os buffers.\n");
        return -1;  // Erro de alocação
    }

    // Alocar e inicializar os buffers circulares
    for (int i = 0; i < *num_buffers; i++) {
        (*buffers_sinal1)[i] = (short *)malloc(buffer_size * sizeof(short));
        (*buffers_sinal2)[i] = (short *)malloc(buffer_size * sizeof(short));

        if ((*buffers_sinal1)[i] == NULL || (*buffers_sinal2)[i] == NULL) {
            printf("Erro ao alocar memória para os buffers individuais.\n");
            return -1;  // Erro de alocação
        }
    }

    // Preencher os buffers com os sinais (acesso circular)
    for (int i = 0; i < *num_buffers; i++) {
        for (int j = 0; j < buffer_size; j++) {
            int index = (i * buffer_size + j) % tamanho;  // Acesso circular
            (*buffers_sinal1)[i][j] = sinal1[index];
            (*buffers_sinal2)[i][j] = sinal2[index];
        }
    }

    return 0;  // Sucesso
}

// Função filtro_exemplo
void filtro_exemplo(short *buffer, int buffer_size) {
    for (int i = 1; i < buffer_size - 1; i++) {
        buffer[i] = (buffer[i - 1] + buffer[i] + buffer[i + 1]) / 3;
    }
}

// Função para processar os buffers de sinal1 e sinal2
// Aplica o filtro a cada buffer dos dois sinais
int processar_buffers_circulares(short ***buffers_sinal1, short ***buffers_sinal2, int num_buffers, int buffer_size, float *coeficientes_filtro, int ordem_filtro) {
    // Verificar se os buffers estão alocados corretamente
    if (!buffers_sinal1 || !buffers_sinal2) {
        printf("Erro: buffers não alocados corretamente.\n");
        return -1;
    }

    // Alocar buffers filtrados
    short **buffers_sinal1_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    if (!buffers_sinal1_filtrado || !buffers_sinal2_filtrado) {
        printf("Erro: falha ao alocar buffers filtrados.\n");
        return -1;
    }

    // Inicializar buffers filtrados
    for (int i = 0; i < num_buffers; i++) {
        buffers_sinal1_filtrado[i] = (short *)malloc(buffer_size * sizeof(short));
        buffers_sinal2_filtrado[i] = (short *)malloc(buffer_size * sizeof(short));
    }

    // Alocar buffer para a média dos buffers filtrados
    short *buffer_media = (short *)malloc(buffer_size * sizeof(short));
    if (!buffer_media) {
        printf("Erro: falha ao alocar o buffer de média.\n");
        return -1;
    }

    // Processar os buffers e aplicar o filtro FIR
    for (int i = 0; i < num_buffers; i++) {
        // Atualizar coeficientes a cada 10 buffers
        if (i > 0 && i % 10 == 0) {
            // Atualizar os coeficientes, se necessário
            // alterar_coeficientes(coeficientes_filtro, nova_ordem_filtro);
        }

        // Aplicar o filtro FIR para o sinal1
        if ((*buffers_sinal1)[i] != NULL) {
            aplicar_filtro_FIR_buffer((*buffers_sinal1)[i], buffers_sinal1_filtrado[i], buffer_size, coeficientes_filtro, ordem_filtro);
        }

        // Aplicar o filtro FIR para o sinal2
        if ((*buffers_sinal2)[i] != NULL) {
            aplicar_filtro_FIR_buffer((*buffers_sinal2)[i], buffers_sinal2_filtrado[i], buffer_size, coeficientes_filtro, ordem_filtro);
        }

        // Calcular a média dos buffers filtrados
        for (int j = 0; j < buffer_size; j++) {
            buffer_media[j] = (buffers_sinal1_filtrado[i][j] + buffers_sinal2_filtrado[i][j]) / 2;
        }

        // Aqui, o buffer_media pode ser usado para mais processamento ou armazenado conforme necessário

        // Exemplo de uso do buffer_media: o buffer_media pode ser processado mais uma vez ou armazenado
        // processar_buffer_media(buffer_media, buffer_size);
    }

    // Liberação dos buffers filtrados e buffer de média
    for (int i = 0; i < num_buffers; i++) {
        free(buffers_sinal1_filtrado[i]);
        free(buffers_sinal2_filtrado[i]);
    }
    free(buffers_sinal1_filtrado);
    free(buffers_sinal2_filtrado);
    free(buffer_media);

    return 0; // Sucesso no processamento dos buffers
}





void liberar_buffers(short **buffers_sinal1, short **buffers_sinal2, int num_buffers) {
    // Verifica e libera buffers_sinal1
    if (buffers_sinal1 != NULL) {
        for (int i = 0; i < num_buffers; i++) {
            // Liberar cada buffer de sinal1 (que é um ponteiro para short)
            if (buffers_sinal1[i] != NULL) {
                free(buffers_sinal1[i]);
            }
        }
        // Liberar o array de ponteiros buffers_sinal1
        free(buffers_sinal1);
    }

    // Verifica e libera buffers_sinal2
    if (buffers_sinal2 != NULL) {
        for (int i = 0; i < num_buffers; i++) {
            // Liberar cada buffer de sinal2 (que é um ponteiro para short)
            if (buffers_sinal2[i] != NULL) {
                free(buffers_sinal2[i]);
            }
        }
        // Liberar o array de ponteiros buffers_sinal2
        free(buffers_sinal2);
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

