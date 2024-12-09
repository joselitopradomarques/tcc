#include <stdio.h>
#include <stdlib.h>
#include <speex/speex_preprocess.h>

#define FRAME_SIZE 160  // Tamanho do quadro para processamento

int main() {
    // Inicializa o estado do processador de áudio
    SpeexPreprocessState *state = speex_preprocess_state_init(FRAME_SIZE, 8000);  // Exemplo com 8kHz de taxa de amostragem
    if (!state) {
        fprintf(stderr, "Erro ao inicializar o estado do pre-processador\n");
        return -1;
    }

    // Vetor de entrada de áudio (simulação)
    spx_int16_t input[FRAME_SIZE] = {0};  // Inicializa com zeros

    // Rodar o pré-processamento
    int ret = speex_preprocess_run(state, input);  // Apenas passa o vetor de entrada
    if (ret != 1) {
        fprintf(stderr, "Erro no processamento\n");
        speex_preprocess_state_destroy(state);
        return -1;
    }

    // Aqui você pode processar o áudio ou fazer outras operações

    // Finaliza o estado do pre-processador
    speex_preprocess_state_destroy(state);

    printf("Áudio processado com sucesso\n");

    return 0;
}
