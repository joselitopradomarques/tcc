#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define FRAME_SIZE 1024
#define CHANNELS 2
#define SAMPLE_SIZE 4  // 32 bits = 4 bytes
#define SHM_PATH "/dev/shm/audio_shared"  // Caminho para a memória compartilhada

int main() {
    // Abre o arquivo mapeado na memória compartilhada
    int fd = open(SHM_PATH, O_RDWR);
    if (fd == -1) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Mapeia o arquivo para a memória
    char *mapped_memory = mmap(NULL, FRAME_SIZE * CHANNELS * SAMPLE_SIZE + 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_memory == MAP_FAILED) {
        perror("Erro ao mapear a memória");
        close(fd);
        return 1;
    }

    // Aqui, o código C vai continuamente ler os dados da memória compartilhada
    printf("C: Lendo dados da memória compartilhada continuamente...\n");

    // Buffer para armazenar dados lidos
    unsigned char buffer[FRAME_SIZE * CHANNELS * SAMPLE_SIZE];

    while (1) {
        // Verifica se os dados estão prontos (flag == 1)
        if (mapped_memory[0] == 1) {
            // Lê os dados da memória compartilhada diretamente
            memcpy(buffer, mapped_memory + 4, FRAME_SIZE * CHANNELS * SAMPLE_SIZE);

            // Processa os dados (aqui estamos imprimindo os bytes como exemplo)
            printf("C: Dados recebidos da memória compartilhada:\n");
            for (int i = 0; i < FRAME_SIZE * CHANNELS * SAMPLE_SIZE; i++) {
                printf("Byte %d: %d\n", i, buffer[i]);
            }

            // Reseta a flag para 0 (indicando que a leitura foi realizada)
            mapped_memory[0] = 0;
            printf("C: Flag resetada para 0. Esperando nova escrita...\n");  // Debug
        }

        // Opcional: Adicione um pequeno delay para não consumir 100% da CPU
        usleep(100000);  // 100ms
    }

    // Desmapeia a memória compartilhada
    if (munmap(mapped_memory, FRAME_SIZE * CHANNELS * SAMPLE_SIZE + 4) == -1) {
        perror("Erro ao desmapear a memória");
    }

    // Fecha o arquivo
    close(fd);

    return 0;
}
