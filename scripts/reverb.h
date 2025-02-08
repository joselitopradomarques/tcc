#ifndef REVERB_H
#define REVERB_H

#define BUFFER_SIZE 1024  // Tamanho do buffer de delay

// Estrutura para o efeito de reverb
typedef struct {
    float* delayBuffer;  // Buffer de delay
    int delayIndex;      // Índice do buffer de delay
    int delay;           // Atraso em amostras
    float feedback;      // Feedback do reverb
    float decay;         // Decaimento do efeito (não usado diretamente aqui, mas pode ser útil)
} Reverb;

// Funções de inicialização e processamento
void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback);
void freeReverb(Reverb* reverb);
void applyReverbEffectBuffer(float* buffer, int numSamples, float wetness, float feedback);
void applyGain(float* buffer, int numSamples, float gain);
void normalize(float* buffer, int numSamples);

#endif
