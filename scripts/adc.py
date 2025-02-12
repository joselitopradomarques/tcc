import time
import RPi.GPIO as GPIO
import smbus2 as smbus


# Definir o endereço I2C do PCF8591
PCF8591_ADDR = 0x48  # Endereço I2C do PCF8591 (0x48)
GPIO_PIN = 4         # Pino GPIO 4 para leitura digital

# Inicializar a biblioteca GPIO e configurar os pinos
GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)  # Configura o GPIO4 com pull-up

# Inicializar o barramento I2C
bus = smbus.SMBus(1)

# Função para ler o valor analógico de um canal AIN (0 ou 1)
def read_analog(channel):
    # Enviar o comando para o PCF8591
    bus.write_byte(PCF8591_ADDR, 0x40 | channel)  # 0x40 é o comando de leitura
    time.sleep(0.1)  # Atraso para permitir que o PCF8591 tenha tempo para responder
    value = bus.read_byte(PCF8591_ADDR)  # Lê o valor analógico
    return value

# Loop principal
try:
    while True:
        # Leitura dos canais analógicos AIN0 e AIN1
        analog_value0 = read_analog(0)  # AIN0
        analog_value1 = read_analog(1)  # AIN1

        # Leitura digital do pino GPIO 4
        digital_value = GPIO.input(GPIO_PIN)

        # Exibe os valores lidos
        print(f"AIN0: {analog_value0} | AIN1: {analog_value1} | GPIO 4: {digital_value}")

        # Atraso de 1 segundo entre as leituras
        time.sleep(1)

except KeyboardInterrupt:
    print("\nPrograma interrompido pelo usuário.")
finally:
    GPIO.cleanup()  # Limpar configurações do GPIO ao finalizar o programa
