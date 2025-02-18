#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // Necessário para usar a função sin() e PI
#include "proc.h" // Responsável pela integração das funcionalidades
#include "reverb.h" // Responsável pelas funções de reverb
#include "delay.h" // Responsável pelas funções de delay
#include "audio.h" // Responsável pela interface de reprodução de áudio
#include "filt.h" // Responsável pelas frequências de corte e coeficientes dos filtros
#include "adc.h" // Responsável pela aquisição dos sinais de controle

// Função para aplicar o filtro FIR em cada buffer circular
void aplicar_filtro_FIR_buffer(short *buffer_sinal, short *buffer_sinal_filtrado, int buffer_size, float *coeficientes, int ordem) {

    int M = ordem / 2;  // Índice do meio da janela
    short buffer_temp[ordem];  // Definindo buffer_temp como short

    // Inicializa o buffer com zeros
    for (int j = 0; j < ordem; j++) {
        buffer_temp[j] = 0.0;
    }

    // Aplicar o filtro FIR em um único buffer
    for (int j = 0; j < buffer_size; j++) {
        float acumulador = 0.0;
        // Aplicar FIR para cada amostra no buffer
        for (int k = 0; k < ordem; k++) {
            int indice_buffer_sig = j + (k - M);  // Desloca a janela para o centro

            // Caso 1: Se o índice for menor que 0 (antes do início do buffer)
            if (indice_buffer_sig < 0) {
                // Refletir para o limite inferior (espelhando em relação ao índice 0)
                indice_buffer_sig = -indice_buffer_sig;  
            } 
            // Caso 2: Se o índice for maior ou igual ao tamanho do buffer (depois do final do buffer)
            else if (indice_buffer_sig >= buffer_size) {
                // Refletir para o limite superior (espelhando em relação ao último índice válido)
                indice_buffer_sig = 2 * (buffer_size - 1) - indice_buffer_sig;  
            }

            // Atribui os valores do buffer com reflexão no caso de valores fora do limite
            buffer_temp[k] = buffer_sinal[indice_buffer_sig];

            // Calcular a soma ponderada
            acumulador += coeficientes[k] * buffer_temp[k];
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
    *tamanho = num_samples*2;

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
    const char *filename1 = "/home/joselito/git/tcc/scripts/song02_b.wav";
    const char *filename2 = "/home/joselito/git/tcc/scripts/song12.wav";


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
    printf("Tamanho da matriz buffers_sinal1: %d x %d\n", *num_buffers, buffer_size);

    return 0;  // Sucesso
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
int processar_buffers_circulares(short ***buffers_sinal1, short ***buffers_sinal2, int num_buffers, int buffer_size) {
    
    const char *filename = "sinal_processado.wav";
    const char *device = "hw:2,0"; // Dispositivo ALSA para os fones de ouvido
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
  
    // Definição dos valores de controle
    int fd, analogValue0, analogValue1, digitalValue;

    // Definição para efeito Reverb
    float wetness = 0.0f; // Defina o valor apropriado para o efeito

    // Declaração de arrays para coeficientes e frequências de corte
    float frequencias_log[N_FREQUENCIES];
    float matriz_coeficientes[N_FREQUENCIES][ORDER];

    // Gerar as frequências logarítmicas e os coeficientes
    gerar_pontos_logaritmicos(frequencias_log);
    gerar_matriz_coeficientes(matriz_coeficientes, frequencias_log);

    // Inicializa o dispositivo de áudio
    if (inicializar(device, &pcm_handle, &hw_params) != 0) {
        printf("Erro ao inicializar o stream de áudio\n");
        return -1;  // Retorna erro caso a inicialização falhe
    }

    // Verificar se os buffers estão alocados corretamente
    if (!buffers_sinal1 || !buffers_sinal2) {
        printf("Erro: buffers não alocados corretamente.\n");
        return -1;  // Retorna erro
    }

    // Teste 1: arquivo original
    /*
        int posicao1 = 0;  // Variável para controle da posição no sinal_completo
            int tamanho_total_sinal1 = num_buffers * buffer_size;  // Total de amostras no sinal final
            short *sinal_completo1 = (short *)malloc(tamanho_total_sinal1 * sizeof(short));

            if (sinal_completo1 == NULL) {
                printf("Erro ao alocar memória para o sinal completo.\n");
                return -1;  // Erro de alocação
            }

            // Reconstruir o sinal completo a partir dos buffers circulares
            for (int i = 0; i < num_buffers; i++) {
                for (int j = 0; j < buffer_size; j++) {
                    // Copiar os dados do buffer para o sinal completo
                    sinal_completo1[posicao1++] = (*buffers_sinal1)[i][j];
                }
            }

            // Salvar o sinal completo no arquivo WAV
            if (escrever_wav_estereo("sinal_etapa1.wav", sinal_completo1, tamanho_total_sinal1 / 2) != 0) {
                printf("Erro ao salvar o arquivo WAV.\n");
                free(sinal_completo1);  // Liberar a memória alocada antes de retornar
                return -1;
            }

            free(sinal_completo1);  // Liberar a memória alocada após o processamento
            return 0;  // Sucesso
    */

    // Inicializar os sensores
    fd = setup_sensors();
    if (fd == -1) {
        return 1;
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

    // Alocar buffers para os quatro canais (sem o 'filtrado' no nome)
    short **buffers_sinal1_left = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal1_right = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_left = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_right = (short **)malloc(num_buffers * sizeof(short *));

    if (!buffers_sinal1_left || !buffers_sinal1_right ||
        !buffers_sinal2_left || !buffers_sinal2_right) {
        printf("Erro: falha ao alocar buffers de sinal.\n");
        return -1;  // Retorna erro
    } else {
        printf("Sucesso ao alocar buffers de canais\n");
    }

    // Definir buffer_channel_size como metade de buffer_size
    int buffer_channel_size = buffer_size / 2;

// Teste 2: Verificação se a separação de canais consegue retornar ao original
/*
    // Inicializar buffers para os quatro canais
    for (int i = 0; i < num_buffers; i++) {
        buffers_sinal1_left[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal1_right[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal2_left[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal2_right[i] = (short *)malloc(buffer_channel_size * sizeof(short));

        if (!buffers_sinal1_left[i] || !buffers_sinal1_right[i] ||
            !buffers_sinal2_left[i] || !buffers_sinal2_right[i]) {
            printf("Erro: falha ao alocar memória para buffer de sinal.\n");
            return -1;  // Retorna erro
        }

        // Verifica se o último buffer foi alocado
        if (i == num_buffers - 1) {
            printf("Sucesso ao alocar memória para buffers de sinais de canais\n");
        }

        // Loop para separar valores pares e ímpares de (*buffers_sinal1) e (*buffers_sinal2)
        int left_index_sinal1 = 0;  // Índice para buffers_sinal1_left
        int right_index_sinal1 = 0; // Índice para buffers_sinal1_right
        int left_index_sinal2 = 0;  // Índice para buffers_sinal2_left
        int right_index_sinal2 = 0; // Índice para buffers_sinal2_right

        for (int j = 0; j < buffer_size; j++) {
            // Processamento de buffers_sinal1
            if (j % 2 == 0) { // Verifica se o índice é par
                buffers_sinal1_left[i][left_index_sinal1++] = (*buffers_sinal1)[i][j];
            } else { // Caso o índice seja ímpar
                buffers_sinal1_right[i][right_index_sinal1++] = (*buffers_sinal1)[i][j];
            }

            // Processamento de buffers_sinal2
            if (j % 2 == 0) { // Verifica se o índice é par
                buffers_sinal2_left[i][left_index_sinal2++] = (*buffers_sinal2)[i][j];
            } else { // Caso o índice seja ímpar
                buffers_sinal2_right[i][right_index_sinal2++] = (*buffers_sinal2)[i][j];
            }
        }
    }

                int posicao2 = 0;  // Variável para controle da posição no sinal_completo
                int tamanho_total_sinal2 = num_buffers * buffer_size;  // Total de amostras no sinal final
                short *sinal_completo2 = (short *)malloc(tamanho_total_sinal2 * sizeof(short));

                if (sinal_completo2 == NULL) {
                    printf("Erro ao alocar memória para o sinal completo.\n");
                    return -1;  // Erro de alocação
                }

                // Reconstruir o sinal completo a partir dos buffers separados
                for (int i = 0; i < num_buffers; i++) {
                    int left_index_sinal1 = 0;  // Índice para buffers_sinal1_left
                    int right_index_sinal1 = 0; // Índice para buffers_sinal1_right

                    // Intercalação dos canais esquerdo e direito
                    for (int j = 0; j < buffer_size / 2; j++) {  // Considerando a intercalação de cada canal
                        // Canal esquerdo
                        sinal_completo2[posicao2++] = buffers_sinal1_left[i][left_index_sinal1++];
                        // Canal direito
                        sinal_completo2[posicao2++] = buffers_sinal1_right[i][right_index_sinal1++];
                    }
                }

                // Salvar o sinal reconstruído no arquivo WAV
                if (escrever_wav_estereo("sinal_etapa2.wav", sinal_completo2, tamanho_total_sinal2 / 2) != 0) {
                    printf("Erro ao salvar o arquivo WAV.\n");
                    free(sinal_completo2);  // Liberar a memória alocada antes de retornar
                    return -1;
                }

                free(sinal_completo2);  // Liberar a memória alocada após o processamento
                return 0;  // Sucesso


    */
// Fim do Teste 2

    // Inicializar buffers para os quatro canais
    for (int i = 0; i < num_buffers; i++) {
        buffers_sinal1_left[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal1_right[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal2_left[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal2_right[i] = (short *)malloc(buffer_channel_size * sizeof(short));

        if (!buffers_sinal1_left[i] || !buffers_sinal1_right[i] ||
            !buffers_sinal2_left[i] || !buffers_sinal2_right[i]) {
            printf("Erro: falha ao alocar memória para buffer de sinal.\n");
            return -1;  // Retorna erro
        }

        // Verifica se o último buffer foi alocado
        if (i == num_buffers - 1) {
            printf("Sucesso ao alocar memória para buffers de sinais de canais\n");
        }

        // Loop para separar valores pares e ímpares de (*buffers_sinal1) e (*buffers_sinal2)
        int left_index_sinal1 = 0;  // Índice para buffers_sinal1_left
        int right_index_sinal1 = 0; // Índice para buffers_sinal1_right
        int left_index_sinal2 = 0;  // Índice para buffers_sinal2_left
        int right_index_sinal2 = 0; // Índice para buffers_sinal2_right

        for (int j = 0; j < buffer_size; j++) {
            // Processamento de buffers_sinal1
            if (j % 2 == 0) { // Verifica se o índice é par
                buffers_sinal1_left[i][left_index_sinal1++] = (*buffers_sinal1)[i][j];
            } else { // Caso o índice seja ímpar
                buffers_sinal1_right[i][right_index_sinal1++] = (*buffers_sinal1)[i][j];
            }

            // Processamento de buffers_sinal2
            if (j % 2 == 0) { // Verifica se o índice é par
                buffers_sinal2_left[i][left_index_sinal2++] = (*buffers_sinal2)[i][j];
            } else { // Caso o índice seja ímpar
                buffers_sinal2_right[i][right_index_sinal2++] = (*buffers_sinal2)[i][j];
            }
        }

                // Verificando se os buffers estão vazios
        if (left_index_sinal1 == 0 && right_index_sinal1 == 0) {
            printf("O buffer sinal1 está vazio!\n");
        }

        if (left_index_sinal2 == 0 && right_index_sinal2 == 0) {
            printf("O buffer sinal2 está vazio!\n");
        }

        if (left_index_sinal1 == 0 && right_index_sinal1 == 0) {
            printf("O buffer sinal1 esquerdo está vazio!\n");
        }

        if (left_index_sinal2 == 0 && right_index_sinal2 == 0) {
            printf("O buffer sinal2 direito está vazio!\n");
        }

    }

        // Exibir o tamanho de cada buffer ao final
    printf("O buffer do sinal 1, canal esquerdo possui %d buffers de %d amostras.\n", num_buffers, buffer_size / 2);
    printf("O buffer do sinal 1, canal direito possui %d buffers de %d amostras.\n", num_buffers, buffer_size / 2);
    printf("O buffer do sinal 2, canal esquerdo possui %d buffers de %d amostras.\n", num_buffers, buffer_size / 2);
    printf("O buffer do sinal 2, canal direito possui %d buffers de %d amostras.\n", num_buffers, buffer_size / 2);

    //exit(0);

    // Alocar array para o sinal completo (todos os buffers combinados)
    int tamanho_total_sinal = num_buffers * buffer_size;  // Total de amostras no sinal final
    short *sinal_completo = (short *)malloc(tamanho_total_sinal * sizeof(short));
    if (!sinal_completo) {
        printf("Erro: falha ao alocar o array do sinal completo.\n");
        return -1;  // Retorna erro
    }

    // Processar os buffers, aplicar o filtro FIR e o delay
    int posicao = 0;  // Variável para controle da posição no sinal_completo

    // Definir ponteiros para os buffers filtrados (alocar de forma iterativa)
    short **buffers_sinal1_left_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal1_right_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_left_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_right_filtrado = (short **)malloc(num_buffers * sizeof(short *));

    // Verificar se a alocação dos ponteiros foi bem-sucedida
    if (!buffers_sinal1_left_filtrado || !buffers_sinal1_right_filtrado || 
        !buffers_sinal2_left_filtrado || !buffers_sinal2_right_filtrado) {
        printf("Erro: falha ao alocar memória para buffers filtrados.\n");
        return -1;  // Retorna erro
    }

    // Criar buffers iterativamente para armazenar os sinais filtrados
    for (int i = 0; i < num_buffers; i++) {
        // Alocar memória para os buffers filtrados
        buffers_sinal1_left_filtrado[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal1_right_filtrado[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal2_left_filtrado[i] = (short *)malloc(buffer_channel_size * sizeof(short));
        buffers_sinal2_right_filtrado[i] = (short *)malloc(buffer_channel_size * sizeof(short));

        // Verificar falha na alocação de memória para os buffers filtrados
        if (!buffers_sinal1_left_filtrado[i] || !buffers_sinal1_right_filtrado[i] ||
            !buffers_sinal2_left_filtrado[i] || !buffers_sinal2_right_filtrado[i]) {
            printf("Erro: falha ao alocar memória para buffers filtrados no índice %d.\n", i);
            return -1;  // Retorna erro
        }

        // Mensagem de sucesso quando o último buffer for alocado
        if (i == num_buffers - 1) {
            printf("Sucesso ao alocar memória para buffers filtrados.\n");
        }
    }

// Agora você tem buffers temporários alocados para cada buffer filtrado.


    // Alocar buffers temporários para a média dos sinais filtrados (direita e esquerda)
    float *media_buffer_right = (float *)malloc(buffer_channel_size * sizeof(float));  // Alocar buffer temporário para a média do canal direito
    float *media_buffer_left = (float *)malloc(buffer_channel_size * sizeof(float));   // Alocar buffer temporário para a média do canal esquerdo
    float *media_buffer = (float *)malloc(buffer_channel_size * sizeof(float));   // Alocar buffer temporário para a média dos sinais

    short *buffer_reproduzivel = (short *)malloc(buffer_size * sizeof(short));

    if (buffer_reproduzivel == NULL) {
    printf("Erro na alocação de memória para o buffer!\n");
    return -1;
    }

    // Verificar se a alocação foi bem-sucedida
    if (!media_buffer_right) {
        printf("Erro: falha ao alocar memória para media_buffer_right.\n");
        return -1;
    }
    if (!media_buffer_left) {
        printf("Erro: falha ao alocar memória para media_buffer_left.\n");
        return -1;
    }
    if (!media_buffer) {
        printf("Erro: falha ao alocar memória para media_buffer.\n");
        return -1;
    }

    int tamanho_total_sinal5 = num_buffers * buffer_size;  // Total de amostras no sinal final
    int posicao5_1 = 0, posicao5_2 = 0, posicao5 = 0, posicao6 = 0;  // Variáveis para controle da posição nos sinais
    short *sinal_completo5_1 = (short *)malloc(tamanho_total_sinal5 * sizeof(short));
    short *sinal_completo5_2 = (short *)malloc(tamanho_total_sinal5 * sizeof(short));
    short *sinal_completo5 = (short *)malloc(tamanho_total_sinal5 * sizeof(short));
    short *sinal_completo6 = (short *)malloc(tamanho_total_sinal5 * sizeof(short));


    float buffer_media_left[buffer_channel_size];
    float buffer_media_right[buffer_channel_size];
    float buffer_media_estereo[buffer_size];
    float buffer_fx_left[buffer_channel_size];
    float buffer_fx_right[buffer_channel_size];
    short buffer_audio[buffer_size];
    short buffer_media_estereo_short[buffer_size];



        if (sinal_completo5_1 == NULL || sinal_completo5_2 == NULL || sinal_completo5 == NULL || sinal_completo6 == NULL) {
            printf("Erro ao alocar memória para os sinais completos 5_1, 5_2 ou 5.\n");
            return -1;  // Erro de alocação
        }

        for (int i = 0; i < num_buffers; i++) {

            // Atualizar coeficientes a cada 10 buffers (se necessário)
            if (i % 10 == 0) {

            read_analog_values(fd, &analogValue0, &analogValue1);
            wetness = analogValue1 / 255.0;
            digitalValue = read_digital_value();

            // Utilize os valores conforme necessário
            printf("AIN0: %d | AIN1: %d | GPIO4: %d\n", analogValue0, analogValue1, digitalValue);

            // Determinando o efeito com base no valor de digitalValue
            char* efeito = (digitalValue == 0) ? "Delay" : (digitalValue == 1) ? "Reverb" : "Nenhum";

            // Acessando os valores das frequências de corte diretamente e incluindo o efeito
            printf("Frequência de corte 1: %.2f Hz | Frequência de corte 2: %.2f Hz | Efeito: %s | Wetness: %.2f\n", 
                frequencias_log[analogValue0], frequencias_log[255 - analogValue0], efeito, wetness);
            }


            // Obter coeficientes para o filtro FIR para o sinal 1, ambos os canais
            float *coeficientes_filtro_1 = matriz_coeficientes[analogValue0]; // Coeficientes para o sinal 1

            if ((buffers_sinal1_left)[i] != NULL) {
                aplicar_filtro_FIR_buffer((buffers_sinal1_left)[i], (buffers_sinal1_left_filtrado)[i], buffer_channel_size, coeficientes_filtro_1, ORDER);
            }
            if ((buffers_sinal1_right)[i] != NULL) {
                aplicar_filtro_FIR_buffer((buffers_sinal1_right)[i], (buffers_sinal1_right_filtrado)[i], buffer_channel_size, coeficientes_filtro_1, ORDER);
            }


            // Obter coeficientes para o filtro FIR para o sinal 2, ambos os canais
            float *coeficientes_filtro_2 = matriz_coeficientes[255 - analogValue0]; // Coeficientes para o sinal 2

            // Aplicar o filtro FIR para o sinal 2, canal esquerdo
            if ((buffers_sinal2_left)[i] != NULL) {
                aplicar_filtro_FIR_buffer((buffers_sinal2_left)[i], (buffers_sinal2_right_filtrado)[i], buffer_channel_size, coeficientes_filtro_2, ORDER);
            }

            // Aplicar o filtro FIR para o sinal 2, canal direito
            if ((buffers_sinal2_right)[i] != NULL) {
                aplicar_filtro_FIR_buffer((buffers_sinal2_right)[i], (buffers_sinal2_left_filtrado)[i], buffer_channel_size, coeficientes_filtro_2, ORDER);
            }

            // Para cada amostra de ambos os canais (esquerdo e direito)
            for (int j = 0; j < buffer_channel_size; j++) {
                /* -> sinais de teste
                // Intercalação do sinal 1 (canal esquerdo e direito)
                sinal_completo5_1[posicao5_1++] = (short)buffers_sinal1_left_filtrado[i][j];   // Canal esquerdo de sinal 1
                sinal_completo5_1[posicao5_1++] = (short)buffers_sinal1_right_filtrado[i][j];  // Canal direito de sinal 1

                // Intercalação do sinal 2 (canal esquerdo e direito)
                sinal_completo5_2[posicao5_2++] = (short)buffers_sinal2_left_filtrado[i][j];   // Canal esquerdo de sinal 2
                sinal_completo5_2[posicao5_2++] = (short)buffers_sinal2_right_filtrado[i][j];  // Canal direito de sinal 2

                // Média das amostras dos canais esquerdo e direito com conversão explícita
                buffer_media_estereo[2 * j] = ((float)buffers_sinal1_left_filtrado[i][j] + (float)buffers_sinal2_left_filtrado[i][j]) / 2.0f;   // Canal esquerdo
                buffer_media_estereo[2 * j + 1] = ((float)buffers_sinal1_right_filtrado[i][j] + (float)buffers_sinal2_right_filtrado[i][j]) / 2.0f; // Canal direito

                // Transferir o conteúdo do buffer_estereo para sinal_completo5 diretamente
                sinal_completo5[posicao5++] = (short)buffer_media_estereo[2 * j];      // Canal esquerdo
                sinal_completo5[posicao5++] = (short)buffer_media_estereo[2 * j + 1];  // Canal direito
                */
                // Média dos canais esquerdo e direito filtrados
                buffer_media_left[j] = ((float)buffers_sinal1_left_filtrado[i][j] + (float)buffers_sinal2_left_filtrado[i][j]) / 2.0f; 
                buffer_media_right[j] = ((float)buffers_sinal1_right_filtrado[i][j] + (float)buffers_sinal2_right_filtrado[i][j]) / 2.0f;
            }
            
            // Avaliar o valor de sel_Fx e aplicar o efeito correspondente para os dois canais
            if (digitalValue == 0) {
                // Se digitalValue for 0, aplicar o efeito de delay
                aplicar_delay(buffer_media_left, buffer_channel_size, wetness, 0.6f);  // Aplicar delay no canal esquerdo
                aplicar_delay(buffer_media_right, buffer_channel_size, wetness, 0.6f);  // Aplicar delay no canal direito
            } else if (digitalValue == 1) {
                // Se digitalValue for 1, aplicar o efeito de reverb
                applyReverbEffectBuffer(buffer_media_left, buffer_channel_size, wetness, 0.6f);  // Aplicar reverb no canal esquerdo
                applyReverbEffectBuffer(buffer_media_right, buffer_channel_size, wetness, 0.6f);  // Aplicar reverb no canal direito
            }
            
            
            int posicao_audio = 0;
            for (int j = 0; j < buffer_channel_size; j++) {

                // Transferir o conteúdo do buffer_estereo para sinal_completo5 diretamente
                sinal_completo6[posicao6++] = (short)buffer_media_left[2 * j];      // Canal esquerdo
                sinal_completo6[posicao6++] = (short)buffer_media_right[2 * j + 1];  // Canal direito
                // Preenche o buffer_audio com os valores dos canais esquerdo e direito
                buffer_audio[posicao_audio++] = (short)buffer_media_left[j];  // Canal esquerdo
                buffer_audio[posicao_audio++] = (short)buffer_media_right[j];  // Canal direito
            }

            // Converte os valores de float para short diretamente no loop
            for (size_t i = 0; i < buffer_size; i++) {
                // Escala o valor de float e converte para short
                buffer_media_estereo_short[i] = (short)(buffer_media_estereo[i]);  // Assumindo que o valor esteja entre -1.0 e 1.0
            }

            // Agora, o sinal está reconstruído em buffer_audio. Podemos reproduzir usando a função reproduzir
            size_t buffer_size = BUFFER_SIZE * sizeof(short);  // Tamanho total em bytes
            if (reproduzir(pcm_handle, buffer_audio, buffer_size) < 0) {
                fprintf(stderr, "Erro ao reproduzir o sinal reconstruído\n");
                return -1;
            }


        }
    /*
    // Salvar o sinal completo 5 (média dos sinais 1 e 2) em um arquivo WAV
    if (escrever_wav_estereo("sinal_etapa6.wav", sinal_completo6, tamanho_total_sinal5 / 2) != 0) {
        printf("Erro ao salvar o arquivo WAV para o sinal completo 5.\n");
        free(sinal_completo5_1);
        free(sinal_completo5_2);
        free(sinal_completo5);
        return -1;
    }
    */

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