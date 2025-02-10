#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // Necessário para usar a função sin() e PI
#include "proc.h" // Responsável pela integração das funcionalidades
#include "reverb.h" // Responsável pelas funções de reverb
#include "delay.h" // Responsável pelas funções de delay
#include "audio.h" // Responsável pela interface de reprodução de áudio
#include "filt.h" // Responsável pelas frequências de corte e coeficientes dos filtros

// Função para aplicar o filtro FIR em cada buffer circular
void aplicar_filtro_FIR_buffer(short *buffer_sinal, short *buffer_sinal_filtrado, int buffer_size, float *coeficientes, int ordem) {
    // Aplicar o filtro FIR em um único buffer
    for (int j = 0; j < buffer_size; j++) {
        float acumulador = 0.0;
        // Aplicar FIR para cada amostra no buffer
        for (int k = 0; k < ordem; k++) {
            if (j - k >= 0) {
                acumulador += coeficientes[k] * buffer_sinal[j - k];
            }
        }
        // Limitar a amplitude para valores de 16 bits
        if (acumulador > MAX_16BIT) acumulador = MAX_16BIT;
        if (acumulador < -MAX_16BIT) acumulador = -MAX_16BIT;
        buffer_sinal_filtrado[j] = (short)acumulador;
    }
}

int ler_wav_estereo(const char *filename, short **sinal, int *tamanho) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV: %s\n", filename);
        return -1;
    }

    // Ler cabeçalho WAV (34 primeiros bytes de interesse)
    fseek(file, 22, SEEK_SET);  // Pular até o número de canais
    short num_channels;
    fread(&num_channels, sizeof(short), 1, file);

    fseek(file, 24, SEEK_SET);  // Pular até a taxa de amostragem
    int sample_rate;
    fread(&sample_rate, sizeof(int), 1, file);

    fseek(file, 34, SEEK_SET);  // Pular até os bits por amostra
    short bits_per_sample;
    fread(&bits_per_sample, sizeof(short), 1, file);

    if (num_channels != 2 || bits_per_sample != 16) {
        printf("Erro: o arquivo WAV não é estéreo ou não possui 16 bits por amostra.\n");
        fclose(file);
        return -1;
    }

    // Saltar para a parte de dados do arquivo WAV (pular o cabeçalho até a posição 44)
    fseek(file, 44, SEEK_SET);

    // Descobrir o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 44, SEEK_SET);  // Volta para a posição onde os dados de áudio começam

    // Calcular o número de amostras considerando que é estéreo (2 canais)
    int num_samples = (file_size - 44) / (sizeof(short) * num_channels);  // 2 canais
    *tamanho = num_samples;

    // Alocar memória para o sinal estéreo (2 canais)
    *sinal = (short *)malloc(num_samples * num_channels * sizeof(short));  
    if (!*sinal) {
        printf("Erro ao alocar memória para o sinal estéreo\n");
        fclose(file);
        return -1;
    }

    // Ler os dados do sinal estéreo (2 canais)
    fread(*sinal, sizeof(short), num_samples * num_channels, file);
    fclose(file);

    return 0;
}

int ler_dois_wav_estereo(short **sinal1, short **sinal2, int *tamanho1, int *tamanho2) {
    // Definindo os caminhos dos arquivos WAV diretamente dentro da função
    const char *filename1 = "/home/joselito/git/tcc/scripts/song12.wav";
    const char *filename2 = "/home/joselito/git/tcc/scripts/silencio_6minutos.wav";

    // Ler o primeiro arquivo WAV
    short *sinal_temp1 = NULL;
    if (ler_wav_estereo(filename1, &sinal_temp1, tamanho1) != 0) {
        return -1; // Erro ao ler o primeiro arquivo
    }

    // Ler o segundo arquivo WAV
    short *sinal_temp2 = NULL;
    if (ler_wav_estereo(filename2, &sinal_temp2, tamanho2) != 0) {
        // Se ocorrer erro ao ler o segundo arquivo, liberar a memória do primeiro arquivo
        free(sinal_temp1);
        return -1; // Erro ao ler o segundo arquivo
    }

    // Verificar se as propriedades dos dois arquivos são compatíveis
    if (*tamanho1 != *tamanho2) {
        // Ajuste os sinais para o tamanho do menor
        int min_tamanho = (*tamanho1 < *tamanho2) ? *tamanho1 : *tamanho2;
        *tamanho1 = min_tamanho;
        *tamanho2 = min_tamanho;

        // Redimensionar os sinais para o tamanho compatível
        sinal_temp1 = (short *)realloc(sinal_temp1, min_tamanho * 2 * sizeof(short));  // 2 canais
        sinal_temp2 = (short *)realloc(sinal_temp2, min_tamanho * 2 * sizeof(short));  // 2 canais
    }

    // Se tudo estiver correto, copiar os dados de áudio para os arrays finais
    *sinal1 = sinal_temp1;
    *sinal2 = sinal_temp2;

    return 0; // Sucesso na leitura dos dois arquivos
}

// Função para gerar buffers a partir dos sinais de áudio sinal1 e sinal2
// Os sinais são divididos em buffers de tamanho definido pelo usuário
int gerar_buffers_circulares(short *sinal1, short *sinal2, int tamanho, int buffer_size, short ***buffers_sinal1, short ***buffers_sinal2, int *num_buffers) {
    *num_buffers = (tamanho + buffer_size - 1) / buffer_size;  // Calculando o número de buffers necessários

    // Alocando memória para os buffers circulares
    *buffers_sinal1 = (short **)malloc(*num_buffers * sizeof(short *));
    *buffers_sinal2 = (short **)malloc(*num_buffers * sizeof(short *));
    if (*buffers_sinal1 == NULL || *buffers_sinal2 == NULL) {
        printf("Erro ao alocar memória para os buffers.\n");
        return -1;  // Erro de alocação
    }

    // Alocar e inicializar os buffers circulares
    for (int i = 0; i < *num_buffers; i++) {
        (*buffers_sinal1)[i] = (short *)malloc(buffer_size * sizeof(short));
        (*buffers_sinal2)[i] = (short *)malloc(buffer_size * sizeof(short));

        if ((*buffers_sinal1)[i] == NULL || (*buffers_sinal2)[i] == NULL) {
            printf("Erro ao alocar memória para os buffers individuais.\n");
            return -1;  // Erro de alocação
        }
    }

    // Preencher os buffers com os sinais (acesso circular)
    for (int i = 0; i < *num_buffers; i++) {
        for (int j = 0; j < buffer_size; j++) {
            int index = (i * buffer_size + j) % tamanho;  // Acesso circular
            (*buffers_sinal1)[i][j] = sinal1[index];
            (*buffers_sinal2)[i][j] = sinal2[index];
        }
    }

    return 0;  // Sucesso
}

// Função filtro_exemplo
void filtro_exemplo(short *buffer, int buffer_size) {
    for (int i = 1; i < buffer_size - 1; i++) {
        buffer[i] = (buffer[i - 1] + buffer[i] + buffer[i + 1]) / 3;
    }
}


// Função para salvar o arquivo WAV com o sinal filtrado em estéreo
int escrever_wav_estereo(const char *filename, short *sinal, int tamanho) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para escrita: %s\n", filename);
        return -1;
    }

    // Escrever cabeçalho WAV simples (44 bytes)
    unsigned int chunk_size = 36 + tamanho * 2 * sizeof(short);  // 2 canais
    unsigned short audio_format = 1; // PCM
    unsigned short num_channels = 2; // Estéreo
    unsigned int sample_rate = SAMPLE_RATE;
    unsigned int byte_rate = SAMPLE_RATE * 2 * sizeof(short);  // 2 canais
    unsigned short block_align = 2 * sizeof(short);  // 2 canais
    unsigned short bits_per_sample = 16;

    // Cabeçalho RIFF
    fwrite("RIFF", sizeof(char), 4, file);
    fwrite(&chunk_size, sizeof(unsigned int), 1, file);
    fwrite("WAVE", sizeof(char), 4, file);

    // Sub-chunk 1 "fmt "
    fwrite("fmt ", sizeof(char), 4, file);
    unsigned int sub_chunk1_size = 16;
    fwrite(&sub_chunk1_size, sizeof(unsigned int), 1, file);
    fwrite(&audio_format, sizeof(unsigned short), 1, file);
    fwrite(&num_channels, sizeof(unsigned short), 1, file);
    fwrite(&sample_rate, sizeof(unsigned int), 1, file);
    fwrite(&byte_rate, sizeof(unsigned int), 1, file);
    fwrite(&block_align, sizeof(unsigned short), 1, file);
    fwrite(&bits_per_sample, sizeof(unsigned short), 1, file);

    // Sub-chunk 2 "data"
    unsigned int sub_chunk2_size = tamanho * 2 * sizeof(short);  // 2 canais
    fwrite("data", sizeof(char), 4, file);
    fwrite(&sub_chunk2_size, sizeof(unsigned int), 1, file);
    // Escrever o sinal filtrado em estéreo
    fwrite(sinal, sizeof(short), tamanho * 2, file);

    fclose(file);
    return 0;
}

// Função para processar os buffers de sinal1 e sinal2
int processar_buffers_circulares(short ***buffers_sinal1, short ***buffers_sinal2, int num_buffers, int buffer_size, float *coeficientes_filtro, int ordem_filtro) {
    
    const char *filename = "sinal_processado.wav";
    const char *device = "hw:2,0"; // Dispositivo ALSA para os fones de ouvido
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
    char *audio_buffer;
    
    // Definição para efeito Reverb
    float wetness = 0.0f; // Defina o valor apropriado para o efeito

    // Declaração de arrays para coeficientes e frequências de corte
    float frequencias_log[N_FREQUENCIES];
    float matriz_coeficientes[N_FREQUENCIES][ORDER];

    // Gerar as frequências logarítmicas e os coeficientes
    gerar_pontos_logaritmicos(frequencias_log);
    gerar_matriz_coeficientes(matriz_coeficientes, frequencias_log);

    // Inicializa o dispositivo de áudio
    if (inicializar(device, &pcm_handle, &audio_buffer, &hw_params) != 0) {
        printf("Erro ao inicializar o stream de áudio\n");
        return -1;  // Retorna erro caso a inicialização falhe
    }

    // Verificar se os buffers estão alocados corretamente
    if (!buffers_sinal1 || !buffers_sinal2) {
        printf("Erro: buffers não alocados corretamente.\n");
        return -1;  // Retorna erro
    }

    // Alocar buffers filtrados
    short **buffers_sinal1_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    if (!buffers_sinal1_filtrado || !buffers_sinal2_filtrado) {
        printf("Erro: falha ao alocar buffers filtrados.\n");
        return -1;  // Retorna erro
    }

    // Inicializar buffers filtrados
    for (int i = 0; i < num_buffers; i++) {
        buffers_sinal1_filtrado[i] = (short *)malloc(buffer_size * sizeof(short));
        buffers_sinal2_filtrado[i] = (short *)malloc(buffer_size * sizeof(short));
        if (!buffers_sinal1_filtrado[i] || !buffers_sinal2_filtrado[i]) {
            printf("Erro: falha ao alocar memória para buffer de sinal.\n");
            return -1;  // Retorna erro
        }
    }

    // Alocar array para o sinal completo (todos os buffers combinados)
    int tamanho_total_sinal = num_buffers * buffer_size;  // Total de amostras no sinal final
    short *sinal_completo = (short *)malloc(tamanho_total_sinal * sizeof(short));
    if (!sinal_completo) {
        printf("Erro: falha ao alocar o array do sinal completo.\n");
        return -1;  // Retorna erro
    }

    // Processar os buffers, aplicar o filtro FIR e o delay
    int posicao = 0;  // Variável para controle da posição no sinal_completo
    for (int i = 0; i < num_buffers; i++) {
        // Atualizar coeficientes a cada 10 buffers (se necessário)
        if (i % 10 == 0) {
            // altere_coeficientes(coeficientes_filtro, nova_ordem_filtro);
        }

        // Aplicar o filtro FIR para o sinal1
        if ((*buffers_sinal1)[i] != NULL) {
            aplicar_filtro_FIR_buffer((*buffers_sinal1)[i], buffers_sinal1_filtrado[i], buffer_size, coeficientes_filtro, ordem_filtro);
        }

        // Aplicar o filtro FIR para o sinal2
        if ((*buffers_sinal2)[i] != NULL) {
            aplicar_filtro_FIR_buffer((*buffers_sinal2)[i], buffers_sinal2_filtrado[i], buffer_size, coeficientes_filtro, ordem_filtro);
        }

        // Calcular a média dos buffers filtrados e preencher media_buffers
        float *media_buffer = (float *)malloc(buffer_size * sizeof(float));  // Alocar buffer temporário para a média do buffer atual
        for (int j = 0; j < buffer_size; j++) {
            media_buffer[j] = (float)(buffers_sinal1_filtrado[i][j] + buffers_sinal2_filtrado[i][j]) / 2.0f;
        }

        // Aplicar o delay no buffer de média (feedback = 0.6f)
        //aplicar_delay(media_buffer, buffer_size, wetness, 0.3f);
        applyReverbEffectBuffer(media_buffer, buffer_size, wetness, 0.6f);

        // Normalizar o buffer de média para o intervalo -1.0 a 1.0
        float max_value = 0.0f;
        for (int j = 0; j < buffer_size; j++) {
            // Encontrar o valor máximo absoluto no buffer
            if (fabs(media_buffer[j]) > max_value) {
                max_value = fabs(media_buffer[j]);
            }
        }

        // Se o valor máximo for maior que 1.0, normaliza os valores
        if (max_value > 1.0f) {
            float normalizing_factor = 1.0f / max_value;
            for (int j = 0; j < buffer_size; j++) {
                media_buffer[j] *= normalizing_factor;
            }
        }

        // Conversão de float para short para reprodução com ALSA
        short *buffer_reproduzivel = (short *)malloc(buffer_size * sizeof(short));
        for (int j = 0; j < buffer_size; j++) {
            // Converte cada valor float para short (PCM de 16 bits)
            // Multiplicando por 32767.0f para mapear o intervalo de -1.0 a 1.0 para o intervalo de -32767 a 32767
            buffer_reproduzivel[j] = (short)(media_buffer[j] * 32767.0f);  // 32767 é o valor máximo para PCM de 16 bits
        }

        // Reproduzir o buffer processado
        if (reproduzir(pcm_handle, (char *)buffer_reproduzivel, buffer_size * sizeof(short)) != 1) {
            printf("Erro ao reproduzir áudio\n");
            free(buffer_reproduzivel);
            return -1;  // Retorna erro se falhar na reprodução
        }

        // Copiar o buffer processado para o sinal completo
        for (int j = 0; j < buffer_size; j++) {
            sinal_completo[posicao++] = (short)media_buffer[j];  // Converter de volta para short antes de adicionar ao sinal completo
        }

        // Liberar memória do buffer de média após o uso
        free(media_buffer);
    }

    // Salvar o sinal completo no arquivo WAV
    if (escrever_wav_estereo(filename, sinal_completo, tamanho_total_sinal) != 0) {
        printf("Erro ao salvar o arquivo WAV.\n");
        return -1;  // Retorna erro
    }

    // Liberação dos buffers filtrados e sinal completo
    for (int i = 0; i < num_buffers; i++) {
        free(buffers_sinal1_filtrado[i]);
        free(buffers_sinal2_filtrado[i]);
    }
    free(buffers_sinal1_filtrado);
    free(buffers_sinal2_filtrado);
    free(sinal_completo);

    return 0;  // Retorna sucesso
}

void liberar_buffers(short **buffers_sinal1, short **buffers_sinal2, int num_buffers) {
    // Verifica e libera buffers_sinal1
    if (buffers_sinal1 != NULL) {
        for (int i = 0; i < num_buffers; i++) {
            // Liberar cada buffer de sinal1 (que é um ponteiro para short)
            if (buffers_sinal1[i] != NULL) {
                free(buffers_sinal1[i]);
            }
        }
        // Liberar o array de ponteiros buffers_sinal1
        free(buffers_sinal1);
    }

    // Verifica e libera buffers_sinal2
    if (buffers_sinal2 != NULL) {
        for (int i = 0; i < num_buffers; i++) {
            // Liberar cada buffer de sinal2 (que é um ponteiro para short)
            if (buffers_sinal2[i] != NULL) {
                free(buffers_sinal2[i]);
            }
        }
        // Liberar o array de ponteiros buffers_sinal2
        free(buffers_sinal2);
    }
}

// Função para gerar coeficientes do filtro FIR
void gerar_filtro_FIR(float *coeficientes, int ordem, float corte, float taxa_amostragem) {
    int n = ordem;
    float wc = 2 * PI * corte / taxa_amostragem;  // Frequência de corte normalizada
    for (int i = 0; i < n; i++) {
        if (i == (n - 1) / 2) {
            coeficientes[i] = 1 - (wc / PI); 
        } else {
            coeficientes[i] = -sin(wc * (i - (n - 1) / 2)) / (PI * (i - (n - 1) / 2));  // Inverte a fase
        }
    }
}

