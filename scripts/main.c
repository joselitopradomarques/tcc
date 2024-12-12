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
    float effectAmount = 1.0f;  // Ajuste para variar entre 0.0 (sem efeito) e 1.0 (feedback máximo)
    apply_effect_to_audio(0, effectAmount);  // Efeito de Reverb

    // Definindo a frequência de corte para a filtragem
    float frequencia_corte = 2000.0f;  // Exemplo de frequência de corte
    executar_filtragem_e_soma(frequencia_corte);

    printf("Processamento concluído com sucesso!\n");

    return 0;

    return 0;
}
