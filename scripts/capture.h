#ifndef CAPTURE_H
#define CAPTURE_H

#include <alsa/asoundlib.h>

#define DEVICE "hw:3,0"      // Defina o dispositivo de captura
#define SAMPLE_RATE 44100     // Taxa de amostragem
#define FORMAT SND_PCM_FORMAT_S16_LE  // Formato de áudio
#define CHANNELS 2           // Número de canais de áudio

// Definição do buffer circular
typedef struct {
    short *buffer;  // Buffer de áudio
    size_t size;    // Tamanho total do buffer
    size_t start;   // Posição de leitura
    size_t end;     // Posição de escrita
    size_t count;   // Contagem de elementos no buffer
} CircularBuffer;

// Funções de captura de áudio e buffer circular
int configure_capture(snd_pcm_t **pcm_handle, snd_pcm_hw_params_t **params);
void capture_audio_loop(snd_pcm_t *pcm_handle, CircularBuffer *write_buffer, CircularBuffer *read_buffer);
void close_capture(snd_pcm_t *pcm_handle, CircularBuffer *write_buffer, CircularBuffer *read_buffer);
int init_circular_buffer(CircularBuffer *circular_buffer, size_t size);
void free_circular_buffer(CircularBuffer *circular_buffer);
int write_circular_buffer(CircularBuffer *circular_buffer, short *data, size_t size);
int read_circular_buffer(CircularBuffer *circular_buffer, short *data, size_t size);

#endif // CAPTURE_H