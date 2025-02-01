// princ.c
// gcc -o princ princ.c proc.c -lpthread -lm
#include <stdio.h>
#include <stdlib.h>
#include "proc.h"

int main() {
    short *sinal1 = NULL, *sinal2 = NULL;
    int tamanho1 = 0, tamanho2 = 0;
    int buffer_size = 1024; // Tamanho dos buffers (pode ser ajustado conforme necessário)
    short **buffers_sinal1 = NULL, **buffers_sinal2 = NULL;
    int num_buffers = 0;

    // Definição do filtro FIR
    int ordem_filtro = 1;  // Ordem mínima do filtro (equivale a um atraso de uma amostra)
    float coeficientes_filtro[] = {1.0};  // Coeficiente de um filtro passa-tudo

    // Definição para efeito Reverb
    float wetness = 1.0f; // Defina o valor apropriado para o efeito

    // Ler os dois arquivos WAV estéreo
    if (ler_dois_wav_estereo(&sinal1, &sinal2, &tamanho1, &tamanho2) != 0) {
        printf("Erro ao ler os arquivos WAV.\n");
        return -1;
    }

    // Exibir tamanho dos sinais lidos
    printf("Tamanho do sinal 1: %d amostras\n", tamanho1);
    printf("Tamanho do sinal 2: %d amostras\n", tamanho2);

    // Gerar os buffers circulares a partir dos sinais
    if (gerar_buffers_circulares(sinal1, sinal2, tamanho1, buffer_size, &buffers_sinal1, &buffers_sinal2, &num_buffers) != 0) {
        printf("Erro ao gerar os buffers circulares.\n");
        free(sinal1);
        free(sinal2);
        return -1;
    }

    // Exibir informações sobre os buffers gerados
    printf("Número de buffers: %d\n", num_buffers);

    // Verificar se os buffers foram gerados corretamente
    if (buffers_sinal1 == NULL || buffers_sinal2 == NULL) {
        printf("Erro: os buffers não foram alocados corretamente.\n");
        free(sinal1);
        free(sinal2);
        return -1;
    }

    // Processar os buffers circulares aplicando o filtro FIR
    if (processar_buffers_circulares(&buffers_sinal1, &buffers_sinal2, num_buffers, buffer_size, coeficientes_filtro, ordem_filtro, wetness) != 0) {
        printf("Erro ao processar os buffers circulares.\n");
        free(sinal1);
        free(sinal2);
        return -1;
    }

    // Verificar se os buffers foram processados corretamente
    printf("Buffers processados com sucesso.\n");

    // Aqui você pode adicionar código para salvar os buffers processados ou realizar outras operações

    // Liberar memória alocada
    liberar_buffers(buffers_sinal1, buffers_sinal2, num_buffers);
    free(sinal1);
    free(sinal2);

    return 0; // Sucesso
}
