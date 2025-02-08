#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2
#define BITS_PER_SAMPLE 16
#define DURATION_SECONDS 360 // 6 minutos

// Função para salvar um arquivo WAV com silêncio
int escrever_wav_silencio(const char *filename) {
    // Calcular o número total de amostras
    int num_samples = SAMPLE_RATE * DURATION_SECONDS * NUM_CHANNELS;
    
    // Criar um buffer de silêncio (todos os valores são zero)
    short *sinal_silencio = (short *)calloc(num_samples, sizeof(short));
    if (!sinal_silencio) {
        printf("Erro ao alocar memória para o sinal de silêncio.\n");
        return -1;
    }

    // Abrir o arquivo WAV para escrita
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para escrita: %s\n", filename);
        free(sinal_silencio);
        return -1;
    }

    // Escrever cabeçalho WAV simples (44 bytes)
    unsigned int chunk_size = 36 + num_samples * sizeof(short); // Tamanho total dos dados
    unsigned short audio_format = 1; // PCM
    unsigned short num_channels = NUM_CHANNELS; // 2 canais
    unsigned int sample_rate = SAMPLE_RATE;
    unsigned int byte_rate = SAMPLE_RATE * NUM_CHANNELS * sizeof(short); // 2 canais
    unsigned short block_align = NUM_CHANNELS * sizeof(short); // 2 canais
    unsigned short bits_per_sample = BITS_PER_SAMPLE;

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
    unsigned int sub_chunk2_size = num_samples * sizeof(short); // 2 canais
    fwrite("data", sizeof(char), 4, file);
    fwrite(&sub_chunk2_size, sizeof(unsigned int), 1, file);

    // Escrever o sinal de silêncio (todos os valores são zero)
    fwrite(sinal_silencio, sizeof(short), num_samples, file);

    // Fechar o arquivo
    fclose(file);

    // Liberar a memória alocada
    free(sinal_silencio);

    return 0;
}

int main() {
    const char *filename = "silencio_6minutos.wav";
    
    // Chamar a função para escrever o arquivo WAV com silêncio
    if (escrever_wav_silencio(filename) == 0) {
        printf("Arquivo WAV de silêncio de 6 minutos gerado com sucesso: %s\n", filename);
    } else {
        printf("Falha ao gerar o arquivo WAV.\n");
    }

    return 0;
}
