#include <stdio.h>
#include "filter_final.h"  // Incluindo o cabeçalho filter_final.h
#include <math.h>

float calcular_nova_frequencia_corte(float frequencia_corte_atual) {
    float fc1_min = 20.0;
    float fc1_max = 22050.0;

    // Fator de normalização baseado no intervalo de frequências
    float fator = (frequencia_corte_atual - fc1_min) / (fc1_max - fc1_min);

    // Usamos uma transformação logarítmica para garantir que a mudança seja mais lenta em frequências baixas
    float fc2 = fc1_max * pow(1.0 - fator, 2);  // A mudança é mais rápida para frequências altas
    printf("Para fc1 = %.2f, fc2 = %.2f\n", frequencia_corte_atual, fc2);
    return fc2;
}



// Aqui você pode chamar as funções de filter_final.c
