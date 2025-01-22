import time
import smbus

# Endereço do dispositivo e canal
address = 0x48  # Substitua com o endereço correto, se necessário
channel = 0     # Canal de entrada analógica (0 a 3)

# Função para ler o valor do ADC
def read_adc():
    bus = smbus.SMBus(1)  # Abertura do barramento I2C
    bus.write_byte(address, 0x40 | (channel & 0x03))  # Comando de leitura
    bus.read_byte(address)  # Leitura de comando (descartada)
    adc_value = bus.read_byte(address)  # Leitura do valor ADC
    return adc_value

# Classe para filtro de média móvel
class MovingAverageFilter:
    def __init__(self, window_size):
        self.window_size = window_size  # Tamanho da janela do filtro
        self.values = []

    def apply(self, value):
        self.values.append(value)
        if len(self.values) > self.window_size:
            self.values.pop(0)  # Remove o valor mais antigo se ultrapassar a janela
        return sum(self.values) / len(self.values)  # Retorna a média dos valores na janela

# Função para tirar a média de múltiplas leituras
def get_average_adc_reading(num_samples=10, pause_time=0.01):
    readings = []
    for _ in range(num_samples):
        readings.append(read_adc())
        time.sleep(pause_time)  # Pausa entre leituras para evitar interferência
    return sum(readings) / len(readings)  # Média das leituras

# Função para ler e filtrar os valores do ADC com filtro de média móvel
def read_and_filter(window_size=5, num_samples=10, interval=0.5, pause_time=0.01):
    filter = MovingAverageFilter(window_size=window_size)  # Defina o tamanho da janela do filtro
    
    while True:
        # Leitura crua e média das leituras
        raw_value = get_average_adc_reading(num_samples=num_samples, pause_time=pause_time)  # Média das leituras
        filtered_value = filter.apply(raw_value)  # Aplica o filtro de média móvel
        
        # Exibe a leitura crua e a filtrada
        print(f"Leitura crua (média de {num_samples} amostras): {raw_value:.2f}, Leitura filtrada (janela={window_size}): {filtered_value:.2f}")
        
        # Aguarda o intervalo entre as leituras
        time.sleep(interval)

# Função para iterar diferentes parâmetros de intervalo, pausa e quantidade de samples
def iterate_parameters():
    window_sizes = [3, 5, 7, 10]  # Tamanhos da janela do filtro para testar
    num_samples_list = [5, 10, 20]  # Quantidade de amostras para calcular a média
    interval_list = [0.1, 0.2, 0.5]  # Intervalo de tempo entre as leituras
    pause_time_list = [0.01, 0.02, 0.05]  # Pausa entre as leituras

    # Iterando sobre os parâmetros
    for window_size in window_sizes:
        for num_samples in num_samples_list:
            for interval in interval_list:
                for pause_time in pause_time_list:
                    print(f"Testando: Janela={window_size}, Amostras={num_samples}, Intervalo={interval}s, Pausa={pause_time}s")
                    read_and_filter(window_size=window_size, num_samples=num_samples, interval=interval, pause_time=pause_time)
                    time.sleep(2)  # Aguarda 2 segundos antes de tentar uma nova combinação de parâmetros

# Inicia o processo de iteração com os parâmetros desejados
iterate_parameters()
