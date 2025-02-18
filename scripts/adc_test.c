#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>

#define PCF8591_ADDR 0x48 // Endereço I2C do PCF8591 (0x48)
#define GPIO_PIN 7       // Pino GPIO 4 no WiringPi (usando a numeração do WiringPi)

int main() {
    int fd;
    int gpioPin = GPIO_PIN;
    int digitalValue;
    int analogValue0, analogValue1;

    // Inicializa a biblioteca wiringPi
    if (wiringPiSetup() == -1) {
        printf("Erro ao inicializar o wiringPi\n");
        return 1;
    }

    // Configura o pino GPIO 4 (ou 7 no WiringPi) como entrada com pull-up
    pinMode(gpioPin, INPUT);
    pullUpDnControl(gpioPin, PUD_UP);  // Configura o GPIO com pull-up

    // Inicializa a comunicação I2C com o PCF8591
    fd = wiringPiI2CSetup(PCF8591_ADDR);
    if (fd == -1) {
        printf("Erro ao inicializar o PCF8591\n");
        return 1;
    }

    printf("Leitura do PCF8591 e GPIO 4\n");

    while (1) {
        // Leitura dos canais analógicos AIN0 e AIN1
        // Para o canal AIN0 (0x40) e AIN1 (0x41)
        analogValue0 = wiringPiI2CReadReg8(fd, 0x40); // Leitura de AIN0
        analogValue1 = wiringPiI2CReadReg8(fd, 0x41); // Leitura de AIN1

        // Leitura digital do pino GPIO 4 (agora usando wiringPi numeração)
        digitalValue = digitalRead(gpioPin);

        // Exibe os valores lidos
        printf("AIN0: %d | AIN1: %d | GPIO 4: %d\n", analogValue0, analogValue1, digitalValue);

        // Atraso de 1 segundo entre as leituras
        sleep(1);
    }

    return 0;
}
