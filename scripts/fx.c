#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>

#define BUFFER_SIZE 44100  // O tamanho do buffer de delay pode ser ajustado conforme necessário

typedef struct {
    float* buffer;        // Buffer de amostras de atraso
    int bufferSize;       // Tamanho do buffer (em número de amostras)
    int writeIndex;       // Índice de escrita no buffer de delay
    float feedback;       // Quantidade de feedback (quanto do som processado volta ao sinal)
    float delayTime;      // Tempo de atraso (em segundos)
    int sampleRate;       // Taxa de amostragem (em Hz)
} Delay;

// Estrutura para o Reverb
typedef struct {
    int delay;            // Atraso em amostras
    float feedback;       // Feedback para o reverb
    float decay;          // Decaimento do reverb
    float decayBuffer[BUFFER_SIZE];  // Buffer para o reverb
} Reverb;

// Função para ler o arquivo WAV
void readWAV(const char* filePath, float** leftChannel, float** rightChannel, int* numSamples) {
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(filePath, SFM_READ, &sfInfo);
    if (!file) {
        printf("Erro ao abrir o arquivo %s\n", filePath);
        exit(1);
    }

    *numSamples = sfInfo.frames;
    *leftChannel = (float*)malloc(*numSamples * sizeof(float));
    *rightChannel = (float*)malloc(*numSamples * sizeof(float));

    float* buffer = (float*)malloc(*numSamples * 2 * sizeof(float));
    sf_readf_float(file, buffer, *numSamples);

    for (int i = 0; i < *numSamples; i++) {
        (*leftChannel)[i] = buffer[2 * i];
        (*rightChannel)[i] = buffer[2 * i + 1];
    }

    free(buffer);
    sf_close(file);
}

// Função para escrever o arquivo WAV
void writeWAV(const char* filePath, float* leftChannel, float* rightChannel, int numSamples) {
    SF_INFO sfInfo;
    sfInfo.channels = 2;
    sfInfo.samplerate = 44100;
    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE* file = sf_open(filePath, SFM_WRITE, &sfInfo);
    if (!file) {
        printf("Erro ao criar o arquivo %s\n", filePath);
        exit(1);
    }

    float* buffer = (float*)malloc(numSamples * 2 * sizeof(float));
    for (int i = 0; i < numSamples; i++) {
        buffer[2 * i] = leftChannel[i];
        buffer[2 * i + 1] = rightChannel[i];
    }

    sf_writef_float(file, buffer, numSamples);
    free(buffer);
    sf_close(file);
}

void initDelay(Delay* delay, float delayTime, int sampleRate, float feedbackAmount) {
    printf("Inicializando o delay...\n");
    // Inicializa o delay com o valor do tempo de atraso e feedback

    // Calcula o número de amostras para o delay
    delay->delayTime = delayTime;
    delay->sampleRate = sampleRate;
    delay->bufferSize = (int)(delayTime * sampleRate);
    delay->buffer = (float*)malloc(delay->bufferSize * sizeof(float));

    if (delay->buffer == NULL) {
        printf("Erro ao alocar memória para o buffer de delay.\n");
        exit(1);  // Não há memória suficiente
    }

    // Inicializa o buffer com zeros
    for (int i = 0; i < delay->bufferSize; i++) {
        delay->buffer[i] = 0.0f;
    }

    delay->writeIndex = 0; // Começa a escrever no índice 0
    delay->feedback = feedbackAmount;
    printf("Delay inicializado com bufferSize: %d e feedback: %.2f\n", delay->bufferSize, delay->feedback);
}

float processDelay(Delay* delay, float inputSample, int* writeIndex) {
    //printf("Processando Delay...\n");
    // Lê o valor do delay a partir do buffer de memória
    int readIndex = *writeIndex - delay->bufferSize;
    if (readIndex < 0) {
        readIndex += delay->bufferSize; // Garante que o índice de leitura seja circular
    }

    // Aplica o feedback (se houver)
    float outputSample = delay->buffer[readIndex] + inputSample;

    // Armazena a amostra processada no buffer de delay
    delay->buffer[*writeIndex] = outputSample * delay->feedback;

    // Atualiza o índice de escrita
    *writeIndex = (*writeIndex + 1) % delay->bufferSize;

    return outputSample;  // Retorna a amostra processada
}

// Função para inicializar o Reverb
void initReverb(Reverb* reverb, float decayDB, float delayTime, int sampleRate, float feedbackAmount) {
    printf("Inicializando o reverb...\n");
    reverb->delay = (int)(delayTime * sampleRate);  // Atraso em amostras
    reverb->feedback = feedbackAmount;
    reverb->decay = powf(10, decayDB / 20);  // Converte o decaimento em dB para fator linear

    for (int i = 0; i < BUFFER_SIZE; i++) {
        reverb->decayBuffer[i] = 0.0f;
    }

    printf("Reverb inicializado com atraso: %d amostras e feedback: %.2f\n", reverb->delay, reverb->feedback);
}

// Função para processar o Reverb
float processReverb(Reverb* reverb, float inputSample, int* reverbIndex) {
   // printf("Processando Reverb...\n");
    int delayedIndex = (*reverbIndex + reverb->delay) % BUFFER_SIZE;
    float outputSample = reverb->decayBuffer[delayedIndex];  // Aplica o efeito do reverb

    reverb->decayBuffer[*reverbIndex] = inputSample + reverb->feedback * reverb->decayBuffer[*reverbIndex];

    *reverbIndex = (*reverbIndex + 1) % BUFFER_SIZE;

    return outputSample;
}

// Função para calcular o volume do arquivo de som com base no valor de f1,
// com f1 limitado entre 20 e 22050 Hz.
float calculateVolume(float f1) {
    // Limita f1 entre 20 Hz e 22050 Hz
    f1 = fmax(20.0f, fmin(f1, 22050.0f));

    // Cálculo do volume com a fórmula fornecida
    return (exp(-pow((f1 - 11000) / 1000, 2)) / (exp(-pow((f1 - 11000) / 1000, 2)) + exp((f1 - 11000) / 2000))) * 2;
}

// Função para aplicar o efeito selecionado (delay ou reverb)
void applyFX(const char* inputFilePath, const char* outputFilePath, float effectAmount, int sel_fx, float f1) {
    int sampleRate = 44100;
    float delayTime = 1.0f;    // Atraso de 1 segundo para o delay
    float feedbackAmount = effectAmount;  // Feedback controlado pelo parâmetro effectAmount
    float gain = 0.5f;         // Redução de ganho para evitar estouro
    int numSamples;

    // Lê o arquivo WAV estéreo de entrada
    float *leftChannel, *rightChannel;
    readWAV(inputFilePath, &leftChannel, &rightChannel, &numSamples);

    // Inicializa o delay ou o reverb com o valor de feedback controlado pelo parâmetro effectAmount
    Delay delayLeft, delayRight;
    Reverb reverbLeft, reverbRight;
    int delayIndexLeft = 0, delayIndexRight = 0;
    int reverbIndexLeft = 0, reverbIndexRight = 0;

    // Se sel_fx for 0, configura o delay
    if (sel_fx == 0) {
        printf("Aplicando efeito Delay...\n");
        initDelay(&delayLeft, delayTime, sampleRate, feedbackAmount);  // Inicializa o delay
        initDelay(&delayRight, delayTime, sampleRate, feedbackAmount);  // Inicializa o delay
    }
    // Se sel_fx for 1, configura o reverb
    else if (sel_fx == 1) {
        printf("Aplicando efeito Reverb...\n");
        initReverb(&reverbLeft, -40.0f, delayTime, sampleRate, feedbackAmount);  // Inicializa o reverb
        initReverb(&reverbRight, -40.0f, delayTime, sampleRate, feedbackAmount);  // Inicializa o reverb
    }

    // Arrays para armazenar as amostras de saída dos dois canais
    float* outputLeftChannel = (float*)malloc(numSamples * sizeof(float));
    float* outputRightChannel = (float*)malloc(numSamples * sizeof(float));

    if (outputLeftChannel == NULL || outputRightChannel == NULL) {
        printf("Erro ao alocar memória para os canais de saída.\n");
        exit(1);
    }

    // Aplica o efeito selecionado (delay ou reverb)
    for (int i = 0; i < numSamples; i++) {
        float effectGain = effectAmount;

        if (sel_fx == 0) {  // Delay
            outputLeftChannel[i] = processDelay(&delayLeft, leftChannel[i], &delayIndexLeft) * effectGain;
            outputRightChannel[i] = processDelay(&delayRight, rightChannel[i], &delayIndexRight) * effectGain;
        } else if (sel_fx == 1) {  // Reverb
            outputLeftChannel[i] = processReverb(&reverbLeft, leftChannel[i], &reverbIndexLeft) * effectGain;
            outputRightChannel[i] = processReverb(&reverbRight, rightChannel[i], &reverbIndexRight) * effectGain;
        }
    }

    // Aplica o ganho ao sinal processado
    for (int i = 0; i < numSamples; i++) {
        outputLeftChannel[i] *= gain;
        outputRightChannel[i] *= gain;
    }

    // Escreve o arquivo WAV de saída
    writeWAV(outputFilePath, outputLeftChannel, outputRightChannel, numSamples);

    // Libera a memória alocada
    free(leftChannel);
    free(rightChannel);
    free(outputLeftChannel);
    free(outputRightChannel);
}

int main() {
    const char* inputFilePath = "/home/joselito/git/tcc/scripts/audio02.wav";
    const char* outputFilePath = "/home/joselito/git/tcc/scripts/effect_output.wav";

    // Valor de f1 (exemplo)
    float f1 = 21.0f;  // Este valor deve ser ajustado conforme necessário

    // Calcula o volume com base em f1
    float volume = calculateVolume(f1);
    printf("Volume calculado: %.4f\n", volume);

    // Parâmetros do efeito
    int sel_fx = 0;  // 0 para delay, 1 para reverb
    float effectAmount = volume; // Ajusta o efeito com o volume calculado

    // Aplica o efeito
    applyFX(inputFilePath, outputFilePath, effectAmount, sel_fx, f1);

    return 0;
}
