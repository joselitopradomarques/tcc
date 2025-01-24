#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <time.h>  // Inclua a biblioteca de tempo para controle do tempo de captura

#define DEVICE "hw:4,0"  // Defina o dispositivo desejado (hw:3,0)
#define PCM_CAPTURE_FILE "captured_audio.bin"  // Arquivo de captura de áudio em formato binário
#define CAPTURE_TIME 10  // Tempo de captura em segundos (modificado para 10 segundos)
#define SAMPLE_RATE 44100  // Taxa de amostragem (44100 Hz para qualidade CD)
#define CHANNELS 2         // Número de canais (Estéreo)
#define FORMAT SND_PCM_FORMAT_S16_LE  // Formato de áudio (16 bits, little-endian)

int main() {
    int rc;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    unsigned int rate = SAMPLE_RATE;
    snd_pcm_uframes_t buffer_size = 1024;  // Tamanho do buffer
    int periods = 2;  // Número de períodos
    short *buffer;  // Buffer para os dados capturados

    // Inicializar a captura com logging
    printf("Iniciando configuração ALSA...\n");

    // Abrir o dispositivo de captura
    rc = snd_pcm_open(&pcm_handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo %s: %s\n", DEVICE, snd_strerror(rc));
        return -1;
    }
    printf("Dispositivo PCM aberto com sucesso: %s\n", DEVICE);

    // Alocar memória para os parâmetros de hardware
    snd_pcm_hw_params_malloc(&params);

    // Configurar parâmetros de hardware
    snd_pcm_hw_params_any(pcm_handle, params);
    rc = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar acesso intercalado: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params_set_format(pcm_handle, params, FORMAT);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar formato: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Formato configurado com sucesso: S16_LE\n");

    rc = snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);  // Estéreo
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar canais: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Canais configurados com sucesso: %d\n", CHANNELS);

    rc = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar taxa de amostragem: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Taxa de amostragem configurada com sucesso: %d Hz\n", rate);

    rc = snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &buffer_size, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar tamanho do frame: %s\n", snd_strerror(rc));
        return -1;
    }
    printf("Tamanho do frame configurado com sucesso: %ld frames\n", buffer_size);

    rc = snd_pcm_hw_params_set_periods_near(pcm_handle, params, &periods, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar número de períodos: %s\n", snd_strerror(rc));
        return -1;
    }

    // Aplicar os parâmetros ao dispositivo
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar os parâmetros de hardware: %s\n", snd_strerror(rc));
        return -1;
    }

    // Liberar a memória alocada para os parâmetros
    snd_pcm_hw_params_free(params);

    // Alocar memória para o buffer de captura
    buffer = (short *) malloc(buffer_size * CHANNELS * sizeof(short)); // 16 bits por amostra (S16_LE)

    if (buffer == NULL) {
        perror("Erro ao alocar memória para o buffer");
        return -1;
    }

    // Variáveis de controle de tempo
    time_t start_time = time(NULL);  // Tempo inicial

    printf("Iniciando captura de áudio...\n");

    // Abrir arquivo para gravação em formato binário
    FILE *bin_file = fopen(PCM_CAPTURE_FILE, "wb");
    if (!bin_file) {
        printf("Erro ao abrir o arquivo para escrita!\n");
        return -1;
    }

    while (1) {
        // Verifica se o tempo de captura foi atingido
        if (difftime(time(NULL), start_time) >= CAPTURE_TIME) {
            printf("Tempo de captura de %d segundos alcançado. Finalizando...\n", CAPTURE_TIME);
            break;
        }

        // Captura o áudio em frames
        int frames = snd_pcm_readi(pcm_handle, buffer, buffer_size);
        if (frames < 0) {
            fprintf(stderr, "Erro de captura: %s\n", snd_strerror(frames));
            break;
        }

        // Escrever os dados capturados no arquivo binário
        fwrite(buffer, sizeof(short), frames * CHANNELS, bin_file);  // 2 canais (estéreo)
    }

    // Fechar arquivo de captura
    fclose(bin_file);

    // Fechar o dispositivo
    snd_pcm_close(pcm_handle);
    free(buffer);

    printf("Captura de áudio finalizada e salva em %s\n", PCM_CAPTURE_FILE);

    return 0;
}
