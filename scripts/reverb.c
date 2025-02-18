#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <math.h>
#include "reverb.h"  // Inclui o cabeçalho das funções

#define BUFFER_SIZE 1024  // Tamanho do buffer (exemplo: 1 segundo de áudio a 44.1kHz)
#define SAMPLE_RATE 44100

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

    // Limitar o feedback a um valor seguro
    feedback = fminf(fmaxf(feedback, 0.2f), 0.7f);  // Feedback deve estar entre 0.2 e 0.7

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
