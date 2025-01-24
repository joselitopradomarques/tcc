#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <time.h>  // Inclua a biblioteca de tempo para controle do tempo de captura

#define DEVICE "hw:4,0"  // Dispositivo alterado para hw:4,0
#define PCM_CAPTURE_FILE "captured_audio_hw_4_0.wav"
#define CAPTURE_TIME 10  // Tempo de captura em segundos
#define SAMPLE_RATE 44100  // Taxa de amostragem (44100 Hz)
#define NUM_CHANNELS 2  // Estéreo (2 canais)
#define BUFFER_SIZE 1024  // Tamanho do buffer (ajustado conforme parâmetros)
#define PERIODS 4  // Número de períodos (ajustado conforme parâmetros)

int escrever_wav_estereo(const char *filename, short *sinal, int tamanho) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao abrir o arquivo WAV para escrita: %s\n", filename);
        return -1;
    }

    // Escrever cabeçalho WAV simples (44 bytes)
    unsigned int chunk_size = 36 + tamanho * 2 * sizeof(short);  // 2 canais
    unsigned short audio_format = 1; // PCM
    unsigned short num_channels = 2; // Estéreo
    unsigned int sample_rate = SAMPLE_RATE;
    unsigned int byte_rate = SAMPLE_RATE * 2 * sizeof(short);  // 2 canais
    unsigned short block_align = 2 * sizeof(short);  // 2 canais
    unsigned short bits_per_sample = 16;

    // Cabeçalho RIFF
    fwrite("RIFF", sizeof(char), 4, file);
    fwrite(&chunk_size, sizeof(unsigned int), 1, file);
    fwrite("WAVE", sizeof(char), 4, file);

    // Sub-chunk 1 "fmt "
    fwrite("fmt ", sizeof(char), 4, file);
    unsigned int sub_chunk1_size = 16;
    fwrite(&sub_chunk1_size, sizeof(unsigned int), 1, file);
    fwrite(&audio_format, sizeof(unsigned short), 1, file);
    fwrite(&num_channels, sizeof(unsigned short), 1, file);
    fwrite(&sample_rate, sizeof(unsigned int), 1, file);
    fwrite(&byte_rate, sizeof(unsigned int), 1, file);
    fwrite(&block_align, sizeof(unsigned short), 1, file);
    fwrite(&bits_per_sample, sizeof(unsigned short), 1, file);

    // Sub-chunk 2 "data"
    unsigned int sub_chunk2_size = tamanho * 2 * sizeof(short);  // 2 canais
    fwrite("data", sizeof(char), 4, file);
    fwrite(&sub_chunk2_size, sizeof(unsigned int), 1, file);
    // Escrever o sinal filtrado em estéreo
    fwrite(sinal, sizeof(short), tamanho * 2, file);

    fclose(file);
    return 0;
}

int main() {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S24_3LE;  // Formato S24_3LE
    unsigned int rate = SAMPLE_RATE;
    int channels = NUM_CHANNELS;  // Estéreo
    snd_pcm_uframes_t buffer_size = 264600;  // Ajuste para um valor maior, se necessário

    int periods = PERIODS;  // Número de períodos

    // Abrir o dispositivo de captura
    if (snd_pcm_open(&pcm_handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo %s\n", DEVICE);
        return -1;
    }

    // Alocar memória para os parâmetros de hardware
    snd_pcm_hw_params_malloc(&params);

    // Configurar parâmetros de hardware
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, format);
    snd_pcm_hw_params_set_channels(pcm_handle, params, channels);  // Estéreo
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &buffer_size, 0);
    snd_pcm_hw_params_set_periods_near(pcm_handle, params, &periods, 0);

    // Aplicar os parâmetros ao dispositivo
    if (snd_pcm_hw_params(pcm_handle, params) < 0) {
        fprintf(stderr, "Erro ao configurar os parâmetros de hardware\n");
        return -1;
    }

    // Liberar a memória alocada para os parâmetros
    snd_pcm_hw_params_free(params);

    // Buffer para captura de dados
    short *buffer = (short *) malloc(buffer_size * channels * sizeof(short)); // 16 bits por amostra (S24_3LE)

    if (buffer == NULL) {
        perror("Erro ao alocar memória para o buffer");
        return -1;
    }

    // Variáveis de controle de tempo
    time_t start_time = time(NULL);  // Tempo inicial

    printf("Iniciando captura de áudio...\n");

    int total_frames = 0;

    // Abrir arquivo WAV para gravação
    FILE *wav_file = fopen(PCM_CAPTURE_FILE, "wb");
    if (!wav_file) {
        printf("Erro ao abrir o arquivo para escrita!\n");
        return -1;
    }

    // Escrever cabeçalho WAV inicialmente
    unsigned int chunk_size = 36 + total_frames * 2 * sizeof(short);  // 2 canais
    unsigned short audio_format = 1; // PCM
    unsigned short num_channels = 2; // Estéreo
    unsigned int sample_rate = SAMPLE_RATE;
    unsigned int byte_rate = SAMPLE_RATE * 2 * sizeof(short);  // 2 canais
    unsigned short block_align = 2 * sizeof(short);  // 2 canais
    unsigned short bits_per_sample = 16;

    fwrite("RIFF", sizeof(char), 4, wav_file);
    fwrite(&chunk_size, sizeof(unsigned int), 1, wav_file);
    fwrite("WAVE", sizeof(char), 4, wav_file);

    fwrite("fmt ", sizeof(char), 4, wav_file);
    unsigned int sub_chunk1_size = 16;
    fwrite(&sub_chunk1_size, sizeof(unsigned int), 1, wav_file);
    fwrite(&audio_format, sizeof(unsigned short), 1, wav_file);
    fwrite(&num_channels, sizeof(unsigned short), 1, wav_file);
    fwrite(&sample_rate, sizeof(unsigned int), 1, wav_file);
    fwrite(&byte_rate, sizeof(unsigned int), 1, wav_file);
    fwrite(&block_align, sizeof(unsigned short), 1, wav_file);
    fwrite(&bits_per_sample, sizeof(unsigned short), 1, wav_file);

    fwrite("data", sizeof(char), 4, wav_file);
    unsigned int sub_chunk2_size = total_frames * 2 * sizeof(short);  // 2 canais
    fwrite(&sub_chunk2_size, sizeof(unsigned int), 1, wav_file);

    while (1) {
        // Verifica se o tempo de captura foi atingido
        if (difftime(time(NULL), start_time) >= CAPTURE_TIME) {
            printf("Tempo de captura de %d segundos alcançado. Finalizando...\n", CAPTURE_TIME);
            break;
        }

        int frames = snd_pcm_readi(pcm_handle, buffer, buffer_size);
        if (frames < 0) {
            fprintf(stderr, "Erro de captura: %s\n", snd_strerror(frames));
            break;
        }

        // Escrever os dados no arquivo WAV
        fwrite(buffer, sizeof(short), frames * 2, wav_file);

        total_frames += frames;
    }

    // Atualizar o tamanho do chunk no início
    fseek(wav_file, 4, SEEK_SET);
    chunk_size = 36 + total_frames * 2 * sizeof(short);
    fwrite(&chunk_size, sizeof(unsigned int), 1, wav_file);

    fseek(wav_file, 40, SEEK_SET);
    sub_chunk2_size = total_frames * 2 * sizeof(short);
    fwrite(&sub_chunk2_size, sizeof(unsigned int), 1, wav_file);

    // Fechar arquivo WAV
    fclose(wav_file);

    // Fechar o dispositivo
    snd_pcm_close(pcm_handle);
    free(buffer);

    printf("Captura de áudio finalizada e salva em %s\n", PCM_CAPTURE_FILE);

    return 0;
}
