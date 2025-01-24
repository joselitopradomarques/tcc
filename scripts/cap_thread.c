#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <time.h>  // Inclua a biblioteca de tempo para controle do tempo de captura

#define DEVICE_1 "hw:4,0"  // Dispositivo 1
#define DEVICE_2 "hw:3,0"  // Dispositivo 2
#define PCM_CAPTURE_FILE_1 "captured_audio_1.bin"  // Arquivo de captura de áudio do dispositivo 1
#define PCM_CAPTURE_FILE_2 "captured_audio_2.bin"  // Arquivo de captura de áudio do dispositivo 2
#define CAPTURE_TIME 1  // Tempo de captura em segundos (modificado para 10 segundos)
#define SAMPLE_RATE 44100  // Taxa de amostragem (44100 Hz para qualidade CD)
#define CHANNELS_1 2        // Número de canais (Estéreo para o dispositivo 1)
#define CHANNELS_2 2        // Número de canais (Estéreo para o dispositivo 2)
#define FORMAT_1 SND_PCM_FORMAT_S24_3LE  // Formato de áudio para dispositivo 1 (24 bits, 3 bytes, little-endian)
#define FORMAT_2 SND_PCM_FORMAT_S16_LE  // Formato de áudio para dispositivo 2 (16 bits, little-endian)

typedef struct {
    const char *device;
    const char *filename;
    snd_pcm_format_t format;
    int channels;
} capture_params_t;

void convert_s24_to_s16(void *buffer, int frames, int channels);

void convert_s24_to_s16(void *buffer, int frames, int channels) {
    unsigned char *buf = (unsigned char *)buffer;
    int i, j;
    for (i = 0; i < frames; i++) {
        for (j = 0; j < channels; j++) {
            int index = (i * channels + j) * 3;  // Índice para 24 bits
            int value = (buf[index] << 16) | (buf[index + 1] << 8) | buf[index + 2];

            // Ajuste para 16 bits, truncando ou fazendo outro tipo de ajuste
            short *output = (short *)(buf + i * channels * 2 + j * 2);
            *output = (short)(value >> 8);  // Simples truncamento de 24 para 16 bits
        }
    }
}


void *capture_audio(void *arg) {
    capture_params_t *params = (capture_params_t *)arg;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *pcm_params;
    unsigned int rate = SAMPLE_RATE;
    snd_pcm_uframes_t buffer_size = 1024;  // Tamanho do buffer
    int periods = 2;  // Número de períodos
    void *buffer;  // Buffer para os dados capturados
    size_t frame_size;  // Tamanho de cada frame em bytes

    // Definir o tamanho do frame com base no formato
    if (params->format == SND_PCM_FORMAT_S24_3LE) {
        frame_size = 3;  // 24 bits, 3 bytes
    } else if (params->format == SND_PCM_FORMAT_S16_LE) {
        frame_size = 2;  // 16 bits, 2 bytes
    } else {
        fprintf(stderr, "Formato de áudio não suportado\n");
        return NULL;
    }

    // Abrir o dispositivo de captura
    printf("Abrindo o dispositivo de captura: %s...\n", params->device);
    if (snd_pcm_open(&pcm_handle, params->device, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo %s\n", params->device);
        return NULL;
    }
    printf("Dispositivo %s aberto com sucesso.\n", params->device);

    // Alocar memória para os parâmetros de hardware
    snd_pcm_hw_params_malloc(&pcm_params);
    if (!pcm_params) {
        fprintf(stderr, "Erro ao alocar memória para os parâmetros de hardware.\n");
        return NULL;
    }

    // Configurar parâmetros de hardware
    snd_pcm_hw_params_any(pcm_handle, pcm_params);
    if (snd_pcm_hw_params_set_access(pcm_handle, pcm_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        fprintf(stderr, "Erro ao configurar o acesso ao dispositivo de captura.\n");
        return NULL;
    }
    if (snd_pcm_hw_params_set_format(pcm_handle, pcm_params, params->format) < 0) {
        fprintf(stderr, "Erro ao configurar o formato de captura.\n");
        return NULL;
    }
    if (snd_pcm_hw_params_set_channels(pcm_handle, pcm_params, params->channels) < 0) {
        fprintf(stderr, "Erro ao configurar o número de canais.\n");
        return NULL;
    }
    if (snd_pcm_hw_params_set_rate_near(pcm_handle, pcm_params, &rate, 0) < 0) {
        fprintf(stderr, "Erro ao configurar a taxa de amostragem.\n");
        return NULL;
    }
    if (snd_pcm_hw_params_set_period_size_near(pcm_handle, pcm_params, &buffer_size, 0) < 0) {
        fprintf(stderr, "Erro ao configurar o tamanho do buffer.\n");
        return NULL;
    }
    if (snd_pcm_hw_params_set_periods_near(pcm_handle, pcm_params, &periods, 0) < 0) {
        fprintf(stderr, "Erro ao configurar o número de períodos.\n");
        return NULL;
    }

    // Aplicar os parâmetros ao dispositivo
    if (snd_pcm_hw_params(pcm_handle, pcm_params) < 0) {
        fprintf(stderr, "Erro ao aplicar os parâmetros de hardware.\n");
        return NULL;
    }
    printf("Parâmetros de hardware configurados com sucesso para o dispositivo %s.\n", params->device);

    // Liberar a memória alocada para os parâmetros
    snd_pcm_hw_params_free(pcm_params);

    // Alocar memória para o buffer de captura
    buffer = malloc(buffer_size * params->channels * frame_size);
    if (buffer == NULL) {
        perror("Erro ao alocar memória para o buffer");
        return NULL;
    }

    // Variáveis de controle de tempo
    time_t start_time = time(NULL);  // Tempo inicial

    printf("Iniciando captura de áudio no dispositivo %s...\n", params->device);

    // Abrir arquivo para gravação em formato binário
    FILE *bin_file = fopen(params->filename, "wb");
    if (!bin_file) {
        printf("Erro ao abrir o arquivo para escrita!\n");
        return NULL;
    }

    while (1) {
        // Verifica se o tempo de captura foi atingido
        if (difftime(time(NULL), start_time) >= CAPTURE_TIME) {
            printf("Tempo de captura de %d segundos alcançado no dispositivo %s. Finalizando...\n", CAPTURE_TIME, params->device);
            break;
        }

        // Captura o áudio em frames
        int frames = snd_pcm_readi(pcm_handle, buffer, buffer_size);
        if (frames < 0) {
            fprintf(stderr, "Erro de captura no dispositivo %s: %s\n", params->device, snd_strerror(frames));
            break;
        }

        // **Converter 24 bits para 16 bits (apenas para o dispositivo 1)**
        if (params->format == SND_PCM_FORMAT_S24_3LE) {
            convert_s24_to_s16(buffer, frames, params->channels);  // Aplica a conversão
        }

        // Escrever os dados capturados no arquivo binário
        fwrite(buffer, frame_size, frames * params->channels, bin_file);
    }

    // Fechar arquivo de captura
    fclose(bin_file);

    // Fechar o dispositivo
    snd_pcm_close(pcm_handle);
    free(buffer);

    printf("Captura de áudio finalizada e salva em %s\n", params->filename);

    return NULL;
}

int main() {
    pthread_t thread_1, thread_2;
    capture_params_t params_1 = {DEVICE_1, PCM_CAPTURE_FILE_1, FORMAT_1, CHANNELS_1};
    capture_params_t params_2 = {DEVICE_2, PCM_CAPTURE_FILE_2, FORMAT_2, CHANNELS_2};

    // Criar as threads para capturar os dados de áudio simultaneamente
    printf("Criando a thread para captura do dispositivo 1...\n");
    if (pthread_create(&thread_1, NULL, capture_audio, &params_1) != 0) {
        perror("Erro ao criar a thread para o dispositivo 1");
        return -1;
    }

    printf("Criando a thread para captura do dispositivo 2...\n");
    if (pthread_create(&thread_2, NULL, capture_audio, &params_2) != 0) {
        perror("Erro ao criar a thread para o dispositivo 2");
        return -1;
    }

    // Aguardar as threads terminarem
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    printf("Captura simultânea de áudio concluída.\n");

    return 0;
}
