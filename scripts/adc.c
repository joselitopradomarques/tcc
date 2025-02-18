#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>  // Para a função qsort

// Configurações iniciais
int setup_sensors() {
    int fd;
    
    // Inicializa a biblioteca WiringPi
    if (wiringPiSetup() == -1) {
        printf("Erro ao inicializar o WiringPi\n");
        return -1;
    }

    // Configura o pino GPIO 4 como entrada com pull-up
    pinMode(4, INPUT);
    pullUpDnControl(4, PUD_UP);

    // Inicializa a comunicação I2C com o PCF8591
    fd = wiringPiI2CSetup(0x48);
    if (fd == -1) {
        printf("Erro ao inicializar o PCF8591\n");
        return -1;
    }

    return fd;
}

// Função para comparar dois números (necessária para qsort)
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Leitura analógica (AIN0 e AIN1) com suavização usando média móvel
void read_analog_values(int fd, int *analogValue0, int *analogValue1) {
    int sum0 = 0, sum1 = 0;
    int num_samples = 5;  // Número de amostras para a média móvel (ajuste conforme necessário)
    
    // Realizar múltiplas leituras e somá-las
    for (int i = 0; i < num_samples; i++) {
        sum0 += wiringPiI2CReadReg8(fd, 0x41);  // Leitura de AIN1
        sum1 += wiringPiI2CReadReg8(fd, 0x40);  // Leitura de AIN0
    }

    // Calcular a média das leituras
    *analogValue0 = sum0 / num_samples;
    *analogValue1 = sum1 / num_samples;
}




// Leitura digital (GPIO4)
int read_digital_value() {
    return digitalRead(7); // Leitura digital do GPIO4
}
