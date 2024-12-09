#include <stdio.h>
#include <math.h>

typedef struct {
    float last_in;  // última amostra de entrada
    float last_out; // última amostra de saída
    float coef;     // coeficiente do filtro
} HighPassFilter;

void init_hpf(HighPassFilter *hpf, float sr, float fc) {
    hpf->last_in = 0.0;
    hpf->last_out = 0.0;
    hpf->coef = 1 - (2 * M_PI * fc) / sr;
    if (hpf->coef < 0) hpf->coef = 0;
    else if (hpf->coef > 1) hpf->coef = 1;
    printf("Coeficiente: %f\n", hpf->coef);  // Mostra o coeficiente para depuração
}

float process_hpf(HighPassFilter *hpf, float input) {
    float new_out = 0.5 * (1 + hpf->coef) * (input - hpf->last_in) + hpf->coef * hpf->last_out;
    hpf->last_in = input;
    hpf->last_out = new_out;
    return new_out;
}

int main() {
    float sr = 44100.0;  // taxa de amostragem
    float fc = 1000.0;   // frequência de corte
    HighPassFilter hpf;
    init_hpf(&hpf, sr, fc);

    // Exemplo de processamento de um sinal
    float signal[10] = {0.1, 0.2, 0.15, 0.22, 0.18, 0.25, 0.3, 0.28, 0.35, 0.32};
    float filtered_signal[10];
    
    printf("Sinal de entrada: \n");
    for (int i = 0; i < 10; i++) {
        printf("%f ", signal[i]);
    }
    printf("\n");

    // Processa o sinal usando o filtro passa-alta
    for (int i = 0; i < 10; i++) {
        filtered_signal[i] = process_hpf(&hpf, signal[i]);
    }

    printf("Sinal filtrado: \n");
    for (int i = 0; i < 10; i++) {
        printf("%f ", filtered_signal[i]);
    }
    printf("\n");

    return 0;
}
