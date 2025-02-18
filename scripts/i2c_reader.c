#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEVICE_ADDR 0x48  // Endereço do PCF8591
#define READ_CMD 0x00     // Comando para ler dados (no caso do PCF8591)
#define DEBOUNCE_THRESHOLD 5  // Número de amostras consecutivas para aceitar a leitura
#define DEBOUNCE_DELAY 100    // Atraso entre as leituras (em milissegundos)
#define A0_CHANNEL_CMD 0x40 // Comando para selecionar o canal A0


int main(void) {
    int fd;
    int value;
    int last_value = -1;
    int consistent_count = 0;

    // Inicializa a biblioteca wiringPi
    if (wiringPiSetup() == -1) {
        printf("Erro ao inicializar o wiringPi\n");
        return -1;
    }

    // Abre o dispositivo I2C
    fd = wiringPiI2CSetup(DEVICE_ADDR);
    if (fd == -1) {
        printf("Erro ao abrir o barramento I2C\n");
        return -1;
    }

    // Loop de leitura contínua
    while (1) {
        // Envia o comando para ler o valor do ADC
        wiringPiI2CWrite(fd, A0_CHANNEL_CMD); // Seleciona o canal A0
        value = wiringPiI2CReadReg8(fd, READ_CMD); // Lê o valor do canal A0
        if (value == -1) {
            printf("Erro ao ler dados do dispositivo\n");
        } else {
            // Verifica se o valor lido é igual ao último valor
            if (value == last_value) {
                // Se o valor for consistente, aumentamos o contador
                consistent_count++;
            } else {
                // Se o valor mudou, reiniciamos o contador
                consistent_count = 0;
            }

            // Se o valor for consistente por um número de amostras, aceitamos a leitura
            if (consistent_count >= DEBOUNCE_THRESHOLD) {
                printf("Valor estabilizado: %d\n", value);
                consistent_count = 0; // Reinicia o contador após a leitura estabilizada
            }

            // Atualiza o último valor lido
            last_value = value;
        }

        // Atraso de debounce
        delay(DEBOUNCE_DELAY);
    }

    return 0;
}
