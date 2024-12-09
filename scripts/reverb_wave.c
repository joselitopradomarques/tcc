#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define SAMPLE_RATE 44100
#define PI 3.141592653589793

// Estrutura para armazenar o estado do reverb
typedef struct {
    float* buffer;
    int bufferSize;
    int writeIndex;
    float decayFactor;
    float preDelayTime;
    int preDelaySamples;
    float wetLevel;
    float dryLevel;
} Reverb;

// Função para inicializar o reverb com parâmetros configuráveis
void initReverb(Reverb *reverb, float decayTime, float preDelay, float wetLevel, float dryLevel) {
    reverb->bufferSize = SAMPLE_RATE * decayTime;  // Tamanho do buffer baseado no tempo de decaimento
    reverb->buffer = (float*)calloc(reverb->bufferSize, sizeof(float));
    reverb->writeIndex = 0;

    // Conversão de dB para fator de decaimento
    reverb->decayFactor = powf(10.0f, decayTime / -20.0f);

    // Configura o tempo de pre-delay em amostras
    reverb->preDelayTime = preDelay;
    reverb->preDelaySamples = (int)(SAMPLE_RATE * preDelay);

    // Níveis do efeito (wet) e do sinal original (dry)
    reverb->wetLevel = wetLevel;
    reverb->dryLevel = dryLevel;
}

// Função para processar uma amostra de áudio com o reverb
float processReverb(Reverb *reverb, float inputSample) {
    int delayIndex = (reverb->writeIndex - reverb->preDelaySamples + reverb->bufferSize) % reverb->bufferSize;
    float delayedSample = reverb->buffer[delayIndex];

    // Combinação de dry e wet
    float outputSample = (inputSample * reverb->dryLevel) + (delayedSample * reverb->wetLevel);

    // Escreve a nova amostra no buffer, aplicando o fator de decaimento
    reverb->buffer[reverb->writeIndex] = inputSample + delayedSample * reverb->decayFactor;

    // Atualiza o índice de gravação circular
    reverb->writeIndex = (reverb->writeIndex + 1) % reverb->bufferSize;

    return outputSample;
}

// Função para escrever dados de áudio em um arquivo WAV
void writeWAV(const char* filename, float* data, int numSamples) {
    FILE *file = fopen(filename, "wb");

    // Valores fixos para o cabeçalho WAV
    uint32_t sampleRate = SAMPLE_RATE;
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    uint32_t byteRate = SAMPLE_RATE * numChannels * sizeof(int16_t);
    uint16_t blockAlign = numChannels * sizeof(int16_t);
    uint16_t bitsPerSample = 16;
    uint32_t subChunk2Size = numSamples * sizeof(int16_t);
    uint32_t chunkSize = 36 + subChunk2Size;

    // Escreve o cabeçalho do WAV
    fwrite("RIFF", 1, 4, file);
    fwrite(&chunkSize, sizeof(uint32_t), 1, file);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    uint32_t subChunk1Size = 16;
    fwrite(&subChunk1Size, sizeof(uint32_t), 1, file);
    fwrite(&audioFormat, sizeof(uint16_t), 1, file);
    fwrite(&numChannels, sizeof(uint16_t), 1, file);
    fwrite(&sampleRate, sizeof(uint32_t), 1, file);
    fwrite(&byteRate, sizeof(uint32_t), 1, file);
    fwrite(&blockAlign, sizeof(uint16_t), 1, file);
    fwrite(&bitsPerSample, sizeof(uint16_t), 1, file);
    fwrite("data", 1, 4, file);
    fwrite(&subChunk2Size, sizeof(uint32_t), 1, file);

    // Converte as amostras em int16_t e escreve no arquivo
    for (int i = 0; i < numSamples; i++) {
        int16_t sample = (int16_t)(data[i] * 32767.0f); // Converte float [-1, 1] para int16_t [-32768, 32767]
        fwrite(&sample, sizeof(int16_t), 1, file);
    }

    fclose(file);
}

int main() {
    Reverb reverb;

    // Parâmetros configuráveis
    float decayTime = 3.0f;  // Tempo de decaimento (segundos)
    float preDelay = 0.02f;  // Pre-delay (segundos)
    float wetLevel = 0.6f;   // Nível do efeito (0.0 a 1.0)
    float dryLevel = 0.4f;   // Nível do sinal original (0.0 a 1.0)
    int duration = 5;        // Duração da gravação (segundos)
    int numSamples = SAMPLE_RATE * duration;
    float frequency = 440.0f; // Frequência da onda senoidal (440 Hz)

    // Inicializa o reverb com os parâmetros configuráveis
    initReverb(&reverb, decayTime, preDelay, wetLevel, dryLevel);

    // Array para armazenar as amostras de saída
    float* outputBuffer = (float*)malloc(numSamples * sizeof(float));

    // Gera a onda senoidal e aplica o reverb
    for (int i = 0; i < numSamples; i++) {
        // Gera uma onda senoidal de 440 Hz
        float inputSample = sinf(2.0f * PI * frequency * i / SAMPLE_RATE);

        // Aplica o reverb à amostra
        outputBuffer[i] = processReverb(&reverb, inputSample);
    }

    // Escreve o arquivo WAV de saída
    writeWAV("reverb_output_configurable.wav", outputBuffer, numSamples);

    // Libera a memória
    free(outputBuffer);
    free(reverb.buffer);

    printf("Arquivo reverb_output_configurable.wav criado com sucesso!\n");
    return 0;
}
