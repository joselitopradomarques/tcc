#ifndef REVERB_H
#define REVERB_H

// Estrutura que define as propriedades do efeito Reverb
typedef struct {
    float decay;   // Decaimento do efeito
    int delay;     // Atraso em amostras
    float feedback; // Feedback do efeito
} Reverb;

// Tamanho do buffer de delay (1 segundo de áudio a 44.1kHz)
#define BUFFER_SIZE 44100

// Funções de processamento de reverb
void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback);
// Arquivo: reverb.h
float processReverb(Reverb* reverb, float inputSample, float* delayBuffer, int delayBufferSize, int* delayIndex, int sampleIndex, float effectAmount);


// Funções auxiliares para leitura e gravação de arquivos WAV
int ler_wav_estereo(const char* filename, short **sinal, int *tamanho);
int escrever_wav_estereo(const char* filename, short *sinal, int tamanho);

// Funções auxiliares para manipulação de áudio
void applyGain(float* leftChannel, float* rightChannel, int numSamples, float gain);
void normalize(float* leftChannel, float* rightChannel, int numSamples);
void applyReverbEffect(const char* inputFilePath, const char* outputFilePath, float effectAmount);

#endif
