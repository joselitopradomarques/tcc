#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <math.h>
#include "reverb.h" // Inclui o cabeçalho das funções

#define BUFFER_SIZE 44100  // Defina o tamanho do buffer (exemplo: 1 segundo de áudio a 44.1kHz)
#define SAMPLE_RATE 44100


float delayBufferLeft[BUFFER_SIZE];
float delayBufferRight[BUFFER_SIZE];
int delayIndexLeft = 0;
int delayIndexRight = 0;

// Função para ler o arquivo WAV estéreo
int ler_wav_estereo(const char *filename, short **sinal, int *tamanho) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV: %s\n", filename);
        return -1;  // Apenas retorna sem fazer mais nada
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
        return -1;  // Apenas retorna sem fazer mais nada
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
        return -1;  // Apenas retorna sem fazer mais nada
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

void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback) {
    reverb->decay = decayDB;
    reverb->feedback = feedback;
    // Converte o tempo de atraso para o número de amostras
    reverb->delay = (int)(delayTimeInSeconds * sampleRate);  // Atraso em amostras
}

// Processamento do reverb com o atraso aplicado
float processReverb(Reverb* reverb, float inputSample, float* delayBuffer, int delayBufferSize, int* delayIndex, int sampleIndex) {
    // Se o índice atual for menor que o atraso, não aplica o efeito
    if (sampleIndex < reverb->delay) {
        delayBuffer[*delayIndex] = 0.0f;  // Não aplica o efeito antes do tempo de atraso
        return 0.0f;  // Retorna 0 antes do início do efeito
    }
    
    // Quando o atraso for superado, começa a aplicar o reverb
    float outputSample = delayBuffer[*delayIndex];  // Somente o valor do buffer de delay

    // Atualiza o buffer de delay com a amostra atual + feedback ajustado pelo decaimento
    delayBuffer[*delayIndex] = inputSample + reverb->feedback * delayBuffer[*delayIndex];

    // Incrementa o índice do delay e faz o wraparound para simular um buffer circular
    *delayIndex = (*delayIndex + 1) % delayBufferSize;

    return outputSample;  // Retorna apenas o efeito de reverb
}

void applyGain(float* leftChannel, float* rightChannel, int numSamples, float gain) {
    for (int i = 0; i < numSamples; i++) {
        leftChannel[i] *= gain;
        rightChannel[i] *= gain;
    }
}

void normalize(float* leftChannel, float* rightChannel, int numSamples) {
    float maxLeft = 0.0f, maxRight = 0.0f;

    // Encontra o valor máximo absoluto em cada canal
    for (int i = 0; i < numSamples; i++) {
        if (fabs(leftChannel[i]) > maxLeft) {
            maxLeft = fabs(leftChannel[i]);
        }
        if (fabs(rightChannel[i]) > maxRight) {
            maxRight = fabs(rightChannel[i]);
        }
    }

    float maxVal = fmax(maxLeft, maxRight);

    // Normaliza os canais para evitar clipping
    if (maxVal > 0.0f) {
        float normalizationFactor = 1.0f / maxVal;
        for (int i = 0; i < numSamples; i++) {
            leftChannel[i] *= normalizationFactor;
            rightChannel[i] *= normalizationFactor;
        }
    }
}

void applyReverbEffect(const char* inputFilePath, const char* outputFilePath, float effectAmount) {
    int sampleRate = 44100;
    float decayDB = 40.0f;    // Ajuste do decaimento
    float delayTime = 1.0f;   // Atraso de 4 segundos
    float feedbackAmount = effectAmount;  // Ajuste de feedback controlado pelo parâmetro
    float gain = 0.5f;        // Redução de ganho para evitar estouro
    int numSamples;

    // Lê o arquivo WAV estéreo de entrada
    short *sinal;
    ler_wav_estereo(inputFilePath, &sinal, &numSamples);
    if (sinal == NULL) {
        exit(1);
    }

    // Inicializa o reverb com o valor de feedback controlado pelo parâmetro effectAmount
    Reverb reverbLeft, reverbRight;
    initReverb(&reverbLeft, decayDB, delayTime, sampleRate, feedbackAmount);
    initReverb(&reverbRight, decayDB, delayTime, sampleRate, feedbackAmount);

    // Arrays para armazenar as amostras de saída dos dois canais
    float* outputLeftChannel = (float*)malloc(numSamples * sizeof(float));
    float* outputRightChannel = (float*)malloc(numSamples * sizeof(float));

    if (outputLeftChannel == NULL || outputRightChannel == NULL) {
        printf("Erro ao alocar memória para os canais de saída.\n");
        exit(1);
    }

    // Aplica o reverb em ambos os canais com delay
    for (int i = 0; i < numSamples; i++) {
        outputLeftChannel[i] = processReverb(&reverbLeft, sinal[i*2], delayBufferLeft, BUFFER_SIZE, &delayIndexLeft, i);
        outputRightChannel[i] = processReverb(&reverbRight, sinal[i*2 + 1], delayBufferRight, BUFFER_SIZE, &delayIndexRight, i);
    }

    // Aplica o ganho ao sinal processado
    applyGain(outputLeftChannel, outputRightChannel, numSamples, gain);

    // Normaliza o sinal para evitar clipping
    normalize(outputLeftChannel, outputRightChannel, numSamples);

    // Converte os valores de volta para short e escreve o arquivo WAV estéreo de saída
    short* outputSignal = (short*)malloc(numSamples * 2 * sizeof(short));
    for (int i = 0; i < numSamples; i++) {
        outputSignal[i*2] = (short)(outputLeftChannel[i] * 32767);   // Conversão de volta para valores de 16-bit
        outputSignal[i*2 + 1] = (short)(outputRightChannel[i] * 32767);
    }
    escrever_wav_estereo(outputFilePath, outputSignal, numSamples);

    // Libera a memória
    free(outputLeftChannel);
    free(outputRightChannel);
    free(sinal);
    free(outputSignal);

    printf("Arquivo %s criado com sucesso!\n", outputFilePath);
}

int main() {
    // Caminho do arquivo de entrada e saída
    const char* inputFilePath = "/home/joselito/git/tcc/scripts/audio02.wav";
    const char* outputFilePath = "/home/joselito/git/tcc/scripts/reverb_output_stereo.wav";

    // Define o efeito de reverb com o valor entre 0.0 e 1.0, onde 1.0 corresponde ao feedback máximo (100 dB)
    float effectAmount = 0.3f;  // Ajuste para variar entre 0.0 (sem efeito) e 1.0 (feedback máximo)

    // Chama a função de aplicação do efeito com o parâmetro effectAmount
    applyReverbEffect(inputFilePath, outputFilePath, effectAmount);

    return 0;
}
