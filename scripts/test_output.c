#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>  // Biblioteca para leitura de arquivos WAV

#define DEVICE "hw:2,0"  // Dispositivo de saída (fone de ouvido)

// Função para reproduzir o arquivo WAV
void play_audio(const char *filename) {
    int rc;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    SNDFILE *file;
    SF_INFO sf_info;
    short *buffer;
    sf_count_t frames;
    int channels, sample_rate;
    
    // Abrir o arquivo WAV com a libsndfile
    file = sf_open(filename, SFM_READ, &sf_info);
    if (!file) {
        fprintf(stderr, "Erro ao abrir o arquivo WAV: %s\n", sf_strerror(file));
        return;
    }

    channels = sf_info.channels;
    sample_rate = sf_info.samplerate;

    // Abrir o dispositivo de reprodução (ALSA)
    rc = snd_pcm_open(&pcm_handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo %s: %s\n", DEVICE, snd_strerror(rc));
        return;
    }

    // Alocar memória para os parâmetros de hardware
    snd_pcm_hw_params_malloc(&params);

    // Configurar parâmetros de hardware
    rc = snd_pcm_hw_params_any(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar parâmetros de hardware: %s\n", snd_strerror(rc));
        return;
    }

    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar acesso: %s\n", snd_strerror(rc));
        return;
    }

    rc = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar formato: %s\n", snd_strerror(rc));
        return;
    }

    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar canais: %s\n", snd_strerror(rc));
        return;
    }

    rc = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar taxa de amostragem: %s\n", snd_strerror(rc));
        return;
    }

    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar os parâmetros de hardware: %s\n", snd_strerror(rc));
        return;
    }

    // Liberar memória dos parâmetros
    snd_pcm_hw_params_free(params);

    // Alocar buffer para armazenar o áudio
    buffer = malloc(1024 * channels * sizeof(short));  // Tamanho do buffer de leitura

    if (buffer == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o buffer\n");
        return;
    }

    // Reproduzir áudio
    while ((frames = sf_readf_short(file, buffer, 1024)) > 0) {
        rc = snd_pcm_writei(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            fprintf(stderr, "Underrun ocorrido no dispositivo de reprodução\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "Erro na reprodução: %s\n", snd_strerror(rc));
            break;
        }
    }

    // Fechar o arquivo e o dispositivo
    sf_close(file);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);

    printf("Reprodução finalizada.\n");
}

int main() {
    const char *filename = "saida_filtrada_1.wav";  // Caminho do arquivo WAV
    play_audio(filename);
    return 0;
}
