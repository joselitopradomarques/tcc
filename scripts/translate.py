import numpy as np
import matplotlib.pyplot as plt

# Função para ler os valores brutos do arquivo .txt
def ler_valores_brutos_txt(filename):
    # Ler os valores do arquivo e retornar como um array
    with open(filename, 'r') as f:
        valores = [float(line.strip()) for line in f.readlines()]
    return np.array(valores)

# Função para converter valores para dB
def valor_para_db(valor):
    if valor == 0:
        return -np.inf  # Evita erro com log10(0)
    return 20 * np.log10(np.abs(valor))

# Função para converter todos os valores do sinal para dB
def converter_para_db(valores):
    return np.array([valor_para_db(v) for v in valores])

# Função para calcular e plotar o espectro em dB
def plot_spectrum(valores, valores_db, fs, title_original, title_db):
    N = len(valores)
    T = 1.0 / fs
    yf = np.fft.fft(valores)
    xf = np.fft.fftfreq(N, T)[:N//2]
    
    # Magnitude e espectro do sinal original em dB
    magnitude = 2.0 / N * np.abs(yf[:N//2])
    magnitude_db = 20 * np.log10(magnitude)  # Converte para dB
    
    # Evitar problemas logarítmicos com valores zero ou negativos
    magnitude_db[magnitude_db == -np.inf] = -100  # Definir um valor mínimo para dB
    
    # Espectro do sinal original em dB
    plt.figure(figsize=(12, 6))
    plt.subplot(2, 1, 1)
    plt.semilogx(xf, magnitude_db)
    plt.title(title_original)
    plt.xlabel('Frequência (Hz) [Escala Logarítmica]')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    
    # Espectro do sinal em dB após conversão
    yf_db = np.fft.fft(valores_db)
    magnitude_db_convertido = 20 * np.log10(np.abs(yf_db[:N//2]))  # Converte para dB
    
    # Evitar problemas logarítmicos com valores zero ou negativos
    magnitude_db_convertido[magnitude_db_convertido == -np.inf] = -100  # Definir um valor mínimo para dB
    
    plt.subplot(2, 1, 2)
    plt.semilogx(xf, magnitude_db_convertido)
    plt.title(title_db)
    plt.xlabel('Frequência (Hz) [Escala Logarítmica]')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    
    plt.tight_layout()
    plt.show()

# Caminho do arquivo com valores brutos
input_txt = '/home/joselito/git/tcc/datas/valores_brutos.txt'  # Caminho do arquivo de entrada
fs = 44100  # Frequência de amostragem (Hz), ajustada conforme necessário

# 1. Ler os valores brutos do arquivo .txt
valores_brutos = ler_valores_brutos_txt(input_txt)

# 2. Converter os valores para dB
valores_db = converter_para_db(valores_brutos)

# 3. Plotar o espectro do sinal original em dB e o espectro após conversão para dB
plot_spectrum(valores_brutos, valores_db, fs, 'Espectro do Sinal Original em dB', 'Espectro do Sinal Convertido para dB')
