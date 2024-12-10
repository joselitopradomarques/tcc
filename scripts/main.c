#include <stdio.h>
#include <stdlib.h>
#include "filter_final.h"
#include "filter_final2.h"

int main() {
    // Definindo os caminhos dos arquivos de entrada e saída
    const char *input_files[] = {
        "/home/joselito/git/tcc/datas/audio01.wav",
        "/home/joselito/git/tcc/datas/audio02.wav"
    };
    const char *output_files[] = {
        "/home/joselito/git/tcc/scripts/saida_filtrada_1.wav",
        "/home/joselito/git/tcc/scripts/saida_filtrada_2.wav"
    };

    // Definindo os ponteiros para os sinais
    short *sinal_estereo[2] = {NULL, NULL};
    short *sinal_filtrado[2] = {NULL, NULL};
    int tamanho[2] = {0, 0};

    // Frequência de corte do primeiro arquivo (inicialmente definida)
    float frequencia_corte1 = 20.0;  // Definida diretamente no código

    // Loop para processar os dois arquivos
    for (int i = 0; i < 2; i++) {
        // Frequência de corte ajustada para cada arquivo
        float frequencia_corte_atual = (i == 0) ? frequencia_corte1 : calcular_nova_frequencia_corte(frequencia_corte1);

        // Leitura do arquivo atual
        if (ler_wav_estereo(input_files[i], &sinal_estereo[i], &tamanho[i]) != 0) {
            // Liberar memória alocada caso haja erro
            for (int j = 0; j < i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return -1;
        }

        // Alocar memória para o sinal filtrado
        sinal_filtrado[i] = (short *)malloc(tamanho[i] * 2 * sizeof(short));  // 2 canais (estéreo)
        if (!sinal_filtrado[i]) {
            printf("Erro ao alocar memória para o sinal filtrado %d\n", i + 1);
            for (int j = 0; j <= i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return -1;
        }

        // Gerar os coeficientes do filtro FIR para o arquivo atual
        float coeficientes[ORDEM];
        gerar_filtro_FIR(coeficientes, ORDEM, frequencia_corte_atual, SAMPLE_RATE);

        // Aplicar o filtro FIR no sinal atual
        aplicar_filtro_FIR(sinal_estereo[i], sinal_filtrado[i], tamanho[i] * 2, coeficientes, ORDEM);

        // Escrever o sinal filtrado no arquivo de saída
        if (escrever_wav_estereo(output_files[i], sinal_filtrado[i], tamanho[i]) != 0) {
            // Liberar memória em caso de erro
            for (int j = 0; j <= i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return -1;
        }
    }

    // Liberar memória após o processamento
    for (int i = 0; i < 2; i++) {
        free(sinal_estereo[i]);
        free(sinal_filtrado[i]);
    }

    printf("Processamento concluído com sucesso!\n");
    return 0;
}
