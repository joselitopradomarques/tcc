import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wav
from scipy.signal import windows

# Função para ler o arquivo WAV
def read_wav(file_path):
    sample_rate, data = wav.read(file_path)
    return sample_rate, data

# Função para calcular e plotar o espectro usando FFT com variação de cor
def plot_fft_color(file_path, window_size=1024, window_type='hann', noverlap=512):
    sample_rate, data = read_wav(file_path)
    
    if len(data.shape) > 1:
        data = data[:, 0]  # Usar apenas o canal esquerdo se for estéreo

    # Escolher a janela
    if window_type == 'hann':
        window = windows.hann(window_size)
    elif window_type == 'hamming':
        window = windows.hamming(window_size)
    elif window_type == 'blackman':
        window = windows.blackman(window_size)
    elif window_type == 'bartlett':
        window = windows.bartlett(window_size)
    elif window_type == 'kaiser':
        window = windows.kaiser(window_size, beta=5.0)
    else:
        raise ValueError("Tipo de janela desconhecido")

    # Preparar para a plotagem
    step = window_size - noverlap
    num_windows = int(np.ceil(len(data) / step))
    times = []
    freqs = []
    magnitudes = []

    for i in range(num_windows):
        start = i * step
        end = min(start + window_size, len(data))
        segment = data[start:end]
        
        # Aplicar a janela
        if len(segment) < window_size:
            segment = np.pad(segment, (0, window_size - len(segment)), mode='constant')

        segment *= window

        # Calcular a FFT
        fft_values = np.fft.fft(segment) / window_size
        freqs.append(np.fft.fftfreq(window_size, d=1/sample_rate)[:window_size//2])
        magnitudes.append(np.abs(fft_values[:window_size//2]))

        # Tempo para a janela
        times.append((start + end) / 2 / sample_rate)

    # Convertendo listas para arrays
    freqs = np.array(freqs).T
    magnitudes = np.array(magnitudes).T
    times = np.array(times)

    # Plotar o espectro com variação de cor
    plt.figure(figsize=(12, 6))
    plt.pcolormesh(times, freqs[:, 0], 20 * np.log10(magnitudes), shading='gouraud', cmap='inferno')
    plt.title('Espectro FFT com Variação de Cor')
    plt.xlabel('Tempo (s)')
    plt.ylabel('Frequência (Hz)')
    plt.colorbar(label='Magnitude (dB)')
    plt.yscale('log')  # Escala logarítmica no eixo y (frequências)
    plt.show()

# Caminho para o arquivo WAV
file_path = '/home/joselito/git/tcc/PureData/case12_cut.wav'

# Exemplo de uso com diferentes tipos e tamanhos de janela
plot_fft_color(file_path, window_size=1024, window_type='hann')
