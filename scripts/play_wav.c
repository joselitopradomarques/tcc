#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>

#define PCM_DEVICE "default"  // Dispositivo de áudio (pode ser 'hw:0', 'plughw:0', ou 'default')

void play_audio(const char *filename) {
    // Abrindo o arquivo WAV com a biblioteca libsndfile
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    sndfile = sf_open(filename, SFM_READ, &sfinfo);
    
    if (!sndfile) {
        printf("Erro ao abrir o arquivo WAV: %s\n", sf_strerror(NULL));
        return;
    }
    
    // Obtendo o número de canais e a taxa de amostragem
    int num_channels = sfinfo.channels;
    int sample_rate = sfinfo.samplerate;

    // Preparando o dispositivo ALSA para reprodução
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;  // Formato de amostra (16 bits, Little Endian)
    
    // Abrindo o dispositivo de áudio
    if (snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        printf("Erro ao abrir o dispositivo de áudio.\n");
        sf_close(sndfile);
        return;
    }

    snd_pcm_hw_params_alloca(&params);
    
    // Definindo parâmetros de hardware
    if (snd_pcm_hw_params_any(pcm_handle, params) < 0) {
        printf("Erro ao configurar parâmetros de hardware.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    if (snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        printf("Erro ao configurar o acesso de leitura/gravação intercalado.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    if (snd_pcm_hw_params_set_format(pcm_handle, params, format) < 0) {
        printf("Erro ao configurar o formato de áudio.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    if (snd_pcm_hw_params_set_rate(pcm_handle, params, sample_rate, 0) < 0) {
        printf("Erro ao configurar a taxa de amostragem.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    if (snd_pcm_hw_params_set_channels(pcm_handle, params, num_channels) < 0) {
        printf("Erro ao configurar o número de canais.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    // Aplicando os parâmetros
    if (snd_pcm_hw_params(pcm_handle, params) < 0) {
        printf("Erro ao aplicar parâmetros de hardware.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    // Buffer para armazenar o áudio lido
    short *buffer = (short *)malloc(1024 * num_channels * sizeof(short));
    if (buffer == NULL) {
        printf("Erro ao alocar memória para o buffer de áudio.\n");
        snd_pcm_close(pcm_handle);
        sf_close(sndfile);
        return;
    }

    // Reproduzindo o áudio
    int frames;
    while ((frames = sf_readf_short(sndfile, buffer, 1024)) > 0) {
        if (snd_pcm_writei(pcm_handle, buffer, frames) < 0) {
            printf("Erro ao escrever no dispositivo de áudio.\n");
            break;
        }
    }

    // Finalizando
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    sf_close(sndfile);
    free(buffer);
}

int main() {
    const char *filename = "audio.wav";  // Substitua pelo caminho do seu arquivo WAV
    play_audio(filename);
    return 0;
}
