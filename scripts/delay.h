#ifndef DELAY_H
#define DELAY_H

#include <sndfile.h>

#define SAMPLE_RATE 44100
#define MAX_DELAY_TIME 1000 // Máximo delay de 1s (1000 ms)

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

#endif // DELAY_H
