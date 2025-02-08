// filter_final.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "filter_final.h"  // Inclui o cabeçalho com as declarações das funções

// Função para aplicar o filtro FIR no sinal mono
void aplicar_filtro_FIR(short *sinal, short *sinal_filtrado, int tamanho, float *coeficientes, int ordem) {
    for (int i = 0; i < tamanho; i++) {
        float acumulador = 0.0;
        for (int j = 0; j < ordem; j++) {
            if (i - j >= 0) {
                acumulador += coeficientes[j] * sinal[i - j];
            }
        }
        // Limitar a amplitude para valores de 16 bits
        if (acumulador > MAX_16BIT) acumulador = MAX_16BIT;
        if (acumulador < -MAX_16BIT) acumulador = -MAX_16BIT;
        sinal_filtrado[i] = (short)acumulador;
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

float calcular_nova_frequencia_corte(float frequencia_corte_atual) {
    // Definindo os valores mínimos e máximos para as frequências
    float fc1_min = 20.0;
    float fc1_max = 22050.0;
    
    // Aplicando a transformação logarítmica inversa
    float fc2_log = log10(fc1_max) - (log10(frequencia_corte_atual) - log10(fc1_min));
    
    // Voltando para a escala linear após a transformação log
    float fc2 = pow(10.0, fc2_log);
    
    // Exibindo o resultado
    printf("\n---- Seção de filtragem:\n"); 
    printf("fc = %.2f Hz\n", frequencia_corte_atual);
    printf("HPF aplicado ao canal 1 com frequência de corte igual a %.2f Hz\n", frequencia_corte_atual);
    printf("HPF aplicado ao canal 2 com frequência de corte igual a %.2f Hz\n\n", fc2);

    
    // Retornando o valor de fc2
    return fc2;
}
