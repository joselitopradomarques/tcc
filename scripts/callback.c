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
#define PCM_DEVICE_2 "hw:4,0"  // Dispositivo 2 de captura (ajuste conforme necessário)
#define BUFFER_SIZE 512  // Ajuste conforme necessário
#define SAMPLE_RATE 44100  // Taxa de amostragem
#define I2C_DEVICE "/dev/i2c-1"  // Caminho do dispositivo I2C (ajuste conforme necessário)
#define PCF8591_ADDRESS 0x48 // Endereço I2C do PCF8591 (verifique com o i2cdetect)
#define CHANNEL_A0 0x48  // Canal A0 (entrada analógica 0)
#define CHANNEL_A1 0x49  // Canal A1 (entrada analógica 1)
#define N_BUFFERS_UPDATE 100  // Atualizar as frequências de corte a cada N buffers

int abrir_i2c() {
    int file = open(I2C_DEVICE, O_RDWR);
    if (file < 0) {
        perror("Erro ao abrir o barramento I2C");
        exit(1);
    }
    return file;
}


int ler_valor_i2c(int file, unsigned char channel) {
    unsigned char config;

    // Definir o canal de entrada (A0 ou A1) no PCF8591
    if (channel == CHANNEL_A0) {
        config = 0x40;  // Configuração para canal A0
    } else if (channel == CHANNEL_A1) {
        config = 0x41;  // Configuração para canal A1
    } else {
        fprintf(stderr, "Canal inválido\n");
        return -1;
    }

    // Define o endereço do dispositivo I2C (PCF8591)
    if (ioctl(file, I2C_SLAVE, PCF8591_ADDRESS) < 0) {
        perror("Erro ao selecionar o dispositivo I2C");
        exit(1);
    }

    // Enviar comando para selecionar o canal
    if (write(file, &config, 1) != 1) {
        perror("Erro ao escrever no dispositivo I2C");
        exit(1);
    }

    // Ler o valor (duas leituras necessárias: a primeira é lixo)
    unsigned char buffer[1];
    
    // Leitura "lixo"
    if (read(file, buffer, 1) != 1) {
        perror("Erro ao ler valor de conversão inicial (lixo)");
        exit(1);
    }

    // Leitura válida
    if (read(file, buffer, 1) != 1) {
        perror("Erro ao ler valor de conversão válida");
        exit(1);
    }

    // O valor retornado é de 8 bits
    return (int)buffer[0];  // Retornar o valor lido (0 a 255)
}


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

    // Liberar memória
    free(sinal_filtrado);
}

void audio_callback(snd_pcm_t *pcm_handle_capture1, snd_pcm_t *pcm_handle_capture2, snd_pcm_t *pcm_handle_playback, float* coeficientes_filtro1, float* coeficientes_filtro2) {
    short buffer1[BUFFER_SIZE];
    short buffer2[BUFFER_SIZE];

    int rc;

    // Abrir o dispositivo I2C
    int file = abrir_i2c();

    // Contador de buffers processados
    int contador_buffers = 0;

    while (1) {
        clock_t start_time = clock();  // Captura o tempo antes do processamento

        // A cada N buffers, atualiza as frequências de corte
        if (contador_buffers % N_BUFFERS_UPDATE == 0) {
            // Lê os valores das frequências de corte do dispositivo I2C
            int frequencia_corte1_valor = ler_valor_i2c(file, CHANNEL_A0);
            int frequencia_corte2_valor = ler_valor_i2c(file, CHANNEL_A1);
            printf("Valor de corte1: %d, Valor de corte2: %d\n", frequencia_corte1_valor, frequencia_corte2_valor);
            // Converte os valores lidos para frequências de corte (ajuste conforme necessário)
            float frequencia_corte1 = frequencia_corte1_valor * 1000.0f / 255.0f;  // Exemplo de conversão
            float frequencia_corte2 = frequencia_corte2_valor * 1000.0f / 255.0f;  // Exemplo de conversão

            // Atualiza os coeficientes dos filtros com as novas frequências de corte
            gerar_filtro_FIR(coeficientes_filtro1, ORDEM, frequencia_corte1, SAMPLE_RATE);
            gerar_filtro_FIR(coeficientes_filtro2, ORDEM, frequencia_corte2, SAMPLE_RATE);
        }

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

    close(file);  // Fecha o dispositivo I2C
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

    // Criar os coeficientes dos dois filtros FIR
    float coeficientes_filtro1[ORDEM];
    float coeficientes_filtro2[ORDEM];

    // Frequências de corte iniciais
    float frequencia_corte1 = 1000.0f;
    float frequencia_corte2 = 2000.0f;

    // Gerar os filtros FIR com as frequências de corte iniciais
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

    // Configuração do dispositivo de captura 2
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

    // Inicia o callback de áudio
    audio_callback(pcm_handle_capture1, pcm_handle_capture2, pcm_handle_playback, coeficientes_filtro1, coeficientes_filtro2);

    return 0;
}
