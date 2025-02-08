#include "audio.h"
#include <stdio.h>
#include <alsa/asoundlib.h>

int main() {
    const char *audio_file = "/home/joselito/git/tcc/scripts/sinal_processado.wav"; // Substitua pelo caminho do seu arquivo .wav
    const char *device = "hw:2,0"; // Dispositivo de áudio hw:2,0
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
    FILE *file;
    char *buffer;

    // Inicialização
    if (inicializar(audio_file, device, &pcm_handle, &file, &buffer, &hw_params) != 0) {
        fprintf(stderr, "Falha na inicialização.\n");
        return -1;
    }

    // Reprodução
    while (reproduzir(pcm_handle, file, buffer)) {
        // Continuação da reprodução até o fim do arquivo
    }

    // Finalização
    finalizar(pcm_handle, file, buffer);

    printf("Reprodução concluída!\n");
    return 0;
}
