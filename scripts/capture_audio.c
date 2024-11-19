#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "default" // Dispositivo de captura de áudio padrão

int main() {
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // Formato de 16 bits little-endian
    unsigned int rate = 44100; // Taxa de amostragem de 44.1 kHz
    int channels = 1; // Mono
    int buffer_size = 1024; // Tamanho do buffer de captura
    short *buffer;
    int err;

    // Abrir o dispositivo de captura
    if ((err = snd_pcm_open(&capture_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de captura: %s\n", snd_strerror(err));
        return 1;
    }

    // Alocar memória para os parâmetros de hardware
    snd_pcm_hw_params_alloca(&params);

    // Definir os parâmetros do dispositivo de captura
    if ((err = snd_pcm_hw_params_any(capture_handle, params)) < 0) {
        fprintf(stderr, "Erro ao obter parâmetros do dispositivo: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_access(capture_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "Erro ao configurar o acesso ao dispositivo: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_format(capture_handle, params, format)) < 0) {
        fprintf(stderr, "Erro ao configurar o formato de áudio: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, params, &rate, 0)) < 0) {
        fprintf(stderr, "Erro ao configurar a taxa de amostragem: %s\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_channels(capture_handle, params, channels)) < 0) {
        fprintf(stderr, "Erro ao configurar o número de canais: %s\n", snd_strerror(err));
        return 1;
    }

    // Aplicar os parâmetros configurados
    if ((err = snd_pcm_hw_params(capture_handle, params)) < 0) {
        fprintf(stderr, "Erro ao aplicar os parâmetros: %s\n", snd_strerror(err));
        return 1;
    }

    // Alocar o buffer para captura
    buffer = (short *)malloc(buffer_size * sizeof(short));

    printf("Capturando áudio...\n");

    // Capturar e imprimir os dados de áudio
    while (1) {
        if ((err = snd_pcm_readi(capture_handle, buffer, buffer_size)) != buffer_size) {
            fprintf(stderr, "Erro ao ler o áudio: %s\n", snd_strerror(err));
            return 1;
        }

        // Imprimir os dados capturados
        for (int i = 0; i < buffer_size; i++) {
            printf("%d ", buffer[i]);
        }
        printf("\n");
    }

    // Liberação de recursos
    snd_pcm_close(capture_handle);
    free(buffer);

    return 0;
}
