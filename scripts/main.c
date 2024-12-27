// main.c

#include <stdio.h>
#include <stdlib.h>
#include "filter_final.h"
#include "processamento_sinais.h"  // Incluir o cabeçalho onde somar_sinais está declarada
#include "delay.h"  // Incluir o delay
#include "reverb.h" // Incluir o reverb
#include <sndfile.h>

int main() {
    // Define o efeito de delay com o valor entre 0.0 e 1.0, onde 1.0 corresponde ao feedback máximo (100 dB)
    // Define o efeito de reverb com o valor entre 0.0 e 1.0, onde 1.0 corresponde ao feedback máximo (100 dB)
    float fx_Amount = 0.0f;  // Ajuste para variar entre 0.0 (sem efeito) e 1.0 (feedback máximo)
    int fx_select = 0;

    // Definindo as frequências de corte para a filtragem
    // float frequencias_corte[] = {20.0f, 300.0f, 4000.0f, 22050.0f};  // Exemplo de várias frequências
    float frequencias_corte[] = {20.0f};  // Exemplo de várias frequências
    int num_frequencias = sizeof(frequencias_corte) / sizeof(frequencias_corte[0]);

    for (int i = 0; i < num_frequencias; i++) {
        float frequencia_corte = frequencias_corte[i];
        printf("Testando com selecao de frequencia: %.2f Hz\n", frequencia_corte);

        // Aplica a filtragem e o efeito para cada frequência de corte
        aplicar_filtragem_e_efeito(frequencia_corte, fx_select, fx_Amount);
        printf("\n---------------------------------------------------------------------\n");
    }

    return 0;
}
