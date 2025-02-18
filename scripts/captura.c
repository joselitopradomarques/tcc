/*
gcc -o captura captura.c -lasound
./captura
*/



#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "hw:1,0"  // Ajuste para o dispositivo correto
#define RATE 44100           // Taxa de amostragem
#define CHANNELS 2           // Número de canais (estéreo)
#define SECONDS 10           // Duração da captura em segundos
#define FORMAT SND_PCM_FORMAT_S24_3LE  // Formato de áudio (24 bits)
#define FRAME_SIZE 2048      // Tamanho do frame desejado

int main() {
    int rc;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    unsigned int rate = RATE;
    int dir;    // Direção inicial, pode ser qualquer valor (não será usado diretamente)
    snd_pcm_uframes_t frames = FRAME_SIZE;  // Definindo o tamanho do frame
    int buffer_size;

    // Inicializar a captura com logging
    printf("Iniciando configuração ALSA...\n");

    // Abrir o dispositivo de captura PCM
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo PCM: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Dispositivo PCM aberto com sucesso: %s\n", PCM_DEVICE);

    // Alocar parâmetros de hardware
    snd_pcm_hw_params_alloca(&params);

    // Inicializar parâmetros de hardware
    snd_pcm_hw_params_any(pcm_handle, params);

    // Definir o modo de acesso para leitura intercalada
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar acesso intercalado: %s\n", snd_strerror(rc));
        return -1;
    }

    // Definir o formato (24 bits, 3 bytes)
    rc = snd_pcm_hw_params_set_format(pcm_handle, params, FORMAT);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar formato: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Formato configurado com sucesso: S24_3LE\n");

    // Definir número de canais (estéreo)
    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar canais: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Canais configurados com sucesso: %d\n", CHANNELS);

    // Definir a taxa de amostragem
    rc = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, &dir);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar taxa de amostragem: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Taxa de amostragem configurada com sucesso: %d Hz\n", rate);

    // Definir o tamanho do frame (período)
    rc = snd_pcm_hw_params_set_period_size(pcm_handle, params, frames, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar tamanho do frame: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Tamanho do frame configurado com sucesso: %ld frames\n", frames);

    // Aplicar os parâmetros de hardware
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "Erro ao aplicar os parâmetros de hardware: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Parâmetros de hardware aplicados com sucesso\n");

    // Buffer de captura
    buffer_size = frames * CHANNELS * (24 / 8);  // 24 bits = 3 bytes
    char *buffer = (char *)malloc(buffer_size);

    if (!buffer) {
        fprintf(stderr, "Erro ao alocar memória para o buffer\n");
        return -1;
    }

    // Capturar o áudio por um período
    FILE *file = fopen("capture_test_1024.bin", "wb");
    if (!file) {
        fprintf(stderr, "Erro ao abrir o arquivo para gravação\n");
        return -1;
    }

    printf("Capturando áudio por %d segundos...\n", SECONDS);
    for (int i = 0; i < (RATE * SECONDS) / frames; ++i) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            fprintf(stderr, "Overrun detectado! Preparando o PCM...\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "Erro durante a leitura: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "Lidos menos frames do que o esperado: %d/%ld\n", rc, frames);
        }

        // Escrever o buffer no arquivo
        fwrite(buffer, sizeof(char), buffer_size, file);
    }

    // Fechar o arquivo e limpar recursos
    fclose(file);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);

    printf("Áudio capturado e salvo em 'capture_test_1024.bin'\n");
    return 0;
}
