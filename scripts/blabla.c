#include <iostream>
#include <vector>
#include <stdio.h>
#include <libspeexdsp.h>

int main() {
    // Tamanho do buffer de entrada e saída
    const int input_size = 1024;
    const int output_size = 2048;
    
    // Criação do objeto de resampling
    SpeexResamplerState* resampler = speex_resampler_init(1, input_size, output_size);

    if (resampler == nullptr) {
        std::cerr << "Falha ao inicializar o resampler!" << std::endl;
        return -1;
    }

    // Definir a taxa de entrada e saída (exemplo)
    speex_resampler_set_rate(resampler, 8000, 16000);
