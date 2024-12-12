// processamento_sinais.c

#include <stdio.h>
#include <stdlib.h>
#include "processamento_sinais.h"
#include <limits.h>
#include "delay.h"
#include "reverb.h"

// Função para somar dois sinais de entrada
void somar_sinais(short *sinal1, short *sinal2, short *resultado, int tamanho) {
    if (!sinal1 || !sinal2 || !resultado) {
        printf("Erro: memória não alocada para os sinais ou para o resultado.\n");
        return;
    }

    for (int i = 0; i < tamanho; i++) {
        resultado[i] = sinal1[i] + sinal2[i];
        if (resultado[i] > SHRT_MAX) {
            resultado[i] = SHRT_MAX;
        } else if (resultado[i] < SHRT_MIN) {
            resultado[i] = SHRT_MIN;
        }
    }
}

void apply_effect_to_audio(int effect_choice, float effect_amount) {
    // Definições dos arquivos de entrada e saída dentro da função
    const char *input_file = "/home/joselito/git/tcc/datas/audio01.wav";  // Caminho do arquivo de entrada
    const char *output_file_delay = "/home/joselito/git/tcc/scripts/audio_delay.wav";  // Arquivo de saída do delay
    const char *output_file_reverb = "/home/joselito/git/tcc/scripts/audio_reverb.wav";  // Arquivo de saída do reverb

    if (effect_choice == 0) {
        // Aplicando o efeito de delay
        int delay_time = (int)effect_amount;  // Certifique-se de que delay_time seja um valor inteiro adequado
        apply_delay_to_audio(input_file, output_file_delay, delay_time);
        printf("Efeito de Delay aplicado. Arquivo de saída: %s\n", output_file_delay);
    } else if (effect_choice == 1) {
        // Aplicando o efeito de reverb
        applyReverbEffect(input_file, output_file_reverb, effect_amount);
        printf("Efeito de Reverb aplicado. Arquivo de saída: %s\n", output_file_reverb);
    } else {
        printf("Escolha inválida. Use 0 para delay e 1 para reverb.\n");
    }
}


// Função para executar a filtragem dos dois sinais e somá-los
void executar_filtragem_e_soma(float frequencia_corte) {
    // Definindo os caminhos dos arquivos de entrada e saída
    const char *input_files[] = {
        "/home/joselito/git/tcc/datas/audio01.wav",
        "/home/joselito/git/tcc/datas/audio02.wav"
    };
    const char *output_files[] = {
        "/home/joselito/git/tcc/scripts/saida_filtrada_1.wav",
        "/home/joselito/git/tcc/scripts/saida_filtrada_2.wav",
        "/home/joselito/git/tcc/scripts/saida_filtrada_soma.wav" // Arquivo para a soma
    };

    // Definindo os ponteiros para os sinais
    short *sinal_estereo[2] = {NULL, NULL};
    short *sinal_filtrado[2] = {NULL, NULL};
    short *sinal_soma = NULL;
    int tamanho[2] = {0, 0};

    // Loop para processar os dois arquivos
    for (int i = 0; i < 2; i++) {
        // Leitura do arquivo atual
        if (ler_wav_estereo(input_files[i], &sinal_estereo[i], &tamanho[i]) != 0) {
            // Liberar memória alocada caso haja erro
            for (int j = 0; j < i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return;
        }

        // Alocar memória para o sinal filtrado
        sinal_filtrado[i] = (short *)malloc(tamanho[i] * 2 * sizeof(short));  // 2 canais (estéreo)
        if (!sinal_filtrado[i]) {
            printf("Erro ao alocar memória para o sinal filtrado %d\n", i + 1);
            for (int j = 0; j <= i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return;
        }

        // Gerar os coeficientes do filtro FIR para o arquivo atual
        float coeficientes[ORDEM];
        gerar_filtro_FIR(coeficientes, ORDEM, frequencia_corte, SAMPLE_RATE);

        // Aplicar o filtro FIR no sinal atual
        aplicar_filtro_FIR(sinal_estereo[i], sinal_filtrado[i], tamanho[i] * 2, coeficientes, ORDEM);

        // Escrever o sinal filtrado no arquivo de saída
        if (escrever_wav_estereo(output_files[i], sinal_filtrado[i], tamanho[i]) != 0) {
            // Liberar memória em caso de erro
            for (int j = 0; j <= i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return;
        }
    }

    // Alocar memória para o sinal da soma
    sinal_soma = (short *)malloc(tamanho[0] * 2 * sizeof(short));  // 2 canais (estéreo)
    if (!sinal_soma) {
        printf("Erro ao alocar memória para o sinal da soma\n");
        for (int i = 0; i < 2; i++) {
            free(sinal_estereo[i]);
            free(sinal_filtrado[i]);
        }
        return;
    }

    // Somar os dois sinais filtrados
    somar_sinais(sinal_filtrado[0], sinal_filtrado[1], sinal_soma, tamanho[0] * 2);

    // Escrever o sinal somado no arquivo de saída
    if (escrever_wav_estereo(output_files[2], sinal_soma, tamanho[0]) != 0) {
        // Liberar memória em caso de erro
        for (int i = 0; i < 2; i++) {
            free(sinal_estereo[i]);
            free(sinal_filtrado[i]);
        }
        free(sinal_soma);
        return;
    }

    // Liberar memória após o processamento
    for (int i = 0; i < 2; i++) {
        free(sinal_estereo[i]);
        free(sinal_filtrado[i]);
    }
    free(sinal_soma);

    printf("Filtragem e soma concluídas com sucesso!\n");
}
