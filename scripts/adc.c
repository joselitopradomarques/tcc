#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define PCF8591_ADDRESS 0x48 // Endereço I2C do PCF8591 (verifique com o i2cdetect)
#define CHANNEL_A0 0x48  // Canal A0 (entrada analógica 0)
#define CHANNEL_A1 0x49  // Canal A1 (entrada analógica 1)

int main() {
    int file;
    char *filename = (char*)"/dev/i2c-1";  // Para a Raspberry Pi, normalmente é /dev/i2c-1
    unsigned char buffer[2]; // Para armazenar os dados de leitura
    int value_A0, value_A1;

    // Abre o barramento I2C
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Erro ao abrir o barramento I2C");
        exit(1);
    }

    // Conecta ao dispositivo PCF8591
    if (ioctl(file, I2C_SLAVE, PCF8591_ADDRESS) < 0) {
        perror("Erro ao se conectar ao PCF8591");
        exit(1);
    }

    while (1) {
        // Lê a entrada A0
        buffer[0] = CHANNEL_A0;  // Comando para selecionar A0
        if (write(file, buffer, 1) != 1) {
            perror("Erro ao selecionar A0");
            exit(1);
        }
        // Lê o valor digital da entrada A0
        if (read(file, buffer, 1) != 1) {
            perror("Erro ao ler valor de A0");
            exit(1);
        }
        value_A0 = buffer[0];
        printf("Valor de A0: %d\n", value_A0);

        // Lê a entrada A1
        buffer[0] = CHANNEL_A1;  // Comando para selecionar A1
        if (write(file, buffer, 1) != 1) {
            perror("Erro ao selecionar A1");
            exit(1);
        }
        // Lê o valor digital da entrada A1
        if (read(file, buffer, 1) != 1) {
            perror("Erro ao ler valor de A1");
            exit(1);
        }
        value_A1 = buffer[0];
        printf("Valor de A1: %d\n", value_A1);

        sleep(1);  // Aguarda 1 segundo antes de ler novamente
    }

    // Fecha a comunicação I2C
    close(file);

    return 0;
}
