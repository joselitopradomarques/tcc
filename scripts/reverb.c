#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <math.h>
#include "reverb.h"  // Inclui o cabeçalho das funções

#define BUFFER_SIZE 1024  // Tamanho do buffer (exemplo: 1 segundo de áudio a 44.1kHz)
#define SAMPLE_RATE 44100

// Estrutura Reverb
void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback) {
    reverb->decay = decayDB;
    reverb->feedback = feedback;
    // Converte o tempo de atraso para o número de amostras
    reverb->delay = (int)(delayTimeInSeconds * sampleRate);  // Atraso em amostras
    
    // Aloca o buffer de delay e inicializa
    reverb->delayBuffer = malloc(BUFFER_SIZE * sizeof(float));
    if (reverb->delayBuffer == NULL) {
        fprintf(stderr, "Erro de alocação de memória para delayBuffer\n");
        return;
    }
    
    // Inicializa o índice de delay
    reverb->delayIndex = 0;
    
    // Inicializa o buffer de delay com zeros
    for (int i = 0; i < BUFFER_SIZE; i++) {
        reverb->delayBuffer[i] = 0.0f;
    }
}

void freeReverb(Reverb* reverb) {
    if (reverb->delayBuffer != NULL) {
        free(reverb->delayBuffer);
        reverb->delayBuffer = NULL;
    }
}

// Processa o efeito de reverb
float processReverb(Reverb* reverb, float inputSample) {
    if (reverb->feedback == 0.0f) {
        return inputSample;  // Retorna o sinal original sem aplicar o reverb
    }

    // Calcula o delay com base no índice atual e o buffer de delay
    float outputSample = reverb->delayBuffer[reverb->delayIndex];

    // Atualiza o buffer de delay com a amostra atual + feedback
    reverb->delayBuffer[reverb->delayIndex] = inputSample + reverb->feedback * outputSample;

    // Incrementa o índice de delay e faz o wraparound para o buffer circular
    reverb->delayIndex = (reverb->delayIndex + 1) % BUFFER_SIZE;

    return outputSample;
}

void applyReverbEffectBuffer(float* buffer, int numSamples, float wetness, float feedback) {
    int maxDelay = 22050;  // Aproximadamente 0.5 segundos de delay para 44.1kHz
    static float *delayBuffer = NULL;  // Buffer de delay (estático para manter o estado entre as chamadas)
    static int delayIndex = 0;  // Índice do buffer de delay (circular)
    
    // Inicializar o buffer de delay apenas na primeira execução
    if (delayBuffer == NULL) {
        delayBuffer = (float *)malloc(maxDelay * sizeof(float));
        if (delayBuffer == NULL) {
            // Se não conseguir alocar memória para o buffer, retorne imediatamente
            return;
        }

        // Inicializar o buffer de delay com zeros
        for (int i = 0; i < maxDelay; i++) {
            delayBuffer[i] = 0.0f;
        }
    }

    // Processar o buffer de áudio e aplicar o reverb
    for (int i = 0; i < numSamples; i++) {
        // Sinal seco (não processado)
        float drySignal = buffer[i];

        // Sinal molhado (o sinal com o efeito de reverb aplicado)
        float wetSignal = delayBuffer[delayIndex];

        // Mistura entre o sinal seco e o sinal molhado
        buffer[i] = (1.0f - wetness) * drySignal + wetness * wetSignal;

        // Feedback suave, com limites controlados
        float feedbackSignal = wetSignal * feedback;

        // Atualiza o buffer de delay com o feedback
        delayBuffer[delayIndex] = drySignal + feedbackSignal;

        // Avança o índice circular para o próximo slot do buffer de delay
        delayIndex = (delayIndex + 1) % maxDelay;
    }
}










// Função auxiliar para aplicar ganho ao buffer
void applyGain(float* buffer, int numSamples, float gain) {
    for (int i = 0; i < numSamples; i++) {
        buffer[i] *= gain;
    }
}

// Função para normalizar o buffer para evitar clipping
void normalize(float* buffer, int numSamples) {
    float maxVal = 0.0f;

    // Encontra o valor máximo absoluto no buffer
    for (int i = 0; i < numSamples; i++) {
        if (fabs(buffer[i]) > maxVal) {
            maxVal = fabs(buffer[i]);
        }
    }

    // Normaliza o buffer para evitar clipping
    if (maxVal > 0.0f) {
        float normalizationFactor = 1.0f / maxVal;
        for (int i = 0; i < numSamples; i++) {
            buffer[i] *= normalizationFactor;
        }
    }
}
