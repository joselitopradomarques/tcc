#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

/* PCF8591 address, max SCL clock frequency and WRITE and READ definitions */
#define PCF8591_ADDR  0x48
#define TWI_BITRATE   100000
#define TWI_WRITE     0
#define TWI_READ      1
#define DEBOUNCE_DELAY 10  // Tempo de debouncing em ms
#define NUM_READINGS 5    // Número de leituras para o filtro de média móvel

/* Private TWI hardware methods */
static void _twi_init (void);
static void _twi_start (void);
static void _twi_stop (void);
static void _twi_write (uint8_t data);
static uint8_t _twi_read_ack (void);
static uint8_t _twi_read_no_ack (void);

/* Variáveis globais */
uint8_t readings[NUM_READINGS];  // Buffer para armazenar as leituras
uint8_t index = 0;               // Índice do buffer
uint8_t last_value = 0;          // Último valor estável lido

/* Função para calcular a média móvel */
uint8_t moving_average(void) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < NUM_READINGS; i++) {
        sum += readings[i];
    }
    return sum / NUM_READINGS;
}

/* Inicializa o PCF8591 */
void pcf8591_init (void) {
    _twi_init();
}

/* Função para ler o valor analógico do canal do PCF8591 */
uint8_t pcf8591_analog_read (uint8_t channel) {
    _twi_start();
    _twi_write(PCF8591_ADDR << 1);  // Endereço do PCF8591 com bit de escrita
    _twi_write(channel);             // Seleciona o canal
    _twi_stop();
    _twi_start();
    _twi_write((PCF8591_ADDR << 1) | TWI_READ);  // Endereço com bit de leitura
    _twi_read_ack();
    uint8_t result = _twi_read_no_ack();
    _twi_stop();
    return result;
}

/* Inicializa o barramento TWI */
static void _twi_init(void) {
    TWSR = 0;
    TWBR = (F_CPU / (TWI_BITRATE) - 16) / 2;
    TWCR = (1 << TWEN);
}

/* Envia a operação START do mestre */
static void _twi_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
}

/* Envia a operação STOP do mestre */
static void _twi_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

/* Envia dados pelo barramento */
static void _twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
}

/* Lê dados com ACK (para múltiplas leituras) */
static uint8_t _twi_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while(!(TWCR & (1 << TWINT)));
    return TWDR;
}

/* Lê dados sem ACK (para a última leitura) */
static uint8_t _twi_read_no_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
    return TWDR;
}

/* Função principal */
int main(void) {
    pcf8591_init();

    uint8_t current_value;
    uint8_t stable_value = 0;
    uint8_t stable_count = 0;
    uint8_t debounce_threshold = 5;  // Número de leituras consecutivas para estabilidade

    // Inicializa o buffer de leituras
    for (uint8_t i = 0; i < NUM_READINGS; i++) {
        readings[i] = 0;
    }

    while (1) {
        current_value = pcf8591_analog_read(0);  // Lê o valor do canal A0

        // Atualiza o buffer de leituras para a média móvel
        readings[index] = current_value;
        index = (index + 1) % NUM_READINGS;

        // Calcula a média móvel
        uint8_t avg_value = moving_average();

        // Verifica se o valor foi estabilizado por debouncing
        if (avg_value == stable_value) {
            stable_count++;
        } else {
            stable_count = 0;
        }

        // Se a leitura for estável por um número suficiente de amostras
        if (stable_count >= debounce_threshold) {
            printf("Valor estável: %d\n", avg_value);
            stable_value = avg_value;  // Atualiza o valor estável
            stable_count = 0;  // Reseta a contagem de estabilidade
        }

        _delay_ms(DEBOUNCE_DELAY);  // Delay para debouncing
    }

    return 0;
}
