#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <stdint.h>  // Para int32_t

#define SAMPLE_RATE 44100  // Taxa de amostragem do áudio
#define CHANNELS 2         // Número de canais (2 para estéreo)
#define BUFFER_FRAMES 8192 // Número de frames de áudio
#define PERIOD_SIZE 128    // Número de frames por período

int main() {
    // Configurações de dispositivos ALSA
    snd_pcm_t *capture_handle;  // Dispositivo de captura
    snd_pcm_t *playback_handle; // Dispositivo de reprodução
    snd_pcm_hw_params_t *hw_params;
    int err;
    
    // Cada amostra de áudio de 24 bits requer 3 bytes por canal, então usamos o frame_size
    int frame_size = 3 * CHANNELS; // 3 bytes por canal em áudio de 24 bits (S24_3LE)
    int period_bytes = frame_size * PERIOD_SIZE;  // Número de bytes por período
    int32_t *buffer = (int32_t *) malloc(BUFFER_FRAMES * CHANNELS * sizeof(int32_t));

    // Verifica se a memória foi alocada corretamente
    if (buffer == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o buffer.\n");
        return 1;
    }

    // Abrir dispositivo de captura (hw:3,0)
    if ((err = snd_pcm_open(&capture_handle, "hw:3,0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de captura: %s\n", snd_strerror(err));
        return 1;
    }

    // Abrir dispositivo de reprodução (hw:3,0)
    if ((err = snd_pcm_open(&playback_handle, "hw:3,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de reprodução: %s\n", snd_strerror(err));
        return 1;
    }

    // Configurar os parâmetros de captura
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    // Configuração para 24 bits por amostra (S24_3LE)
    snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S24_3LE);
    snd_pcm_hw_params_set_rate(capture_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);

    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        fprintf(stderr, "Erro ao configurar os parâmetros de captura: %s\n", snd_strerror(err));
        return 1;
    }

    // Configurar os parâmetros de reprodução
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(playback_handle, hw_params);
    snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_period_size(capture_handle, hw_params, PERIOD_SIZE, 0);
    // Configuração para 24 bits por amostra (S24_3LE)
    snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S24_3LE);
    snd_pcm_hw_params_set_rate(playback_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(playback_handle, hw_params, CHANNELS);
    snd_pcm_hw_params_set_period_size(playback_handle, hw_params, PERIOD_SIZE, 0);

    if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
        fprintf(stderr, "Erro ao configurar os parâmetros de reprodução: %s\n", snd_strerror(err));
        return 1;
    }

    // Loop de captura e reprodução
    while (1) {
        // Captura o áudio do dispositivo hw:3,0
        if ((err = snd_pcm_readi(capture_handle, buffer, BUFFER_FRAMES)) != BUFFER_FRAMES) {
            fprintf(stderr, "Erro ao capturar o áudio: %s\n", snd_strerror(err));
            return 1;
        }
        
        // Reproduz o áudio no dispositivo hw:3,0
        if ((err = snd_pcm_writei(playback_handle, buffer, BUFFER_FRAMES)) != BUFFER_FRAMES) {
            fprintf(stderr, "Erro ao reproduzir o áudio: %s\n", snd_strerror(err));
            return 1;
        }
        
    }

    // Fechar os dispositivos ALSA
    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);

    // Libera a memória alocada para o buffer
    free(buffer);

    return 0;
}
