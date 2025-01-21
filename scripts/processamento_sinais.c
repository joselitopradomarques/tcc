// processamento_sinais.c

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "processamento_sinais.h"
#include "delay.h"
#include "reverb.h"

// Função para somar dois sinais de entrada
void somar_sinais(short *sinal1, short *sinal2, short *resultado, int tamanho) {
    if (!sinal1 || !sinal2 || !resultado) {
        printf("Erro: memória não alocada para os sinais ou para o resultado.\n");
        return;
    }

    for (int i = 0; i < tamanho; i++) {
        resultado[i] = sinal1[i] + sinal2[i];
        if (resultado[i] > SHRT_MAX) {
            resultado[i] = SHRT_MAX;
        } else if (resultado[i] < SHRT_MIN) {
            resultado[i] = SHRT_MIN;
        }
    }
}

// Função para executar um arquivo WAV no Linux usando 'aplay'
void executar_arquivo_wav(const char* arquivo) {
    // Exibe a mensagem informando que o sinal de saída filtrado e mixado está sendo reproduzido
    printf("\n---- Seção de reprodução:\n");
    printf("Sinal de saída filtrado e mixado sendo reproduzido\n");

    char comando[256];
    snprintf(comando, sizeof(comando), "aplay %s > /dev/null 2>&1", arquivo);
    system(comando);  // Executa o comando para reproduzir o áudio
}


// Função para aplicar um efeito ao áudio (delay ou reverb)
void apply_effect_to_audio(int effect_choice, float effect_amount) {
    const char *input_file = "/home/joselito/git/tcc/datas/audio01.wav";  // Caminho do arquivo de entrada
    const char *output_file_delay = "/home/joselito/git/tcc/scripts/audio_delay.wav";  // Arquivo de saída do delay
    const char *output_file_reverb = "/home/joselito/git/tcc/scripts/audio_reverb.wav";  // Arquivo de saída do reverb

    // Exibindo informações da seção de efeitos
    printf("---- Seção de efeitos:\n");
    printf("fx_select = %d\n", effect_choice);
    printf("effect_amount = %.2f\n", effect_amount);
    
    if (effect_choice == 0) {
        // Converte effect_amount em milissegundos (considerando 1000 ms como máximo)
        int delay_time = (int)(effect_amount * 1000);  // Tempo do delay em milissegundos
        apply_delay_to_audio(input_file, output_file_delay, effect_amount);

        // Exibe a quantidade de milissegundos do delay
        printf("Efeito Delay selecionado com %d ms de tempo de delay\n", delay_time);
    } else if (effect_choice == 1) {
        applyReverbEffect(input_file, output_file_reverb, effect_amount);
        printf("Efeito Reverb selecionado com %.2f de quantidade de wetness\n", effect_amount);
    } else {
        printf("Escolha inválida. Use 0 para delay e 1 para reverb.\n");
    }
}



void processar_audio(char* buffer, int size, float frequencia_corte_i) {
    // Função onde o processamento de áudio será feito
    printf("Processando áudio...\n");

    // Definir os coeficientes do filtro FIR
    float coeficientes[ORDEM];

    // Gerar o filtro FIR com os coeficientes baseados na frequência de corte
    gerar_filtro_FIR(coeficientes, ORDEM, frequencia_corte_i, SAMPLE_RATE);
    
    // O buffer de áudio precisa ser tratado de acordo com o tipo de dado (por exemplo, short ou float)
    // Aqui assumimos que o buffer é um array de shorts (16 bits)
    short* sinal_filtrado = (short*)malloc(size * sizeof(short));
    if (!sinal_filtrado) {
        printf("Erro ao alocar memória para o sinal filtrado\n");
        return;
    }

    // Aplicar o filtro FIR no buffer de áudio
    aplicar_filtro_FIR((short*)buffer, sinal_filtrado, size, coeficientes, ORDEM);

    // Aqui você pode adicionar outras operações no sinal filtrado, como delay, reverb, etc.

    // Liberar memória
    free(sinal_filtrado);
}


    sinal_soma = (short *)malloc(tamanho[0] * 2 * sizeof(short));  // 2 canais (estéreo)
    if (!sinal_soma) {
        printf("Erro ao alocar memória para o sinal da soma\n");
        for (int i = 0; i < 2; i++) {
            free(sinal_estereo[i]);
            free(sinal_filtrado[i]);
        }
        return;
    }

    somar_sinais(sinal_filtrado[0], sinal_filtrado[1], sinal_soma, tamanho[0] * 2);
    if (escrever_wav_estereo(output_files[2], sinal_soma, tamanho[0]) != 0) {
        for (int i = 0; i < 2; i++) {
            free(sinal_estereo[i]);
            free(sinal_filtrado[i]);
        }
        free(sinal_soma);
        return;
    }

    for (int i = 0; i < 2; i++) {
        free(sinal_estereo[i]);
        free(sinal_filtrado[i]);
    }
    free(sinal_soma);


    // Print comentado por já estar debugado
    //printf("Filtragem e soma concluídas com sucesso!\n");
}


// Função unificada para aplicar filtragem e efeito
void aplicar_filtragem_e_efeito(float frequencia_corte, int fx_select, float effectAmount) {
    executar_filtragem_e_soma(frequencia_corte);
    apply_effect_to_audio(fx_select, effectAmount);
    executar_arquivo_wav("/home/joselito/git/tcc/scripts/saida_filtrada_soma.wav");
}
