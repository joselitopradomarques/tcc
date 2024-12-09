#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ORDEM 1000  // Ordem do filtro FIR
#define PI 3.14159265358979323846
#define SAMPLE_RATE 44100  // Taxa de amostragem padrão (44100 Hz)
#define MAX_16BIT 32767.0  // Valor máximo de 16 bits

// Função para aplicar o filtro FIR no sinal mono
void aplicar_filtro_FIR(short *sinal, short *sinal_filtrado, int tamanho, float *coeficientes, int ordem) {

printf("Gravando %d antes de aplicar os filtros \n", tamanho);  // Para áudio estéreo (duas amostras por amostra)

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
    // Após o processamento do filtro FIR
printf("Gravando %d amostras no arquivo WAV depois dos filtros\n", tamanho);  // Para áudio estéreo (duas amostras por amostra)

}

// Função para normalizar o sinal entre -1 e 1 com base no valor máximo de 16 bits (32767)
void normalizar_sinal(short *sinal, int tamanho) {
    float max_val = 0.0;
    // Encontrar o valor máximo
    printf("Gravando %d amostras no arquivo WAV  a normalização\n", tamanho);  // Para áudio estéreo (duas amostras por amostra)
    for (int i = 0; i < tamanho; i++) {
        if (fabs(sinal[i]) > max_val) {
            max_val = fabs(sinal[i]);
        }
    }
    // Evitar divisão por zero
    if (max_val == 0.0) {
        max_val = 1.0;
    }
    // Normalizar o sinal
    for (int i = 0; i < tamanho; i++) {
        sinal[i] = (short)((sinal[i] / max_val) * MAX_16BIT);
    }
    printf("Gravando %d amostras no arquivo WAV apos a normalização\n", tamanho);  // Para áudio estéreo (duas amostras por amostra)
}

#define SAMPLE_RATE 44100  // Taxa de amostragem

// Função para ler o arquivo WAV estéreo e manter a saída em estéreo
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

// Função para salvar o arquivo WAV com o sinal filtrado e normalizado em estéreo
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
    printf("sub_chunk2_size: %u\n", sub_chunk2_size);
    printf("Tamanho do sinal após chunksize: %d\n", tamanho);
    // Escrever o sinal filtrado em estéreo
    fwrite(sinal, sizeof(short), tamanho * 2, file);

    fclose(file);
    return 0;
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


int main() {
    // Nome do arquivo de entrada e saída
    const char *input_file = "/home/joselito/git/tcc/datas/audio02.wav";
    const char *output_file = "/home/joselito/git/tcc/datas/saida_filtrada_estereo.wav";

    // Variáveis de arquivo
    short *sinal_estereo = NULL;
    short *sinal_filtrado = NULL;
    int tamanho = 0;

    // Leitura do arquivo de entrada estéreo
    if (ler_wav_estereo(input_file, &sinal_estereo, &tamanho) != 0) {
        return -1;
    }

    // Alocar memória para o sinal filtrado
    sinal_filtrado = (short *)malloc(tamanho * 2 * sizeof(short));  // 2 canais (estéreo)
    if (!sinal_filtrado) {
        printf("Erro ao alocar memória para o sinal filtrado\n");
        free(sinal_estereo);
        return -1;
    }

        printf("Tamanho do sinal: %d\n", tamanho);

    // Gerar coeficientes para o filtro FIR
    float coeficientes[ORDEM];
    gerar_filtro_FIR(coeficientes, ORDEM, 300.0, SAMPLE_RATE);

    // Aplicar filtro FIR
    aplicar_filtro_FIR(sinal_estereo, sinal_filtrado, tamanho*2, coeficientes, ORDEM);

    // Normalizar sinal filtrado
    normalizar_sinal(sinal_filtrado, tamanho);

    // Escrever o sinal filtrado no arquivo de saída
    if (escrever_wav_estereo(output_file, sinal_filtrado, tamanho) != 0) {
        free(sinal_estereo);
        free(sinal_filtrado);
        return -1;
    }

    // Liberar memória
    free(sinal_estereo);
    free(sinal_filtrado);

    return 0;
}
