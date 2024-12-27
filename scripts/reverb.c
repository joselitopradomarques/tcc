// reverb.c

#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <math.h>
#include "reverb.h" // Inclui o cabeçalho das funções
#include "filter_final.h"

#define BUFFER_SIZE 44100  // Defina o tamanho do buffer (exemplo: 1 segundo de áudio a 44.1kHz)
#define SAMPLE_RATE 44100

float delayBufferLeft[BUFFER_SIZE];
float delayBufferRight[BUFFER_SIZE];
int delayIndexLeft = 0;
int delayIndexRight = 0;

void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback) {
    reverb->decay = decayDB;
    reverb->feedback = feedback;
    // Converte o tempo de atraso para o número de amostras
    reverb->delay = (int)(delayTimeInSeconds * sampleRate);  // Atraso em amostras
}

float processReverb(Reverb* reverb, float inputSample, float* delayBuffer, int delayBufferSize, int* delayIndex, int sampleIndex) {
    // Se o feedback for 0 (efetivamente quando effectAmount é 0), o reverb é desativado
    if (reverb->feedback == 0.0f) {
        return inputSample;  // Retorna o sinal original sem modificar
    }

    // Se o índice atual for menor que o atraso, não aplica o efeito
    if (sampleIndex < reverb->delay) {
        delayBuffer[*delayIndex] = 0.0f;  // Não aplica o efeito antes do tempo de atraso
        return inputSample;  // Retorna a amostra original, sem efeito
    }

    // Processa o reverb normalmente, sem decaimento no feedback
    float outputSample = delayBuffer[*delayIndex];

    // Atualiza o buffer de delay com a amostra atual + feedback (sem decaimento)
    delayBuffer[*delayIndex] = inputSample + reverb->feedback * delayBuffer[*delayIndex];

    // Incrementa o índice do delay e faz o wraparound para simular um buffer circular
    *delayIndex = (*delayIndex + 1) % delayBufferSize;

    return outputSample;  // Retorna o efeito de reverb
}

float applySigmoidGain(float wetness, float k) {
    // Calcula o feedbackAmount usando uma função sigmoide
    return 1.0f / (1.0f + expf(-k * (wetness - 0.5f)));
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

void applyReverbEffect(const char* inputFilePath, const char* outputFilePath, float wetness) {
    int sampleRate = 44100;
    float decayDB = 1.5f;    // Ajuste do decaimento
    float delayTime = 0.4f;   // Atraso de 1 segundo
    float k = 50.0f;  // Parâmetro que controla a taxa de crescimento (ajuste se necessário)
    float feedbackAmount = applySigmoidGain(wetness, k);  // Aplica a função sigmoide ao wetness
    float gain = 0.9f;        // Redução de ganho para evitar estouro
    int numSamples;

    // Lê o arquivo WAV estéreo de entrada
    short *sinal;
    ler_wav_estereo(inputFilePath, &sinal, &numSamples);
    if (sinal == NULL) {
        exit(1);
    }

    // Inicializa o reverb com o valor de feedback controlado pelo parâmetro wetness
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

    // Aplica uma curva de suavização exponencial ao controle de wetness
    float smoothWetness = pow(wetness, 2);  // Aplica uma curva quadrática para suavização

    // Aplica o reverb e mistura com o som original
    for (int i = 0; i < numSamples; i++) {
        // Processa o reverb nos dois canais
        float wetLeft = processReverb(&reverbLeft, sinal[i*2], delayBufferLeft, BUFFER_SIZE, &delayIndexLeft, i);
        float wetRight = processReverb(&reverbRight, sinal[i*2 + 1], delayBufferRight, BUFFER_SIZE, &delayIndexRight, i);

        // Mistura o som seco (original) com o molhado (com reverb)
        outputLeftChannel[i] = sinal[i*2] * (1.0f - smoothWetness) + wetLeft * smoothWetness;
        outputRightChannel[i] = sinal[i*2 + 1] * (1.0f - smoothWetness) + wetRight * smoothWetness;
    }

    // Aplica o ganho ao sinal processado, antes de aplicar os efeitos
    applyGain(outputLeftChannel, outputRightChannel, numSamples, gain);

    // Normaliza o sinal para evitar clipping (agora após aplicar o ganho e o reverb)
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
