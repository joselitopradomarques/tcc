#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define PI 3.14159265358979323846

// Estrutura para armazenar o cabeçalho WAV
typedef struct {
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char subchunk2_id[4];
    int subchunk2_size;
} WavHeader;

// Função para ler o arquivo WAV
int ler_wav(const char *filename, short **sinal, int *tamanho) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV.\n");
        return -1;
    }

    // Ler o cabeçalho WAV
    WavHeader header;
    fread(&header, sizeof(WavHeader), 1, file);

    // Verificar se o arquivo é um WAV válido
    if (strncmp(header.chunk_id, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0) {
        printf("Arquivo WAV inválido.\n");
        fclose(file);
        return -1;
    }

    // Verificar se o áudio é estéreo e 16 bits
    if (header.num_channels != 2 || header.bits_per_sample != 16) {
        printf("O arquivo WAV deve ser estéreo e 16 bits.\n");
        fclose(file);
        return -1;
    }

    // Calcular o número de amostras
    *tamanho = header.subchunk2_size / (header.num_channels * (header.bits_per_sample / 8));

    // Alocar memória para o sinal
    *sinal = (short *)malloc(*tamanho * header.num_channels * sizeof(short));
    if (!(*sinal)) {
        printf("Erro ao alocar memória para o sinal.\n");
        fclose(file);
        return -1;
    }

    // Ler os dados de áudio
    fread(*sinal, sizeof(short), *tamanho * header.num_channels, file);

    fclose(file);
    return 0;
}

// Função para escrever o arquivo WAV
int escrever_wav(const char *filename, short *sinal, int tamanho) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para escrita.\n");
        return -1;
    }

    // Criar o cabeçalho WAV
    WavHeader header;
    strncpy(header.chunk_id, "RIFF", 4);
    strncpy(header.format, "WAVE", 4);
    strncpy(header.subchunk1_id, "fmt ", 4);
    header.subchunk1_size = 16;
    header.audio_format = 1; // PCM
    header.num_channels = 2; // Estéreo
    header.sample_rate = SAMPLE_RATE;
    header.bits_per_sample = 16;
    header.byte_rate = header.sample_rate * header.num_channels * (header.bits_per_sample / 8);
    header.block_align = header.num_channels * (header.bits_per_sample / 8);
    strncpy(header.subchunk2_id, "data", 4);
    header.subchunk2_size = tamanho * header.num_channels * (header.bits_per_sample / 8);
    header.chunk_size = 36 + header.subchunk2_size;

    // Escrever o cabeçalho WAV
    fwrite(&header, sizeof(WavHeader), 1, file);

    // Escrever os dados de áudio
    fwrite(sinal, sizeof(short), tamanho * header.num_channels, file);

    fclose(file);
    return 0;
}

// Função para gerar os coeficientes do filtro FIR usando janela de Hamming
// Função para gerar os coeficientes do filtro FIR passa-altas usando janela de Hamming
void gerar_filtro_FIR_passa_altas(float *coeficientes, int ordem, float corte) {
    float fc = corte / SAMPLE_RATE; // Frequência de corte normalizada
    int M = ordem / 2;

    // Gerar coeficientes do filtro passa-baixas
    for (int n = -M; n <= M; n++) {
        if (n == 0) {
            coeficientes[n + M] = 2 * fc; // Coeficiente central
        } else {
            coeficientes[n + M] = sin(2 * PI * fc * n) / (PI * n); // Coeficientes laterais
        }
        // Aplicar janela de Hamming
        coeficientes[n + M] *= 0.54 - 0.46 * cos(2 * PI * (n + M) / ordem);
    }

    // Converter para passa-altas: subtrair do filtro tudo passa
    for (int n = -M; n <= M; n++) {
        if (n == 0) {
            coeficientes[n + M] = 1.0f - coeficientes[n + M]; // Coeficiente central
        } else {
            coeficientes[n + M] = -coeficientes[n + M]; // Coeficientes laterais
        }
    }
}

// Função para aplicar o filtro FIR a um canal
void aplicar_filtro_FIR_canal(short *canal, int tamanho, float *coeficientes, int ordem) {
    int M = ordem / 2;
    short *canal_filtrado = (short *)malloc(tamanho * sizeof(short));

    for (int i = 0; i < tamanho; i++) {
        float soma = 0.0f;
        for (int j = -M; j <= M; j++) {
            if (i - j >= 0 && i - j < tamanho) {
                soma += canal[i - j] * coeficientes[j + M];
            }
        }
        canal_filtrado[i] = (short)soma;
    }

    // Copiar o canal filtrado de volta para o canal original
    memcpy(canal, canal_filtrado, tamanho * sizeof(short));
    free(canal_filtrado);
}

int main() {
    const char *input_filename =  "/home/joselito/git/tcc/datas/audio01.wav";
    const char *output_filename = "/home/joselito/git/tcc/scripts/output_filtered_04.wav";
    short *sinal = NULL;
    int tamanho = 0;

    // Ler o arquivo WAV
    if (ler_wav(input_filename, &sinal, &tamanho) != 0) {
        return -1;
    }

    // Definir a frequência de corte (em Hz)
    float corte = 22050.0f; // Exemplo: 1000 Hz

    // Ordem do filtro FIR
    int ordem = 121;

    // Gerar os coeficientes do filtro FIR
    float *coeficientes = (float *)malloc(ordem * sizeof(float));
    // Gerar os coeficientes do filtro FIR passa-altas
    gerar_filtro_FIR_passa_altas(coeficientes, ordem, corte);

    // Separar os canais esquerdo e direito
    short *canal_esquerdo = (short *)malloc(tamanho * sizeof(short));
    short *canal_direito = (short *)malloc(tamanho * sizeof(short));
    for (int i = 0; i < tamanho; i++) {
        canal_esquerdo[i] = sinal[2 * i];         // Canal esquerdo
        canal_direito[i] = sinal[2 * i + 1];      // Canal direito
    }

    // Aplicar o filtro FIR a cada canal
    aplicar_filtro_FIR_canal(canal_esquerdo, tamanho, coeficientes, ordem);
    aplicar_filtro_FIR_canal(canal_direito, tamanho, coeficientes, ordem);

    // Combinar os canais filtrados de volta ao sinal
    for (int i = 0; i < tamanho; i++) {
        sinal[2 * i] = canal_esquerdo[i];         // Canal esquerdo
        sinal[2 * i + 1] = canal_direito[i];      // Canal direito
    }

    // Escrever o sinal filtrado em um novo arquivo WAV
    if (escrever_wav(output_filename, sinal, tamanho) != 0) {
        free(sinal);
        free(coeficientes);
        free(canal_esquerdo);
        free(canal_direito);
        return -1;
    }

    // Liberar memória
    free(sinal);
    free(coeficientes);
    free(canal_esquerdo);
    free(canal_direito);

    printf("Filtro aplicado com sucesso. Arquivo salvo como %s\n", output_filename);
    return 0;
}