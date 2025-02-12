#ifndef ADC_H
#define ADC_H

// Função para inicializar os sensores e o GPIO
int setup_sensors();

// Função para ler os valores analógicos de AIN0 e AIN1
void read_analog_values(int fd, int *analogValue0, int *analogValue1);

// Função para ler o valor digital do GPIO4
int read_digital_value();

#endif /* ADC_H */
