#ifndef FX_H
#define FX_H

#include <sndfile.h>

// -------------------- Efeitos de Reverb --------------------

// Estrutura que define as propriedades do efeito Reverb
typedef struct {
    float decay;   // Decaimento do efeito
    int delay;     // Atraso em amostras
    float feedback; // Feedback do efeito
} Reverb;

// Tamanho do buffer de delay (1 segundo de áudio a 44.1kHz)
#define REVERB_BUFFER_SIZE 44100

// Funções de processamento de reverb
void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback);
float processReverb(Reverb* reverb, float inputSample, float* delayBuffer, int delayBufferSize, int* delayIndex, int sampleIndex);

// Funções auxiliares para leitura e gravação de arquivos WAV (Reverb)
void ler_wav_estereo(const char* filename, short **sinal, int *tamanho);
int escrever_wav_estereo(const char* filename, short *sinal, int tamanho);

// Funções auxiliares para manipulação de áudio (Reverb)
void applyGain(float* leftChannel, float* rightChannel, int numSamples, float gain);
void normalize(float* leftChannel, float* rightChannel, int numSamples);
void applyReverbEffect(const char* inputFilePath, const char* outputFilePath, float effectAmount);


// -------------------- Efeitos de Delay --------------------

// Definições e parâmetros do delay
#define SAMPLE_RATE 44100
#define MAX_DELAY_TIME 1000 // Máximo delay de 1s (1000 ms)

// Estrutura do buffer de delay
typedef struct {
    float *buffer;
    int buffer_size;
    int write_index;
} DelayBuffer;

// Função para inicializar o buffer de delay
void init_delay_buffer(DelayBuffer *db, int delay_ms);

// Função para processar o delay
float process_delay(DelayBuffer *db, float input, int delay_ms);

// Função para aplicar delay ao áudio
void apply_delay_to_audio(const char *input_filename, const char *output_filename, int delay_time);

#endif // FX_H
