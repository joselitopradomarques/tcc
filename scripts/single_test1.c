#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "filter_final.h"
#include <time.h>  // Para medir o tempo
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define PCM_DEVICE_1 "hw:3,0"  // Dispositivo 1 de captura
#define BUFFER_SIZE 16  // Ajuste conforme necessário
#define SAMPLE_RATE 44100  // Taxa de amostragem
#define N_BUFFERS_UPDATE 1000  // Atualizar as frequências de corte a cada N buffers
#define TEMPO_EXECUCAO 120  // Tempo de execução em segundos (2 minutos)


void processar_audio(short int* buffer, int size, float* coeficientes_filtro) {
    // Função onde o processamento de áudio será feito
    printf("Processando áudio...\n");

    // O buffer de áudio precisa ser tratado de acordo com o tipo de dado (por exemplo, short ou float)
    short* sinal_filtrado = (short*)malloc(size * sizeof(short));
    if (!sinal_filtrado) {
        printf("Erro ao alocar memória para o sinal filtrado\n");
        return;
    }

    // Aplicar o filtro FIR no buffer de áudio
    aplicar_filtro_FIR((short*)buffer, sinal_filtrado, size, coeficientes_filtro, ORDEM);
    printf("Função aplicar_filtro_FIR executada com sucesso.\n");  // Print de depuração

    // Copiar o áudio filtrado de volta para o buffer original
    for (int i = 0; i < size; i++) {
        buffer[i] = sinal_filtrado[i];
    }

    // Liberar memória
    free(sinal_filtrado);
}

void audio_callback(snd_pcm_t *pcm_handle_capture1, float* coeficientes_filtro1) {
    short buffer1[BUFFER_SIZE];
    int rc;
    clock_t start_time = clock();
    int contador_buffers = 0;
    double tempo_execucao = TEMPO_EXECUCAO * SAMPLE_RATE / BUFFER_SIZE;

    while (contador_buffers < tempo_execucao) {
        // Captura o áudio do primeiro dispositivo de entrada
        rc = snd_pcm_readi(pcm_handle_capture1, buffer1, BUFFER_SIZE);  
        if (rc == -EPIPE) {
            // Buffer de captura estourado, reconfigure
            snd_pcm_prepare(pcm_handle_capture1);
        } else if (rc < 0) {
            fprintf(stderr, "Erro ao capturar áudio do dispositivo 1: %s\n", snd_strerror(rc));
        }

        // Processa o áudio capturado do dispositivo 1 com o filtro FIR
        processar_audio(buffer1, BUFFER_SIZE, coeficientes_filtro1);

        // Incrementa o contador de buffers
        contador_buffers++;
    }
}


int main() {
    snd_pcm_t *pcm_handle_capture1;
    unsigned int sample_rate = SAMPLE_RATE;
    int dir;
    snd_pcm_hw_params_t *params_capture1;

    // Criar os coeficientes dos filtros FIR
    float coeficientes_filtro1[ORDEM];

    // Frequências de corte iniciais
    float frequencia_corte1 = 1000.0f;

    // Gerar os filtros FIR com as frequências de corte iniciais
    gerar_filtro_FIR(coeficientes_filtro1, ORDEM, frequencia_corte1, SAMPLE_RATE);

    printf("Filtros FIR gerados com sucesso.\n");

    // Abre dispositivo de captura
    if (snd_pcm_open(&pcm_handle_capture1, PCM_DEVICE_1, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de captura 1\n");
        return 1;
    } else {
        printf("Dispositivo de captura 1 aberto com sucesso.\n");
    }

    // Configuração do dispositivo de captura 1
    snd_pcm_hw_params_alloca(&params_capture1);

    if (params_capture1 == NULL) {
    fprintf(stderr, "Erro ao alocar memória para params_capture1\n");
    return 1;
    }

    snd_pcm_hw_params_any(pcm_handle_capture1, params_capture1);
    snd_pcm_hw_params_set_access(pcm_handle_capture1, params_capture1, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format(pcm_handle_capture1, params_capture1, SND_PCM_FORMAT_S16_LE);  // 16 bits por amostra
    snd_pcm_hw_params_set_channels(pcm_handle_capture1, params_capture1, 2);  // Estéreo
    if (snd_pcm_hw_params(pcm_handle_capture1, params_capture1) < 0) {
        fprintf(stderr, "Erro ao configurar o dispositivo de captura 1\n");
        return 1;
    }

    // Inicia o callback de áudio
    audio_callback(pcm_handle_capture1, coeficientes_filtro1);

    // Fecha o dispositivo de captura

int err;
if ((err = snd_pcm_close(pcm_handle_capture1)) < 0) {
    fprintf(stderr, "Erro ao fechar o dispositivo de captura 1: %s\n", snd_strerror(err));
    return 1;
}

    printf("Processamento de áudio concluído com sucesso.\n");

    return 0;
}
