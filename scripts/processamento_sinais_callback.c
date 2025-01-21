#include <stdio.h>
#include <stdlib.h>
#include "filter_final.h"  // Para filtragem
#include "fx.h"            // Para efeitos como delay, reverb
#include "delay.h"         // Efeito de delay
#include "reverb.h"        // Efeito de reverb

// Função para processar áudio com efeitos usando buffers diretamente
void processar_audio(short *sinal_estereo, int tamanho, short **sinal_filtrado, short **sinal_com_efeitos, 
                     float frequencia_corte, float delay_time, float reverb_decay) {
    // Alocação de memória para o sinal filtrado
    *sinal_filtrado = (short *)malloc(tamanho * 2 * sizeof(short));
    if (!(*sinal_filtrado)) {
        printf("Erro ao alocar memória para o sinal filtrado\n");
        return;
    }

    // Etapa 1: Aplicação de filtragem (usando filtro FIR)
    float coeficientes[ORDEM];
    gerar_filtro_FIR(coeficientes, ORDEM, frequencia_corte, SAMPLE_RATE);
    aplicar_filtro_FIR(sinal_estereo, *sinal_filtrado, tamanho * 2, coeficientes, ORDEM);

    // Etapa 2: Aplicação de efeitos de delay e reverb
    *sinal_com_efeitos = (short *)malloc(tamanho * 2 * sizeof(short));
    if (!(*sinal_com_efeitos)) {
        printf("Erro ao alocar memória para o sinal com efeitos\n");
        free(*sinal_filtrado);
        return;
    }

    // Aplicar efeito de delay
    aplicar_delay(*sinal_filtrado, *sinal_com_efeitos, tamanho, delay_time);

    // Aplicar efeito de reverb
    aplicar_reverb(*sinal_com_efeitos, tamanho, reverb_decay);

    // Aqui, você pode realizar qualquer outro processo, como somar sinais ou salvar em arquivos WAV,
    // mas isso não será feito nesta função, já que estamos utilizando buffers diretamente.

    // Não é mais necessário realizar leitura ou escrita de arquivos WAV aqui.
    
    // A memória alocada será liberada após o processamento.
    // Liberação de memória (sempre importante para evitar vazamento de memória)
    free(*sinal_filtrado);
    free(*sinal_com_efeitos);

    printf("Processamento de áudio concluído com sucesso!\n");
}
