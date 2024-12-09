import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.fft import fft

# Função para ler arquivos WAV
def ler_wav(filename):
    fs, sinal = wavfile.read(filename)
    return fs, sinal

# Função para normalizar o sinal de áudio
def normalizar_sinal(sinal):
    max_val = np.max(np.abs(sinal))  # Encontra o valor máximo absoluto
    sinal_normalizado = sinal / max_val  # Normaliza o sinal
    return sinal_normalizado

# Função para calcular o espectro de um sinal
def calcular_espectro(sinal, fs):
    N = len(sinal)
    T = 1.0 / fs
    yf = fft(sinal)
    xf = np.fft.fftfreq(N, T)[:N//2]
    magnitude_sinal = 2.0 / N * np.abs(yf[:N//2])
    magnitude_db_sinal = 20 * np.log10(magnitude_sinal)
    
    # Substituindo valores infinitamente negativos por -100 para a visualização
    magnitude_db_sinal[magnitude_db_sinal == -np.inf] = -100
    
    return xf, magnitude_db_sinal

# Função para plotar espectro do sinal
def plot_espectro(sinal, fs):
    # Calcular espectro do sinal
    xf, magnitude_db_sinal = calcular_espectro(sinal, fs)
    
    # Plotando espectro
    plt.figure(figsize=(10, 6))
    plt.semilogx(xf, magnitude_db_sinal, label='Espectro do Sinal (dB)', color='orange')
    plt.title('Espectro do Sinal de Áudio')
    plt.xlabel('Frequência (Hz) [Escala Logarítmica]')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    plt.legend(loc='upper right')
    plt.show()

# Função para plotar os valores mínimo e máximo do sinal normalizado
def plot_min_max(sinal, fs):
    time_axis = np.linspace(0, len(sinal) / fs, num=len(sinal))  # Eixo do tempo
    
    # Calcula os valores máximo e mínimo ao longo do tempo
    max_sinal = np.max(sinal)
    min_sinal = np.min(sinal)
    
    # Plotando o sinal normalizado, com destaque para os valores mínimo e máximo
    plt.figure(figsize=(10, 6))
    plt.plot(time_axis, sinal, label='Sinal Normalizado', color='blue', alpha=0.6)
    plt.axhline(y=max_sinal, color='red', linestyle='--', label=f'Máximo: {max_sinal:.2f}')
    plt.axhline(y=min_sinal, color='green', linestyle='--', label=f'Mínimo: {min_sinal:.2f}')
    plt.title('Sinal Normalizado - Valores Mínimo e Máximo')
    plt.xlabel('Tempo (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend(loc='upper right')
    plt.show()

# Nome do arquivo de entrada
input_wav = '/home/joselito/git/tcc/audio_files/case4_cut.wav'  # Caminho do arquivo de entrada

# Carregar arquivo WAV de entrada
fs, sinal = ler_wav(input_wav)

# Normalizar o sinal
if sinal.ndim > 1:  # Se o sinal for estéreo, normalizamos cada canal separadamente
    sinal[:, 0] = normalizar_sinal(sinal[:, 0])  # Normaliza o primeiro canal
    sinal[:, 1] = normalizar_sinal(sinal[:, 1])  # Normaliza o segundo canal
    
    # Plotar o espectro do primeiro canal
    plot_espectro(sinal[:, 0], fs)
    
    # Plotar os valores mínimo e máximo do primeiro canal normalizado
    plot_min_max(sinal[:, 0], fs)
    
else:  # Se o sinal for mono
    sinal = normalizar_sinal(sinal)
    
    # Plotar o espectro do sinal mono
    plot_espectro(sinal, fs)
    
    # Plotar os valores mínimo e máximo do sinal mono normalizado
    plot_min_max(sinal, fs)
