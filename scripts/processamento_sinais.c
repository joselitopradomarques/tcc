// processamento_sinais.c
#include <stdio.h>
#include <stdlib.h>
#include "processamento_sinais.h"
#include <limits.h>

// Função para somar dois sinais de entrada
// sinal1 e sinal2 são os sinais de entrada (estéreo ou mono)
// resultado é o sinal de saída, onde será armazenada a soma dos sinais
// tamanho é o número de amostras dos sinais
void somar_sinais(short *sinal1, short *sinal2, short *resultado, int tamanho) {
    // Verificando se os sinais de entrada e o resultado foram alocados corretamente
    if (!sinal1 || !sinal2 || !resultado) {
        printf("Erro: memória não alocada para os sinais ou para o resultado.\n");
        return;
    }

    // Somando os sinais de entrada e armazenando no sinal de resultado
    for (int i = 0; i < tamanho; i++) {
        resultado[i] = sinal1[i] + sinal2[i];
        
        // Para evitar overflow, podemos garantir que o valor da soma não ultrapasse o limite dos valores short
        if (resultado[i] > SHRT_MAX) {
            resultado[i] = SHRT_MAX;  // Atribui o valor máximo possível para short
        } else if (resultado[i] < SHRT_MIN) {
            resultado[i] = SHRT_MIN;  // Atribui o valor mínimo possível para short
        }
    }
}
