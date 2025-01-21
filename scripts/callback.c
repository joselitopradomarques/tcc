#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "filter_final.h"
#include <time.h>  // Para medir o tempo

#define PCM_DEVICE_1 "hw:1,0"  // Dispositivo 1 de captura
#define PCM_DEVICE_2 "hw:2,0"  // Dispositivo 2 de captura (ajuste conforme necessário)
#define BUFFER_SIZE 1024  // Ajuste conforme necessário
#define SAMPLE_RATE 44100  // Taxa de amostragem
#define N_BUFFERS_PARA_ATUALIZAR 10  // Número de buffers após os quais a frequência de corte será atualizada

void processar_audio(char* buffer, int size, float* coeficientes_filtro) {
    // Função onde o processamento de áudio será feito
    printf("Processando áudio...\n");

    // O buffer de áudio precisa ser tratado de acordo com o tipo de dado (por exemplo, short ou float)
    // Aqui assumimos que o buffer é um array de shorts (16 bits)
    short* sinal_filtrado = (short*)malloc(size * sizeof(short));
    if (!sinal_filtrado) {
        printf("Erro ao alocar memória para o sinal filtrado\n");
        return;
    }

    // Aplicar o filtro FIR no buffer de áudio
    aplicar_filtro_FIR((short*)buffer, sinal_filtrado, size, coeficientes_filtro, ORDEM);
    printf("Função aplicar_filtro_FIR executada com sucesso.\n");  // Print de depuração

    // Liberar memória
    free(sinal_filtrado);
}

void audio_callback(snd_pcm_t *pcm_handle_capture1, snd_pcm_t *pcm_handle_capture2, snd_pcm_t *pcm_handle_playback, float* coeficientes_filtro1, float* coeficientes_filtro2) {
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    int rc;
    int contador_buffers = 0;  // Inicializa o contador de buffers

    while (1) {
        clock_t start_time = clock();  // Captura o tempo antes do processamento

        // Captura o áudio do primeiro dispositivo de entrada
        rc = snd_pcm_readi(pcm_handle_capture1, buffer1, BUFFER_SIZE);
        if (rc == -EPIPE) {
            // Buffer de captura estourado, reconfigure
            snd_pcm_prepare(pcm_handle_capture1);
        } else if (rc < 0) {
            fprintf(stderr, "Erro ao capturar áudio do dispositivo 1: %s\n", snd_strerror(rc));
        }

        // Captura o áudio do segundo dispositivo de entrada
        rc = snd_pcm_readi(pcm_handle_capture2, buffer2, BUFFER_SIZE);
        if (rc == -EPIPE) {
            // Buffer de captura estourado, reconfigure
            snd_pcm_prepare(pcm_handle_capture2);
        } else if (rc < 0) {
            fprintf(stderr, "Erro ao capturar áudio do dispositivo 2: %s\n", snd_strerror(rc));
        }

        // Atualiza a frequência de corte a cada N buffers
        if (contador_buffers >= N_BUFFERS_PARA_ATUALIZAR) {
            // Atualiza as frequências de corte via I2C
            // Aqui você deveria chamar uma função que faz a leitura via I2C, por exemplo:
            float nova_frequencia_corte1 = ler_frequencia_corte_I2C(1);  // Função fictícia que lê a frequência de corte do primeiro dispositivo
            float nova_frequencia_corte2 = ler_frequencia_corte_I2C(2);  // Função fictícia que lê a frequência de corte do segundo dispositivo

            // Atualiza os coeficientes do filtro com os novos valores de frequência de corte
            gerar_filtro_FIR(coeficientes_filtro1, ORDEM, nova_frequencia_corte1, SAMPLE_RATE);
            gerar_filtro_FIR(coeficientes_filtro2, ORDEM, nova_frequencia_corte2, SAMPLE_RATE);

            printf("Frequências de corte atualizadas: %.2f Hz, %.2f Hz\n", nova_frequencia_corte1, nova_frequencia_corte2);

            // Reseta o contador de buffers
            contador_buffers = 0;
        }

        // Processa o áudio capturado do dispositivo 1 com o primeiro filtro FIR
        processar_audio(buffer1, BUFFER_SIZE, coeficientes_filtro1);

        // Processa o áudio capturado do dispositivo 2 com o segundo filtro FIR
        processar_audio(buffer2, BUFFER_SIZE, coeficientes_filtro2);

        // Após o processamento, os buffers contêm o áudio já filtrado ou modificado.
        // Agora, podemos enviar estes buffers para reprodução
        rc = snd_pcm_writei(pcm_handle_playback, buffer1, BUFFER_SIZE);
        if (rc == -EPIPE) {
            snd_pcm_prepare(pcm_handle_playback);
        } else if (rc < 0) {
            fprintf(stderr, "Erro ao reproduzir áudio do buffer 1: %s\n", snd_strerror(rc));
        }

        rc = snd_pcm_writei(pcm_handle_playback, buffer2, BUFFER_SIZE);
        if (rc == -EPIPE) {
            snd_pcm_prepare(pcm_handle_playback);
        } else if (rc < 0) {
            fprintf(stderr, "Erro ao reproduzir áudio do buffer 2: %s\n", snd_strerror(rc));
        }

        clock_t end_time = clock();  // Captura o tempo após o processamento

        // Calcula e imprime o tempo de processamento dos buffers
        double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Tempo de processamento dos buffers: %.6f segundos\n", time_taken);

        // Incrementa o contador de buffers
        contador_buffers++;
    }
}

int main() {
    snd_pcm_t *pcm_handle_capture1;
    snd_pcm_t *pcm_handle_capture2;
    snd_pcm_t *pcm_handle_playback;
    unsigned int sample_rate = SAMPLE_RATE;
    int dir;
    snd_pcm_hw_params_t *params_capture1;
    snd_pcm_hw_params_t *params_capture2;
    snd_pcm_hw_params_t *params_playback;

    // Frequência de corte inicial
    float frequencia_corte1 = 1000.0f;  // Exemplo de frequência de corte para o primeiro dispositivo
    float frequencia_corte2 = 2000.0f;  // Exemplo de frequência de corte para o segundo dispositivo

    // Criar os coeficientes dos dois filtros FIR
    float coeficientes_filtro1[ORDEM];
    float coeficientes_filtro2[ORDEM];

    // Gerar os filtros FIR com as frequências de corte correspondentes
    gerar_filtro_FIR(coeficientes_filtro1, ORDEM, frequencia_corte1, SAMPLE_RATE);
    gerar_filtro_FIR(coeficientes_filtro2, ORDEM, frequencia_corte2, SAMPLE_RATE);

    printf("Filtros FIR gerados com sucesso.\n");

    // Abre dispositivos de captura
    if (snd_pcm_open(&pcm_handle_capture1, PCM_DEVICE_1, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de captura 1\n");
        return 1;
    }

    if (snd_pcm_open(&pcm_handle_capture2, PCM_DEVICE_2, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de captura 2\n");
        return 1;
    }

    // Abre dispositivo de reprodução
    if (snd_pcm_open(&pcm_handle_playback, PCM_DEVICE_1, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de reprodução\n");
        return 1;
    }

    // Configuração do dispositivo de captura 1
    snd_pcm_hw_params_alloca(&params_capture1);
    snd_pcm_hw_params_any(pcm_handle_capture1, params_capture1);
    snd_pcm_hw_params_set_access(pcm_handle_capture1, params_capture1, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle_capture1, params_capture1, SND_PCM_FORMAT_S16_LE);  // 16 bits por amostra
    snd_pcm_hw_params_set_channels(pcm_handle_capture1, params_capture1, 2);  // Estéreo
    snd_pcm_hw_params_set_rate_near(pcm_handle_capture1, params_capture1, &sample_rate, &dir);  // Taxa de 44100 Hz
    if (snd_pcm_hw_params(pcm_handle_capture1, params_capture1) < 0) {
        fprintf(stderr, "Erro ao configurar o dispositivo de captura 1\n");
        return 1;
    }

    // Configuração do dispositivo de captura 2 (semelhante)
    snd_pcm_hw_params_alloca(&params_capture2);
    snd_pcm_hw_params_any(pcm_handle_capture2, params_capture2);
    snd_pcm_hw_params_set_access(pcm_handle_capture2, params_capture2, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle_capture2, params_capture2, SND_PCM_FORMAT_S16_LE);  // 16 bits por amostra
    snd_pcm_hw_params_set_channels(pcm_handle_capture2, params_capture2, 2);  // Estéreo
    snd_pcm_hw_params_set_rate_near(pcm_handle_capture2, params_capture2, &sample_rate, &dir);  // Taxa de 44100 Hz
    if (snd_pcm_hw_params(pcm_handle_capture2, params_capture2) < 0) {
        fprintf(stderr, "Erro ao configurar o dispositivo de captura 2\n");
        return 1;
    }

    // Configuração do dispositivo de reprodução
    snd_pcm_hw_params_alloca(&params_playback);
    snd_pcm_hw_params_any(pcm_handle_playback, params_playback);
    snd_pcm_hw_params_set_access(pcm_handle_playback, params_playback, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle_playback, params_playback, SND_PCM_FORMAT_S16_LE);  // 16 bits por amostra
    snd_pcm_hw_params_set_channels(pcm_handle_playback, params_playback, 2);  // Estéreo
    snd_pcm_hw_params_set_rate_near(pcm_handle_playback, params_playback, &sample_rate, &dir);  // Taxa de 44100 Hz
    if (snd_pcm_hw_params(pcm_handle_playback, params_playback) < 0) {
        fprintf(stderr, "Erro ao configurar o dispositivo de reprodução\n");
        return 1;
    }

    // Chama a função de callback de áudio
    audio_callback(pcm_handle_capture1, pcm_handle_capture2, pcm_handle_playback, coeficientes_filtro1, coeficientes_filtro2);

    // Fecha os dispositivos de captura e reprodução
    snd_pcm_close(pcm_handle_capture1);
    snd_pcm_close(pcm_handle_capture2);
    snd_pcm_close(pcm_handle_playback);

    return 0;
}
