#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <math.h>

// Definições para o dispositivo I2C e PCF8591
#define I2C_DEVICE "/dev/i2c-1"  // Caminho do dispositivo I2C
#define PCF8591_ADDRESS 0x48     // Endereço I2C do PCF8591 (verifique com i2cdetect)
#define CHANNEL_A0 0x48          // Canal A0 (entrada analógica 0)
#define CHANNEL_A1 0x49          // Canal A1 (entrada analógica 1)

// Prototipagem das funções
int abrir_i2c();
int ler_valor_i2c(int file, unsigned char channel);
void plotar_valores(int valor1, int valor2);

int abrir_i2c() {
    int file = open(I2C_DEVICE, O_RDWR);
    if (file < 0) {
        perror("Erro ao abrir o barramento I2C");
        exit(1);
    }
    return file;
}

int ler_valor_i2c(int file, unsigned char channel) {
    unsigned char config;

    // Definir o canal de entrada (A0 ou A1) no PCF8591
    if (channel == CHANNEL_A0) {
        config = 0x48;  // Configuração para canal A0
    } else if (channel == CHANNEL_A1) {
        config = 0x49;  // Configuração para canal A1
    } else {
        fprintf(stderr, "Canal inválido\n");
        return -1;
    }

    // Define o endereço do dispositivo I2C (PCF8591)
    if (ioctl(file, I2C_SLAVE, PCF8591_ADDRESS) < 0) {
        perror("Erro ao selecionar o dispositivo I2C");
        exit(1);
    }

    // Enviar comando para selecionar o canal
    if (write(file, &config, 1) != 1) {
        perror("Erro ao escrever no dispositivo I2C");
        exit(1);
    }

    // Ler o valor (duas leituras necessárias: a primeira é lixo)
    unsigned char buffer[1];
    
    // Leitura "lixo"
    if (read(file, buffer, 1) != 1) {
        perror("Erro ao ler valor de conversão inicial (lixo)");
        exit(1);
    }

    // Leitura válida
    if (read(file, buffer, 1) != 1) {
        perror("Erro ao ler valor de conversão válida");
        exit(1);
    }

    // O valor retornado é de 8 bits
    return (int)buffer[0];  // Retornar o valor lido (0 a 255)
}

void plotar_valores(int valor1, int valor2) {
    printf("Valores lidos: A0 = %d, A1 = %d\n", valor1, valor2);
    // Aqui você pode adicionar lógica para criar gráficos mais sofisticados se quiser
}

int main() {
    int file = abrir_i2c();  // Abre a conexão I2C

    int intervalo = 0.001;
    //printf("Digite o intervalo (em milissegundos) entre os plots: ");
    //scanf("%d", &intervalo);

    while (1) {
        // Lê os valores digitalizados dos canais A0 e A1
        int valor_a0 = ler_valor_i2c(file, CHANNEL_A0);
        int valor_a1 = ler_valor_i2c(file, CHANNEL_A1);

        // Plota os valores lidos
        plotar_valores(valor_a0, valor_a1);

        // Converte o intervalo de milissegundos para microsegundos e aguarda
        usleep(intervalo * 1000);  
    }

    close(file);  // Fecha o dispositivo I2C

    return 0;
}
