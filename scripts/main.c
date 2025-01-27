#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "capture.h"

// Declare a constante externa UPDATE_BUFFER_SIZE
extern const size_t UPDATE_BUFFER_SIZE;

int main() {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    CircularBuffer write_buffer, read_buffer;
    size_t circular_buffer_size = UPDATE_BUFFER_SIZE * 1024;  // Tamanho do buffer circular baseado no UPDATE_BUFFER_SIZE

    // Inicializa buffers circulares
    if (init_circular_buffer(&write_buffer, circular_buffer_size) < 0 || init_circular_buffer(&read_buffer, circular_buffer_size) < 0) {
        fprintf(stderr, "Erro ao inicializar buffer circular\n");
        return -1;
    }

    // Configura captura de áudio
    if (configure_capture(&pcm_handle, &params) < 0) {
        fprintf(stderr, "Erro ao configurar captura de áudio\n");
        return -1;
    }

    // Captura áudio em loop
    capture_audio_loop(pcm_handle, &write_buffer, &read_buffer);

    // Fecha captura e libera buffers
    close_capture(pcm_handle, &write_buffer, &read_buffer);

    return 0;
}