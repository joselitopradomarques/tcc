#include <stdio.h>
#include <liquid/liquid.h>

int main() {
    // Coeficientes do filtro FIR
    float h[5] = {0.2, 0.2, 0.2, 0.2, 0.2};  // Filtro FIR simples
    unsigned int N = 5;  // Tamanho do filtro

    // Criando o filtro FIR
    fir_filter f = fir_create(h, N);

    // Exemplo de dados de entrada
    float input_sample = 1.0f;
    float output_sample;

    // Processando um dado de entrada através do filtro
    fir_execute(f, input_sample, &output_sample);

    // Exibindo o resultado
    printf("Entrada: %f, Saída: %f\n", input_sample, output_sample);

    // Destruindo o filtro
    fir_destroy(f);

    return 0;
}
