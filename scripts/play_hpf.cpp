#include <iostream>
#include <cmath>
#include <sndfile.h>
#include <vector>
#include <string>

#define PI 3.14159265359

// Função para aplicar o filtro passa-altas de Butterworth de 2ª ordem
void applyHighPassFilter(short* buffer, int num_samples, int num_channels, float cutoff_freq, int sample_rate) {
    // Frequência de corte normalizada (wc) na faixa de 0 a 1
    float wc = 2 * PI * cutoff_freq / sample_rate;  
    float cos_wc = cos(wc);
    float alpha = sin(wc) / 2.0;

    // Coeficientes do filtro de Butterworth de 2ª ordem
    float a0 = 1.0 + alpha;
    float a1 = -2.0 * cos_wc;
    float a2 = 1.0 - alpha;
    float b0 = (1.0 + cos_wc) / 2.0;
    float b1 = -(1.0 + cos_wc);
    float b2 = (1.0 + cos_wc) / 2.0;

    // Normalizando os coeficientes
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    // Processamento do sinal de cada canal
    for (int ch = 0; ch < num_channels; ++ch) {
        float prev_input1 = 0.0, prev_input2 = 0.0;
        float prev_output1 = 0.0, prev_output2 = 0.0;

        for (int i = ch; i < num_samples * num_channels; i += num_channels) {
            float input = static_cast<float>(buffer[i]);
            // Filtro de Butterworth de 2ª ordem
            float output = b0 * input + b1 * prev_input1 + b2 * prev_input2 
                           - a1 * prev_output1 - a2 * prev_output2;

            // Atualizando variáveis para a próxima amostra
            prev_input2 = prev_input1;
            prev_input1 = input;
            prev_output2 = prev_output1;
            prev_output1 = output;

            // Armazenando o valor filtrado
            buffer[i] = static_cast<short>(output);
        }
    }
}

// Função para controle de ganho pós-processamento (opcional)
void applyGainControl(short* buffer, int num_samples, float gain) {
    for (int i = 0; i < num_samples; ++i) {
        buffer[i] = static_cast<short>(buffer[i] * gain);
    }
}

int main() {
    // Caminho do arquivo WAV
    const std::string filename = "audio02.wav";  // Substitua com o seu arquivo de áudio
    std::vector<float> cutoff_freqs = {1.0, 20.0, 300.0, 4000.0, 22000.0};  // Frequências de corte para o filtro passa-altas
    float gain = 0.8f;  // Controle de ganho pós-processamento (ajustável)
    SNDFILE* sndfile;
    SF_INFO sfinfo;

    // Abrindo o arquivo WAV para leitura
    sndfile = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
        std::cerr << "Erro ao abrir o arquivo WAV: " << sf_strerror(NULL) << std::endl;
        return 1;
    }

    int num_channels = sfinfo.channels;
    int sample_rate = sfinfo.samplerate;
    int num_samples = sfinfo.frames * num_channels;

    // Alocando buffer para armazenar os dados de áudio
    short* buffer = new short[num_samples];
    if (!buffer) {
        std::cerr << "Erro ao alocar memória para o buffer de áudio." << std::endl;
        sf_close(sndfile);
        return 1;
    }

    // Lendo os dados de áudio do arquivo WAV
    sf_readf_short(sndfile, buffer, sfinfo.frames);

    // Iterando sobre as frequências de corte e processando cada uma delas
    for (float cutoff_freq : cutoff_freqs) {
        // Aplicando o filtro passa-altas
        applyHighPassFilter(buffer, sfinfo.frames, num_channels, cutoff_freq, sample_rate);

        // Aplicando o controle de ganho pós-processamento
        applyGainControl(buffer, sfinfo.frames, gain);

        // Nome do arquivo de saída baseado na frequência de corte
        std::string output_filename = "hpf_butter_" + std::to_string(static_cast<int>(cutoff_freq)) + ".wav";

        // Salvando o áudio filtrado em um novo arquivo WAV
        SF_INFO out_sfinfo = sfinfo;  // Usando as mesmas configurações do arquivo de entrada
        SNDFILE* out_sndfile = sf_open(output_filename.c_str(), SFM_WRITE, &out_sfinfo);
        if (!out_sndfile) {
            std::cerr << "Erro ao abrir o arquivo de saída WAV: " << sf_strerror(NULL) << std::endl;
            delete[] buffer;
            sf_close(sndfile);
            return 1;
        }

        // Escrevendo os dados filtrados no arquivo de saída
        sf_writef_short(out_sndfile, buffer, sfinfo.frames);

        // Fechando o arquivo de saída
        sf_close(out_sndfile);

        std::cout << "Áudio filtrado com a frequência de corte " << cutoff_freq 
                  << "Hz e salvo em: " << output_filename << std::endl;
    }

    // Finalizando
    delete[] buffer;
    sf_close(sndfile);

    return 0;
}
