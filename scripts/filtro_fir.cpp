#include <liquid/liquid.h>
#include <iostream>
#include <vector>

int main() {
    // Definindo os coeficientes do filtro FIR
    std::vector<float> h = {0.2, 0.2, 0.2, 0.2, 0.2}; // Coeficientes do filtro

    // Criando o objeto do filtro FIR
    firfilt_rrrf fir_filter = firfilt_rrrf_create(h.data(), h.size());

    // Criando um sinal de entrada (exemplo: um impulso)
    const int N = 10; // Número de amostras
    std::vector<float> x(N, 0.0f);
    x[0] = 1.0f; // Impulso na primeira posição

    // Vetor para armazenar a saída filtrada
    std::vector<float> y(N, 0.0f);

    // Aplicando o filtro FIR ao sinal de entrada
    for (int i = 0; i < N; i++) {
        firfilt_rrrf_push(fir_filter, x[i]);  // Empurra o valor de entrada
        firfilt_rrrf_execute(fir_filter, &y[i]);  // Calcula o valor de saída
    }

    // Exibindo o sinal de saída (filtrado)
    std::cout << "Sinal filtrado: ";
    for (int i = 0; i < N; i++) {
        std::cout << y[i] << " ";
    }
    std::cout << std::endl;

    // Destrói o filtro para liberar a memória
    firfilt_rrrf_destroy(fir_filter);

    return 0;
}
