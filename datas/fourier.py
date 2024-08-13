import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wav
from scipy.signal import windows
from matplotlib.colors import Normalize
import re
import os

# Função para ler o arquivo WAV
def read_wav(file_path):
    sample_rate, data = wav.read(file_path)
    return sample_rate, data

# Função para calcular a FFT para cada segmento do sinal
def calculate_fft_segments(data, window_size, noverlap, sample_rate):
    step = window_size - noverlap
    num_windows = int(np.ceil(len(data) / step))
    
    freqs = np.fft.fftfreq(window_size, d=1/sample_rate)[:window_size // 2]
    time_segments = []
    magnitudes = []
    
    for i in range(num_windows):
        start = i * step
        end = min(start + window_size, len(data))
        segment = data[start:end]
        
        if len(segment) < window_size:
            segment = np.pad(segment, (0, window_size - len(segment)), mode='constant')

        segment = segment.astype(float)
        
        window = windows.hann(window_size)
        segment *= window
        
        fft_values = np.fft.fft(segment) / window_size
        magnitude = np.abs(fft_values[:window_size // 2])
        
        magnitudes.append(magnitude)
        time_segments.append((start + end) / 2 / sample_rate)
    
    return np.array(freqs), np.array(time_segments), np.array(magnitudes)

# Função para extrair o número do nome do arquivo
def extract_case_number(file_path):
    match = re.search(r'case(\d+)', file_path)
    return match.group(1) if match else 'unknown'

# Função para calcular o intervalo global de amplitude
def calculate_global_amplitude_range(file_paths):
    min_amplitude = float('inf')
    max_amplitude = float('-inf')
    
    for file_path in file_paths:
        sample_rate, data = read_wav(file_path)
        if len(data.shape) > 1:
            data = data[:, 0]  # Usar apenas o canal esquerdo se for estéreo
        min_amplitude = min(min_amplitude, np.min(data))
        max_amplitude = max(max_amplitude, np.max(data))
    
    return min_amplitude, max_amplitude

# Função para plotar o sinal e o heatmap espectral e salvar as imagens
def plot_signal_and_fft(file_path, window_size=2048, noverlap=1024, plot_duration=2, output_folder='./'):
    sample_rate, data = read_wav(file_path)
    
    if len(data.shape) > 1:
        data = data[:, 0]  # Usar apenas o canal esquerdo se for estéreo
    
    num_samples = int(plot_duration * sample_rate)
    data = data[:num_samples]
    
    case_number = extract_case_number(file_path)
    
    # Plotar o sinal no domínio do tempo
    plt.figure(figsize=(14, 6))
    plt.plot(np.arange(len(data)) / sample_rate, data, label='Sinal no Domínio do Tempo')
    # Remover o título
    # plt.title(f'Sinal no Domínio do Tempo - Case {case_number}')
    plt.xlabel('Tempo (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend()
    plt.ylim([global_min_amplitude, global_max_amplitude])  # Manter o intervalo global
    plt.savefig(f'{output_folder}signal_time_domain_case_{case_number}.png')  # Salvar como arquivo PNG
    plt.close()
    
    # Calcular e plotar a STFT
    freqs, times, magnitudes = calculate_fft_segments(data, window_size, noverlap, sample_rate)
    
    # Definir intervalo de dB para o heatmap
    magnitude_db = 20 * np.log10(np.array(magnitudes).T + 1e-10)
    norm = Normalize(vmin=-50, vmax=50)  # Normalização entre -50 dB e 50 dB
    
    plt.figure(figsize=(14, 6))
    plt.pcolormesh(times, freqs, magnitude_db, shading='gouraud', cmap='inferno', norm=norm)
    # Remover o título
    # plt.title(f'Heatmap Espectral usando FFT - Case {case_number}')
    plt.xlabel('Tempo (s)')
    plt.ylabel('Frequência (Hz)')
    plt.colorbar(label='Magnitude (dB)')
    plt.yscale('log')  # Escala logarítmica para o eixo y (frequências)
    
    # Ajustar o limite do eixo y para mostrar frequências de 20 Hz até a metade da taxa de amostragem
    plt.ylim([20, sample_rate / 2])
    
    plt.grid(True)
    plt.savefig(f'{output_folder}heatmap_spectral_case_{case_number}.png')  # Salvar como arquivo PNG
    plt.close()

# Caminho para os arquivos WAV
file_paths = [f'/home/joselito/git/tcc/PureData/case{i}_cut.wav' for i in range(1, 9)]

# Encontrar o intervalo global de amplitude
global_min_amplitude, global_max_amplitude = calculate_global_amplitude_range(file_paths)

# Pasta onde os arquivos serão salvos
output_folder = './plots/'
os.makedirs(output_folder, exist_ok=True)  # Criar a pasta se não existir

# Executar o script para cada arquivo
for file_path in file_paths:
    plot_signal_and_fft(file_path, window_size=2048, noverlap=1024, plot_duration=2, output_folder=output_folder)
