#include <stdio.h>

// Função para realizar o deslizamento de janela, preenchendo com zero onde não há valores
void deslizamento_janela_centrada(double *vetor1, int buffer_size, int ordem) {
    int meio = ordem / 2;  // Índice do meio da janela
    double buffer_temp[ordem];  // Buffer temporário para a janela

    // Inicializa o buffer com zeros
    for (int j = 0; j < ordem; j++) {
        buffer_temp[j] = 0.0;
    }

    // A partir daqui, o deslizamento ocorrerá
    for (int i = 0; i < buffer_size; i++) {
        // Preenche o buffer com as amostras da janela atual, com zeros no início e no fim
        for (int j = 0; j < ordem; j++) {
            int indice_vetor1 = i + (j - meio);  // Desloca a janela para o centro
            if (indice_vetor1 >= 0 && indice_vetor1 < buffer_size) {
                buffer_temp[j] = vetor1[indice_vetor1];  // Atribui valores de vetor1
            } else {
                buffer_temp[j] = 0.0;  // Zeros nas posições fora dos limites do vetor1
            }
        }

        // Imprimir o buffer para a janela atual
        printf("Janela [%d]: ", i);
        for (int j = 0; j < ordem; j++) {
            printf("%.2f ", buffer_temp[j]);
        }
        printf("\n");
    }
}

int main() {
    double vetor1[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};  // Vetor com 10 valores
    int ordem = 3;  // Defina a ordem da janela
    int buffer_size = sizeof(vetor1) / sizeof(vetor1[0]);  // Tamanho do vetor

    deslizamento_janela_centrada(vetor1, buffer_size, ordem);

    return 0;
}
