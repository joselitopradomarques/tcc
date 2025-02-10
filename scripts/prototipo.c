int processar_buffers_circulares(short ***buffers_sinal1, short ***buffers_sinal2, int num_buffers, int buffer_size, float *coeficientes_filtro, int ordem_filtro) {

    const char *filename = "sinal_processado.wav";
    const char *device = "default"; // Dispositivo ALSA
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
    char *audio_buffer;
    
    // Inicialize o stream de áudio ALSA
    if (inicializar(device, &pcm_handle, &audio_buffer, &hw_params) != 0) {
        printf("Erro ao inicializar o stream de áudio\n");
        return -1;
    }

    // Definição para efeito Reverb
    float wetness = 0.0f; // Defina o valor apropriado para o efeito

    // Verificar se os buffers estão alocados corretamente
    if (!buffers_sinal1 || !buffers_sinal2) {
        printf("Erro: buffers não alocados corretamente.\n");
        return -1;  // Retorna erro
    }

    // Alocar buffers filtrados
    short **buffers_sinal1_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    short **buffers_sinal2_filtrado = (short **)malloc(num_buffers * sizeof(short *));
    if (!buffers_sinal1_filtrado || !buffers_sinal2_filtrado) {
        printf("Erro: falha ao alocar buffers filtrados.\n");
        return -1;  // Retorna erro
    }

    // Inicializar buffers filtrados
    for (int i = 0; i < num_buffers; i++) {
        buffers_sinal1_filtrado[i] = (short *)malloc(buffer_size * sizeof(short));
        buffers_sinal2_filtrado[i] = (short *)malloc(buffer_size * sizeof(short));
        if (!buffers_sinal1_filtrado[i] || !buffers_sinal2_filtrado[i]) {
            printf("Erro: falha ao alocar memória para buffer de sinal.\n");
            return -1;  // Retorna erro
        }
    }

    // Alocar array para o sinal completo (todos os buffers combinados)
    int tamanho_total_sinal = num_buffers * buffer_size;  // Total de amostras no sinal final
    short *sinal_completo = (short *)malloc(tamanho_total_sinal * sizeof(short));
    if (!sinal_completo) {
        printf("Erro: falha ao alocar o array do sinal completo.\n");
        return -1;  // Retorna erro
    }

    // Processar os buffers, aplicar o filtro FIR e o delay
    int posicao = 0;  // Variável para controle da posição no sinal_completo
    for (int i = 0; i < num_buffers; i++) {
        // Atualizar coeficientes a cada 10 buffers (se necessário)
        if (i % 10 == 0) {
            // altere_coeficientes(coeficientes_filtro, nova_ordem_filtro);
        }

        // Aplicar o filtro FIR para o sinal1
        if ((*buffers_sinal1)[i] != NULL) {
            aplicar_filtro_FIR_buffer((*buffers_sinal1)[i], buffers_sinal1_filtrado[i], buffer_size, coeficientes_filtro, ordem_filtro);
        }

        // Aplicar o filtro FIR para o sinal2
        if ((*buffers_sinal2)[i] != NULL) {
            aplicar_filtro_FIR_buffer((*buffers_sinal2)[i], buffers_sinal2_filtrado[i], buffer_size, coeficientes_filtro, ordem_filtro);
        }

        // Calcular a média dos buffers filtrados e preencher media_buffers
        float *media_buffer = (float *)malloc(buffer_size * sizeof(float));  // Alocar buffer temporário para a média do buffer atual
        for (int j = 0; j < buffer_size; j++) {
            media_buffer[j] = (float)(buffers_sinal1_filtrado[i][j] + buffers_sinal2_filtrado[i][j]) / 2.0f;
        }

        // Aplicar o delay no buffer de média (feedback = 0.6f)
        // aplicar_delay(media_buffer, buffer_size, wetness, 0.3f);
        applyReverbEffectBuffer(media_buffer, buffer_size, wetness, 0.6f);

        // Copiar o buffer processado para o sinal completo
        for (int j = 0; j < buffer_size; j++) {
            sinal_completo[posicao++] = (short)media_buffer[j];  // Converter de volta para short antes de adicionar ao sinal completo
        }

        // Reproduzir o buffer de áudio processado
        if (reproduzir(pcm_handle, media_buffer, buffer_size) != 1) {
            printf("Erro ao reproduzir o áudio no buffer %d.\n", i);
        }

        // Liberar memória do buffer de média após o uso
        free(media_buffer);
    }

    // Salvar o sinal completo no arquivo WAV
    if (escrever_wav_estereo(filename, sinal_completo, tamanho_total_sinal) != 0) {
        printf("Erro ao salvar o arquivo WAV.\n");
        return -1;  // Retorna erro
    }

    // Finalizar o stream de áudio e liberar os recursos
    finalizar(pcm_handle, audio_buffer);

    // Liberação dos buffers filtrados e sinal completo
    for (int i = 0; i < num_buffers; i++) {
        free(buffers_sinal1_filtrado[i]);
        free(buffers_sinal2_filtrado[i]);
    }
    free(buffers_sinal1_filtrado);
    free(buffers_sinal2_filtrado);
    free(sinal_completo);

    return 0;  // Retorna sucesso
}
