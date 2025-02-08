#include <stdio.h>
#include <math.h>

#define N 256  // Número de pontos

// Função para gerar os pontos logarítmicos
void gerar_pontos_logaritmicos() {
    // Definindo os valores mínimos e máximos para as frequências
    float fc1_min = 20.0;
    float fc1_max = 22050.0;
    
    // Gerando 256 pontos igualmente espaçados na escala logarítmica
    printf("Pontos na escala logarítmica:\n");
    for (int i = 0; i < N; i++) {
        // Calcular a frequência logarítmica para o ponto i
        float fc = pow(10.0, log10(fc1_min) + (log10(fc1_max) - log10(fc1_min)) * i / (N - 1));
        printf("Ponto %d: %.2f Hz\n", i, fc);
    }
}

// Função para gerar os coeficientes de Hamming para um filtro passa-altas
void generate_hamming_highpass_filter(int N, double cutoff_frequency, double sample_rate) {
    // Constantes
    int M = (N - 1) / 2;  // Centro da janela (M)
    double nyquist = sample_rate / 2.0;
    double normalized_cutoff = cutoff_frequency / nyquist;  // Normalizar a frequência de corte

    // Coeficientes do filtro
    double h[N];

    // Gerar a resposta ideal do filtro passa-altas
    for (int n = 0; n < N; n++) {
        if (n == M) {
            h[n] = 2 * normalized_cutoff;  // Resposta para o centro (n = M)
        } else {
            h[n] = sin(M_PI * (n - M) * normalized_cutoff) / (M_PI * (n - M));  // Resposta ideal
        }
    }

    // Aplicar a janela de Hamming
    for (int n = 0; n < N; n++) {
        double window = 0.54 - 0.46 * cos(2 * M_PI * n / (N - 1));  // Janela de Hamming
        h[n] *= window;
    }

    // Exibir os coeficientes do filtro
    printf("Coeficientes do filtro passa-altas com janela de Hamming:\n");
    for (int n = 0; n < N; n++) {
        printf("%f\n", h[n]);
    }
}

int main() {
    // Gerar e exibir os pontos logarítmicos
    gerar_pontos_logaritmicos();

    // Gerar e exibir os coeficientes do filtro passa-altas
    int N = 121;  // Ordem do filtro
    double cutoff_frequency = 1000.0;  // Frequência de corte em Hz
    double sample_rate = 44100.0;  // Taxa de amostragem em Hz
    generate_hamming_highpass_filter(N, cutoff_frequency, sample_rate);

    return 0;
}
