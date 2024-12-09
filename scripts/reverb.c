#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <math.h>

// Definição de Reverb com o campo 'feedback' e 'delay'
typedef struct {
    float decay;
    int delay;  // Atraso em amostras
    float feedback;
} Reverb;

#define BUFFER_SIZE 44100  // Defina o tamanho do buffer (exemplo: 1 segundo de áudio a 44.1kHz)

float delayBufferLeft[BUFFER_SIZE];
float delayBufferRight[BUFFER_SIZE];
int delayIndexLeft = 0;
int delayIndexRight = 0;

// Funções auxiliares para leitura e gravação de arquivos WAV
void readWAV(const char* filePath, float** leftChannel, float** rightChannel, int* numSamples) {
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(filePath, SFM_READ, &sfInfo);
    
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para leitura.\n");
        exit(1);
    }

    *numSamples = sfInfo.frames;
    int numChannels = sfInfo.channels;

    *leftChannel = (float*)malloc(*numSamples * sizeof(float));
    *rightChannel = (float*)malloc(*numSamples * sizeof(float));

    if (*leftChannel == NULL || *rightChannel == NULL) {
        printf("Erro ao alocar memória para os canais.\n");
        exit(1);
    }

    float* buffer = (float*)malloc(*numSamples * numChannels * sizeof(float));

    // Lê o conteúdo do arquivo WAV para o buffer
    sf_readf_float(file, buffer, *numSamples);

    // Divide os canais
    for (int i = 0; i < *numSamples; i++) {
        (*leftChannel)[i] = buffer[i * numChannels];
        if (numChannels > 1) {
            (*rightChannel)[i] = buffer[i * numChannels + 1];
        } else {
            (*rightChannel)[i] = 0.0f;
        }
    }

    free(buffer);
    sf_close(file);
}

void writeWAV(const char* filePath, float* leftChannel, float* rightChannel, int numSamples) {
    SF_INFO sfInfo;
    sfInfo.frames = numSamples;
    sfInfo.channels = 2;  // Para um arquivo estéreo
    sfInfo.samplerate = 44100;
    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE* file = sf_open(filePath, SFM_WRITE, &sfInfo);
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para gravação.\n");
        exit(1);
    }

    float* buffer = (float*)malloc(numSamples * 2 * sizeof(float));

    for (int i = 0; i < numSamples; i++) {
        buffer[i * 2] = leftChannel[i];  // Canal esquerdo
        buffer[i * 2 + 1] = rightChannel[i];  // Canal direito
    }

    sf_writef_float(file, buffer, numSamples);
    free(buffer);
    sf_close(file);
}

void initReverb(Reverb* reverb, float decayDB, float delayTimeInSeconds, int sampleRate, float feedback) {
    reverb->decay = decayDB;
    reverb->feedback = feedback;
    // Converte o tempo de atraso para o número de amostras
    reverb->delay = (int)(delayTimeInSeconds * sampleRate);  // Atraso em amostras
}

// Processamento do reverb com o atraso aplicado
float processReverb(Reverb* reverb, float inputSample, float* delayBuffer, int delayBufferSize, int* delayIndex, int sampleIndex) {
    // Se o índice atual for menor que o atraso, não aplica o efeito
    if (sampleIndex < reverb->delay) {
        delayBuffer[*delayIndex] = 0.0f;  // Não aplica o efeito antes do tempo de atraso
        return 0.0f;  // Retorna 0 antes do início do efeito
    }
    
    // Quando o atraso for superado, começa a aplicar o reverb
    float outputSample = delayBuffer[*delayIndex];  // Somente o valor do buffer de delay

    // Atualiza o buffer de delay com a amostra atual + feedback ajustado pelo decaimento
    delayBuffer[*delayIndex] = inputSample + reverb->feedback * delayBuffer[*delayIndex];

    // Incrementa o índice do delay e faz o wraparound para simular um buffer circular
    *delayIndex = (*delayIndex + 1) % delayBufferSize;

    return outputSample;  // Retorna apenas o efeito de reverb
}

void applyGain(float* leftChannel, float* rightChannel, int numSamples, float gain) {
    for (int i = 0; i < numSamples; i++) {
        leftChannel[i] *= gain;
        rightChannel[i] *= gain;
    }
}

void normalize(float* leftChannel, float* rightChannel, int numSamples) {
    float maxLeft = 0.0f, maxRight = 0.0f;

    // Encontra o valor máximo absoluto em cada canal
    for (int i = 0; i < numSamples; i++) {
        if (fabs(leftChannel[i]) > maxLeft) {
            maxLeft = fabs(leftChannel[i]);
        }
        if (fabs(rightChannel[i]) > maxRight) {
            maxRight = fabs(rightChannel[i]);
        }
    }

    float maxVal = fmax(maxLeft, maxRight);

    // Normaliza os canais para evitar clipping
    if (maxVal > 0.0f) {
        float normalizationFactor = 1.0f / maxVal;
        for (int i = 0; i < numSamples; i++) {
            leftChannel[i] *= normalizationFactor;
            rightChannel[i] *= normalizationFactor;
        }
    }
}

void applyReverbEffect(const char* inputFilePath, const char* outputFilePath, float effectAmount) {
    int sampleRate = 44100;
    float decayDB = 40.0f;    // Ajuste do decaimento
    float delayTime = 1.0f;   // Atraso de 4 segundos
    float feedbackAmount = effectAmount;  // Ajuste de feedback controlado pelo parâmetro
    float gain = 0.5f;        // Redução de ganho para evitar estouro
    int numSamples;

    // Lê o arquivo WAV estéreo de entrada
    float *leftChannel, *rightChannel;
    readWAV(inputFilePath, &leftChannel, &rightChannel, &numSamples);
    if (leftChannel == NULL || rightChannel == NULL) {
        exit(1);
    }

    // Inicializa o reverb com o valor de feedback controlado pelo parâmetro effectAmount
    Reverb reverbLeft, reverbRight;
    initReverb(&reverbLeft, decayDB, delayTime, sampleRate, feedbackAmount);
    initReverb(&reverbRight, decayDB, delayTime, sampleRate, feedbackAmount);

    // Arrays para armazenar as amostras de saída dos dois canais
    float* outputLeftChannel = (float*)malloc(numSamples * sizeof(float));
    float* outputRightChannel = (float*)malloc(numSamples * sizeof(float));

    if (outputLeftChannel == NULL || outputRightChannel == NULL) {
        printf("Erro ao alocar memória para os canais de saída.\n");
        exit(1);
    }

    // Aplica o reverb em ambos os canais com delay
    for (int i = 0; i < numSamples; i++) {
        outputLeftChannel[i] = processReverb(&reverbLeft, leftChannel[i], delayBufferLeft, BUFFER_SIZE, &delayIndexLeft, i);
        outputRightChannel[i] = processReverb(&reverbRight, rightChannel[i], delayBufferRight, BUFFER_SIZE, &delayIndexRight, i);
    }

    // Aplica o ganho ao sinal processado
    applyGain(outputLeftChannel, outputRightChannel, numSamples, gain);

    // Normaliza o sinal para evitar clipping
    normalize(outputLeftChannel, outputRightChannel, numSamples);

    // Escreve o arquivo WAV estéreo de saída
    writeWAV(outputFilePath, outputLeftChannel, outputRightChannel, numSamples);

    // Libera a memória
    free(outputLeftChannel);
    free(outputRightChannel);
    free(leftChannel);
    free(rightChannel);

    printf("Arquivo %s criado com sucesso!\n", outputFilePath);
}

int main() {
    // Caminho do arquivo de entrada e saída
    const char* inputFilePath = "/home/joselito/git/tcc/scripts/audio02.wav";
    const char* outputFilePath = "/home/joselito/git/tcc/scripts/reverb_output_stereo.wav";

    // Chama a função de aplicação do efeito com o parâmetro effectAmount
    applyReverbEffect(inputFilePath, outputFilePath, 0.5f);  // Exemplo de efeito com feedback 50%

    return 0;
}
