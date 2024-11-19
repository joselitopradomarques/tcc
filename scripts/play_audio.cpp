#include <iostream>
#include <cmath>
#include <sndfile.h>

#define PI 3.14159265359

// Função de filtro passa-altas (Filtro FIR) com Limitação de Amplitude
void applyHighPassFilter(short* buffer, int num_samples, int num_channels, float cutoff_freq, int sample_rate) {
    float RC = 1.0 / (cutoff_freq * 2 * PI);  // Constante de tempo
    float dt = 1.0 / sample_rate;  // Intervalo de tempo
    float alpha = RC / (RC + dt);  // Fator do filtro

    // Aplicando o filtro a cada amostra de cada canal
    for (int ch = 0; ch < num_channels; ++ch) {
        float prev_input = 0.0;
        float prev_output = 0.0;
        for (int i = ch; i < num_samples * num_channels; i += num_channels) {
            float input = static_cast<float>(buffer[i]);
            // Filtro passa-altas: a equação é adaptada para enfatizar as altas frequências
            float output = alpha * (prev_output + input - prev_input);
            buffer[i] = static_cast<short>(output);
            prev_input = input;
            prev_output = output;
        }
    }

    // Limitação de amplitude para evitar saturação (clipping)
    const float MAX_AMPLITUDE = 32767.0f;  // Máxima amplitude para áudio de 16 bits (valor de saturação)
    const float MIN_AMPLITUDE = -32768.0f; // Para garantir que os valores negativos também sejam limitados

    for (int i = 0; i < num_samples * num_channels; ++i) {
        if (buffer[i] > MAX_AMPLITUDE) {
            buffer[i] = static_cast<short>(MAX_AMPLITUDE); // Saturação no valor máximo
        } else if (buffer[i] < MIN_AMPLITUDE) {
            buffer[i] = static_cast<short>(MIN_AMPLITUDE); // Saturação no valor mínimo
        }
    }
}

// Função para controle de ganho pós-processamento
// Função de controle de ganho e limite de pico
void applyGainControlWithPeakLimiting(short* buffer, int num_samples, float gain) {
    // Variável para armazenar o valor máximo absoluto de amostra
    float max_sample = 0.0f;

    // Encontrar o maior valor absoluto de amostra no buffer
    for (int i = 0; i < num_samples; ++i) {
        max_sample = std::max(max_sample, static_cast<float>(std::abs(buffer[i])));
    }

    // Se o valor máximo for muito pequeno, não aplicar ganho
    if (max_sample < 1e-5f) {
        std::cerr << "Amostras muito pequenas para aplicar ganho." << std::endl;
        return;
    }

    // Normalizando a amplitude total para evitar saturação, calculando o fator de normalização
    float normalization_factor = 32767.0f / max_sample;

    // Aplicando o controle de ganho de forma segura, com limitação de pico
    for (int i = 0; i < num_samples; ++i) {
        // Aplicando ganho com a normalização
        int temp = static_cast<int>(buffer[i] * gain * normalization_factor);

        // Limitando os picos para evitar saturação
        if (temp > 32767) {
            buffer[i] = 32767;  // Clipping superior
        } else if (temp < -32768) {
            buffer[i] = -32768;  // Clipping inferior
        } else {
            buffer[i] = static_cast<short>(temp);
        }
    }

    // Verificando novamente a saturação
    max_sample = 0.0f;
    for (int i = 0; i < num_samples; ++i) {
        max_sample = std::max(max_sample, static_cast<float>(std::abs(buffer[i])));
    }

    // Se necessário, re-ajustando a normalização para garantir que o áudio não sofra saturação
    if (max_sample > 32767.0f) {
        // Reaplicar um fator de limitação de pico
        float final_normalization_factor = 32767.0f / max_sample;
        for (int i = 0; i < num_samples; ++i) {
            buffer[i] = static_cast<short>(buffer[i] * final_normalization_factor);
        }
    }
}


int main() {
    // Caminho do arquivo WAV
    const std::string filename = "audio02.wav";  // Substitua com o seu arquivo de áudio
    const std::string output_filename = "filtered_audio_highpass.wav";  // Nome do arquivo de saída
    float cutoff_freq = 1000.0;  // Frequência de corte do filtro passa-altas
    float gain = 0.5f;  // Controle de ganho pós-processamento (ajustável)
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

    // Aplicando o filtro passa-altas
    applyHighPassFilter(buffer, sfinfo.frames, num_channels, cutoff_freq, sample_rate);

    // Aplicando o controle de ganho pós-processamento
    applyGainControl(buffer, sfinfo.frames, gain);

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

    // Finalizando
    delete[] buffer;
    sf_close(sndfile);
    sf_close(out_sndfile);

    std::cout << "Áudio filtrado e salvo com sucesso em: " << output_filename << std::endl;

    return 0;
}
