#ifndef PROC_H
#define PROC_H

// Função para ler um arquivo WAV estéreo e retornar o sinal em formato de array de shorts
int ler_wav_estereo(const char *filename, short **sinal, int *tamanho);

// Função para ler dois arquivos WAV estéreo e retornar os sinais em formato de array de shorts
int ler_dois_wav_estereo(short **sinal1, short **sinal2, int *tamanho1, int *tamanho2);

// Função para gerar buffers circulares a partir dos sinais de áudio
int gerar_buffers_circulares(short *sinal1, short *sinal2, int tamanho, int buffer_size, short ***buffers_sinal1, short ***buffers_sinal2, int *num_buffers);

// Função para aplicar um filtro exemplo nos buffers
void filtro_exemplo(short *buffer, int buffer_size);

// Função para processar os buffers de sinal1 e sinal2
int processar_buffers_circulares(short ***buffers_sinal1, short ***buffers_sinal2, int num_buffers, int buffer_size);

// Função para liberar a memória dos buffers
void liberar_buffers(short **buffers_sinal1, short **buffers_sinal2, int num_buffers);

#endif // PROC_H
