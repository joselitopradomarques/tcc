import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.fftpack import fft

# Função para calcular a FFT
def compute_fft(audio_data, sample_rate):
    N = len(audio_data)  # Número de pontos
    audio_fft = fft(audio_data)  # Cálculo da FFT
    freqs = np.fft.fftfreq(N, 1 / sample_rate)  # Frequências associadas à FFT
    magnitude = np.abs(audio_fft)[:N // 2]  # Usar apenas componentes positivas
    freqs = freqs[:N // 2]  # Frequências positivas
    return freqs, magnitude

# Função para carregar o arquivo WAV
def load_wav(filename):
    sample_rate, audio_data = wavfile.read(filename)
    
    # Se o áudio for estéreo, converter para mono fazendo a média dos canais
    if len(audio_data.shape) > 1:
        audio_data = np.mean(audio_data, axis=1)
    
    return sample_rate, audio_data

# Função para plotar FFT dos dois sinais
def plot_fft_comparison(wav_filename_original, wav_filename_filtered):
    # Carregar os dois arquivos WAV
    sample_rate_original, audio_data_original = load_wav(wav_filename_original)
    sample_rate_filtered, audio_data_filtered = load_wav(wav_filename_filtered)

    # Calcular a FFT do sinal original e do sinal filtrado
    freqs_original, magnitude_original = compute_fft(audio_data_original, sample_rate_original)
    freqs_filtered, magnitude_filtered = compute_fft(audio_data_filtered, sample_rate_filtered)

    # Limitar o range das frequências de 20 Hz a 22.050 Hz
    freq_min = 20
    freq_max = 22050

    # Filtrar os dados dentro do intervalo desejado para o sinal original
    mask_original = (freqs_original >= freq_min) & (freqs_original <= freq_max)
    freqs_original = freqs_original[mask_original]
    magnitude_original = magnitude_original[mask_original]

    # Filtrar os dados dentro do intervalo desejado para o sinal filtrado
    mask_filtered = (freqs_filtered >= freq_min) & (freqs_filtered <= freq_max)
    freqs_filtered = freqs_filtered[mask_filtered]
    magnitude_filtered = magnitude_filtered[mask_filtered]
    
    # Plotar FFT dos dois sinais na mesma janela
    plt.figure(figsize=(12, 6))

    # Plot do sinal original
    plt.subplot(2, 1, 1)
    plt.plot(freqs_original, magnitude_original)
    plt.xscale('log')  # Escala logarítmica no eixo x
    plt.xlim([freq_min, freq_max])  # Definir limites do eixo x
    plt.xlabel('Frequência (Hz)')
    plt.ylabel('Magnitude')
    plt.title(f'FFT do Sinal Original ({wav_filename_original})')
    plt.grid(True)

    # Plot do sinal filtrado
    plt.subplot(2, 1, 2)
    plt.plot(freqs_filtered, magnitude_filtered)
    plt.xscale('log')  # Escala logarítmica no eixo x
    plt.xlim([freq_min, freq_max])  # Definir limites do eixo x
    plt.xlabel('Frequência (Hz)')
    plt.ylabel('Magnitude')
    plt.title(f'FFT do Sinal Filtrado ({wav_filename_filtered})')
    plt.grid(True)

    # Mostrar os gráficos
    plt.tight_layout()
    plt.show()

# Exemplo de uso:
wav_filename_original = '/home/joselito/git/tcc/audio_files/ref_000_cortado.wav'  # Substitua pelo seu arquivo original
wav_filename_filtered = '/home/joselito/git/tcc/audio_files/ref_300_cortado.wav'  # Substitua pelo seu arquivo filtrado
plot_fft_comparison(wav_filename_original, wav_filename_filtered)
