from scipy.signal import ellip, freqz, filtfilt
import matplotlib.pyplot as plt
import numpy as np
from scipy.fft import fft

# Função para normalizar o sinal
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
    
    # Substituir valores infinitamente negativos por -100 para visualização
    magnitude_db_sinal[magnitude_db_sinal == -np.inf] = -100
    
    return xf, magnitude_db_sinal

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

# Função para plotar o sinal no domínio do tempo
def plot_sinal(sinal, fs, title):
    time_axis = np.linspace(0, len(sinal) / fs, num=len(sinal))  # Eixo do tempo
    
    plt.figure(figsize=(10, 6))
    plt.plot(time_axis, sinal, label='Sinal', color='purple')
    plt.title(title)
    plt.xlabel('Tempo (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend(loc='upper right')
    plt.show()

# Definir parâmetros do filtro
rp = 0.1  # Ripple na banda passante (dB)
rs = 26  # Atenuação na banda rejeitada (dB)
fc = 200  # Frequência de corte (Hz)
fs = 44100  # Frequência de amostragem (Hz)

# Calcular o filtro elíptico
b, a = ellip(4, rp, rs, fc / (0.5 * fs), btype='high', analog=False)

# Gerar um sinal de entrada para aplicar o filtro (sinal senoidal, por exemplo)
t = np.linspace(0, 1.0, fs)  # Eixo do tempo (1 segundo de duração)
sinal = np.sin(2 * np.pi * 100 * t) + 0.5 * np.sin(2 * np.pi * 500 * t)  # Sinal com 100 Hz e 500 Hz

# Plotar o sinal original no domínio do tempo antes de ser normalizado
plot_sinal(sinal, fs, 'Sinal Original no Domínio do Tempo (Antes da Normalização)')

# Calcular o espectro do sinal original
xf_original, espectro_original = calcular_espectro(sinal, fs)

# Plotar o espectro do sinal original
plt.figure(figsize=(10, 6))
plt.plot(xf_original, espectro_original, label='Espectro do Sinal Original (dB)', color='purple')
plt.title('Espectro do Sinal Original')
plt.xlabel('Frequência [Hz]')
plt.ylabel('Magnitude [dB]')
plt.xscale('log')  # Escala logarítmica no eixo x
plt.xlim([10, fs / 2])  # Limites da frequência
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.legend(loc='best')
plt.show()

# Aplicar o filtro ao sinal
sinal_filtrado = filtfilt(b, a, sinal)

# Normalizar o sinal filtrado
sinal_normalizado = normalizar_sinal(sinal_filtrado)

# Calcular o espectro do sinal filtrado
xf_filtrado, espectro_filtrado = calcular_espectro(sinal_filtrado, fs)

# Plotar a resposta em frequência do filtro e o espectro do sinal filtrado
w, h = freqz(b, a, worN=2000)
plt.figure(figsize=(10, 6))

# Resposta do filtro
plt.plot(w * fs / (2 * np.pi), 20 * np.log10(abs(h)), label='Resposta do Filtro Elíptico', color='blue')

# Espectro do sinal filtrado
plt.plot(xf_filtrado, espectro_filtrado, label='Espectro do Sinal Filtrado (dB)', color='orange')

# Configurar o gráfico
plt.title('Resposta em Frequência do Filtro e Espectro do Sinal Filtrado')
plt.xlabel('Frequência [Hz]')
plt.ylabel('Magnitude [dB]')
plt.xscale('log')  # Escala logarítmica no eixo x
plt.xlim([10, fs / 2])  # Limites da frequência
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.legend(loc='best')
plt.show()

# Plotar o sinal normalizado e destacar os valores máximo e mínimo
plot_min_max(sinal_normalizado, fs)
