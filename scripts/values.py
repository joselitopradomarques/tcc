import wave
import numpy as np
import matplotlib.pyplot as plt

# Caminho dos arquivos WAV
filename_filtered = '/home/joselito/git/tcc/datas/saida_filtrada_estereo.wav'
filename_original = '/home/joselito/git/tcc/datas/audio02.wav'

# Função para carregar o áudio de um arquivo WAV
def load_audio(filename):
    with wave.open(filename, 'r') as wav_file:
        framerate = wav_file.getframerate()
        n_frames = wav_file.getnframes()
        n_channels = wav_file.getnchannels()  # Número de canais (1 para mono, 2 para estéreo)
        audio_data = wav_file.readframes(n_frames)
    
    # Converter os dados de áudio para um array NumPy
    audio_samples = np.frombuffer(audio_data, dtype=np.int16)

    if n_channels == 2:
        # Para áudio estéreo, separa os canais
        audio_samples_left = audio_samples[0::2]  # Canal esquerdo (índices pares)
        audio_samples_right = audio_samples[1::2]  # Canal direito (índices ímpares)
        # Retorna apenas o canal esquerdo (ou poderia escolher o direito)
        return audio_samples_left, framerate, n_frames
    else:
        return audio_samples, framerate, n_frames

# Carregar os dois arquivos de áudio
audio_samples_filtered, framerate_filtered, n_frames_filtered = load_audio(filename_filtered)
audio_samples_original, framerate_original, n_frames_original = load_audio(filename_original)

# Função para calcular e plotar o espectro
def plot_spectrum(audio_samples, framerate, title, subplot_index):
    # Calcular a FFT do sinal
    n = len(audio_samples)
    spectrum = np.fft.fft(audio_samples)
    freqs = np.fft.fftfreq(n, 1 / framerate)
    
    # Considerar apenas a metade positiva do espectro
    positive_freqs = freqs[:n // 2]
    positive_spectrum = np.abs(spectrum[:n // 2])

    # Plotar o espectro
    plt.subplot(2, 1, subplot_index)  # Plotar na posição especificada
    plt.plot(positive_freqs, positive_spectrum, label=title)
    plt.title(f'Espectro de {title}')
    plt.xlabel('Frequência (Hz)')
    plt.ylabel('Magnitude')
    plt.grid(True)
    plt.legend()

# Plotar os dois espectros na mesma página
plt.figure(figsize=(12, 6))

# Plot do espectro para o sinal filtrado
plot_spectrum(audio_samples_filtered, framerate_filtered, "Sinal Filtrado", 1)

# Plot do espectro para o sinal original
plot_spectrum(audio_samples_original, framerate_original, "Sinal Original", 2)

plt.tight_layout()
plt.show()
