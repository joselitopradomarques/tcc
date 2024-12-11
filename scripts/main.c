#include <stdio.h>
#include <stdlib.h>
#include "filter_final.h"
#include "processamento_sinais.h"  // Incluir o cabeçalho onde somar_sinais está declarada
#include "delay.h"  // Incluir o delay
#include "reverb.h" // Incluir o reverb
#include <sndfile.h>

int main() {
    // Definindo os caminhos dos arquivos de entrada e saída
    const char *input_files[] = {
        "/home/joselito/git/tcc/datas/audio01.wav",
        "/home/joselito/git/tcc/datas/audio02.wav"
    };
    const char *output_files[] = {
        "/home/joselito/git/tcc/scripts/saida_filtrada_1.wav",
        "/home/joselito/git/tcc/scripts/saida_filtrada_2.wav",
        "/home/joselito/git/tcc/scripts/saida_filtrada_soma.wav" // Arquivo para a soma
    };
    const char *output_files_with_delay[] = {
        "/home/joselito/git/tcc/scripts/saida_soma_delay.wav" // Novo arquivo para a soma com delay
    };
        const char *output_files_with_reverb[] = {
        "/home/joselito/git/tcc/scripts/saida_soma_reverb.wav" // Novo arquivo para a soma com reverb
    };

    // Definindo os ponteiros para os sinais
    short *sinal_estereo[2] = {NULL, NULL};
    short *sinal_filtrado[2] = {NULL, NULL};
    short *sinal_soma = NULL;
    int tamanho[2] = {0, 0};

    // Frequência de corte do primeiro arquivo (inicialmente definida)
    float frequencia_corte1 = 2000.0;  // Definida diretamente no código

    // Loop para processar os dois arquivos
    for (int i = 0; i < 2; i++) {
        // Frequência de corte ajustada para cada arquivo
        float frequencia_corte_atual = (i == 0) ? frequencia_corte1 : calcular_nova_frequencia_corte(frequencia_corte1);

        // Leitura do arquivo atual
        if (ler_wav_estereo(input_files[i], &sinal_estereo[i], &tamanho[i]) != 0) {
            // Liberar memória alocada caso haja erro
            for (int j = 0; j < i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return -1;
        }

        // Alocar memória para o sinal filtrado
        sinal_filtrado[i] = (short *)malloc(tamanho[i] * 2 * sizeof(short));  // 2 canais (estéreo)
        if (!sinal_filtrado[i]) {
            printf("Erro ao alocar memória para o sinal filtrado %d\n", i + 1);
            for (int j = 0; j <= i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return -1;
        }

        // Gerar os coeficientes do filtro FIR para o arquivo atual
        float coeficientes[ORDEM];
        gerar_filtro_FIR(coeficientes, ORDEM, frequencia_corte_atual, SAMPLE_RATE);

        // Aplicar o filtro FIR no sinal atual
        aplicar_filtro_FIR(sinal_estereo[i], sinal_filtrado[i], tamanho[i] * 2, coeficientes, ORDEM);

        // Escrever o sinal filtrado no arquivo de saída
        if (escrever_wav_estereo(output_files[i], sinal_filtrado[i], tamanho[i]) != 0) {
            // Liberar memória em caso de erro
            for (int j = 0; j <= i; j++) {
                free(sinal_estereo[j]);
                free(sinal_filtrado[j]);
            }
            return -1;
        }
    }

    // Alocar memória para o sinal da soma
    sinal_soma = (short *)malloc(tamanho[0] * 2 * sizeof(short));  // 2 canais (estéreo)
    if (!sinal_soma) {
        printf("Erro ao alocar memória para o sinal da soma\n");
        for (int i = 0; i < 2; i++) {
            free(sinal_estereo[i]);
            free(sinal_filtrado[i]);
        }
        return -1;
    }

    // Somar os dois sinais filtrados (chamando a função existente)
    somar_sinais(sinal_filtrado[0], sinal_filtrado[1], sinal_soma, tamanho[0] * 2);

    // Escrever o sinal somado no arquivo de saída
    if (escrever_wav_estereo(output_files[2], sinal_soma, tamanho[0]) != 0) {
        // Liberar memória em caso de erro
        for (int i = 0; i < 2; i++) {
            free(sinal_estereo[i]);
            free(sinal_filtrado[i]);
        }
        free(sinal_soma);
        return -1;
    }

    // Aplicar o delay ao sinal somado e gravar no arquivo de saída com delay
    // Aqui ajustamos o tempo de delay conforme desejado (em milissegundos)
    int delay_time = 1;  // Exemplo: delay de 500 ms
    apply_delay_to_audio(input_files[0], output_files_with_delay[0], delay_time);

    // Define o efeito de reverb com o valor entre 0.0 e 1.0, onde 1.0 corresponde ao feedback máximo (100 dB)
    float effectAmount = 0.1f;  // Ajuste para variar entre 0.0 (sem efeito) e 1.0 (feedback máximo)

    // Chama a função de aplicação do efeito com o parâmetro effectAmount
    applyReverbEffect(input_files[0], output_files_with_reverb[0], effectAmount);


    // Liberar memória após o processamento
    for (int i = 0; i < 2; i++) {
        free(sinal_estereo[i]);
        free(sinal_filtrado[i]);
    }
    free(sinal_soma);

    printf("Processamento concluído com sucesso!\n");
    return 0;
}