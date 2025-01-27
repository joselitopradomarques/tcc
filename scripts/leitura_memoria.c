#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define FRAME_SIZE 1024  // Tamanho do frame para a captura
#define CHANNELS 2
#define SAMPLE_RATE 44100

int main() {
    // Abertura da memória compartilhada
    int shm_fd = open("/dev/shm/audio_shared", O_RDONLY);
    if (shm_fd == -1) {
        perror("Erro ao abrir o arquivo de memória compartilhada");
        return -1;
    }

    // Mapear a memória compartilhada para o processo
    void *shm_ptr = mmap(NULL, FRAME_SIZE * CHANNELS * 4, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Erro ao mapear a memória compartilhada");
        return -1;
    }

    printf("Lendo dados de áudio...\n");

    while (1) {
        // Acessando os dados da memória compartilhada (exemplo de leitura)
        int32_t *audio_data = (int32_t *)shm_ptr;
        for (int i = 0; i < FRAME_SIZE * CHANNELS; i++) {
            printf("%d ", audio_data[i]);
        }
        printf("\n");

        // Aqui você pode processar os dados de áudio ou usá-los em tempo real
        usleep(100000);  // Delay para simular o processamento contínuo
    }

    // Desmapeando e fechando
    munmap(shm_ptr, FRAME_SIZE * CHANNELS * 4);
    close(shm_fd);
    return 0;
}
