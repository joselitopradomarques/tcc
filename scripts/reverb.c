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
    
    // Quando o atraso for superado, começa a aplicar o reverb
    float outputSample = delayBuffer[*delayIndex];  // Somente o valor do buffer de delay

    // Atualiza o buffer de delay com a amostra atual + feedback ajustado pelo decaimento
    delayBuffer[*delayIndex] = inputSample + reverb->feedback * delayBuffer[*delayIndex];

    // Incrementa o índice do delay e faz o wraparound para simular um buffer circular
    *delayIndex = (*delayIndex + 1) % delayBufferSize;

    return outputSample;  // Retorna o efeito de reverb
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

void applyReverbEffect(const char* inputFilePath, const char* outputFilePath, float effectSend) {
    int sampleRate = 44100;
    float decayDB = 40.0f;    // Ajuste do decaimento
    float delayTime = 1.0f;   // Atraso de 1 segundo
    float feedbackAmount = effectSend;  // Ajuste de feedback controlado pelo parâmetro effectSend
    float gain = 0.5f;        // Redução de ganho para evitar estouro
    int numSamples;

    // Lê o arquivo WAV estéreo de entrada
    short *sinal;
    ler_wav_estereo(inputFilePath, &sinal, &numSamples);
    if (sinal == NULL) {
        exit(1);
    }

    // Inicializa o reverb com o valor de feedback controlado pelo parâmetro effectSend
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

    // Ajusta o efeito usando uma curva exponencial para suavizar o controle
    float smoothEffectSend = pow(effectSend, 2);  // Aplica uma curva quadrática (pode ajustar o expoente conforme necessário)

    // Se o efeito for 0, não aplica nenhum reverb
    if (smoothEffectSend == 0.0f) {
        for (int i = 0; i < numSamples; i++) {
            outputLeftChannel[i] = 0.0f;  // Silêncio total
            outputRightChannel[i] = 0.0f;
        }
    } else {
        // Aplica o reverb gradual nos dois canais
        for (int i = 0; i < numSamples; i++) {
            // Aplica o reverb de acordo com a intensidade do smoothEffectSend
            outputLeftChannel[i] = processReverb(&reverbLeft, sinal[i*2], delayBufferLeft, BUFFER_SIZE, &delayIndexLeft, i) * smoothEffectSend;
            outputRightChannel[i] = processReverb(&reverbRight, sinal[i*2 + 1], delayBufferRight, BUFFER_SIZE, &delayIndexRight, i) * smoothEffectSend;
        }
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
