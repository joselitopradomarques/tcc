import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, lfilter, freqz
import wave
import struct

# Função para carregar o arquivo WAV usando a biblioteca wave
def load_audio(filename):
    with wave.open(filename, 'rb') as wav_file:
        # Obter os parâmetros do arquivo WAV
        sample_rate = wav_file.getframerate()
        num_frames = wav_file.getnframes()
        num_channels = wav_file.getnchannels()

        # Ler os frames de áudio
        frames = wav_file.readframes(num_frames)

        # Desempacotar os dados dos frames (considerando que os dados são em 16 bits)
        audio_data = np.array(struct.unpack('<' + 'h' * (num_frames * num_channels), frames))
        
        # Se for estéreo, converter para mono tirando a média dos canais
        if num_channels == 2:
            audio_data = (audio_data[::2] + audio_data[1::2]) / 2

    return sample_rate, audio_data

# Função para aplicar o filtro FIR
def apply_fir_filter(data, sample_rate, cutoff_freq, order):
    # Gera os coeficientes FIR para o filtro passa-alta com janela Hamming
    nyquist = 0.5 * sample_rate
    normal_cutoff = cutoff_freq / nyquist
    order = 33 if order % 2 == 0 else order  # Garante que a ordem seja ímpar
    fir_coeff = firwin(order, normal_cutoff, pass_zero=False, window='hamming')
    
    # Aplica o filtro no áudio
    filtered_data = lfilter(fir_coeff, 1.0, data)
    
    return filtered_data, fir_coeff

# Função para visualizar a resposta em frequência do filtro
def plot_frequency_response(fir_coeff, sample_rate):
    w, h = freqz(fir_coeff, worN=8000)
    plt.plot(0.5 * sample_rate * w / np.pi, np.abs(h), 'b')
    plt.axvline(cutoff_freq, color='k')
    plt.xlim(0, 0.5 * sample_rate)
    plt.title("Resposta em Frequência do Filtro Passa-Alta")
    plt.xlabel('Frequência [Hz]')
    plt.ylabel('Magnitude')
    plt.grid(True)

# Função para plotar o áudio original vs áudio filtrado
def plot_audio_comparison(original, filtered, sample_rate):
    time = np.arange(len(original)) / sample_rate
    plt.figure(figsize=(10, 6))
    plt.subplot(2, 1, 1)
    plt.plot(time, original)
    plt.title("Áudio Original")
    plt.subplot(2, 1, 2)
    plt.plot(time, filtered)
    plt.title("Áudio Filtrado")
    plt.tight_layout()
    plt.show()

# Testando o filtro com uma ordem específica
filename = '/home/joselito/git/tcc/datas/audio01.wav'  # Substitua com o caminho do seu arquivo WAV
cutoff_freq = 500  # Frequência de corte em Hz (ajuste conforme necessário)
order = 33  # Ordem do filtro (ajuste conforme necessário)

# Carregar áudio
sample_rate, audio_data = load_audio(filename)

# Aplicar filtro e plotar resultados
print(f"Aplicando filtro com ordem {order}...")
filtered_audio, fir_coeff = apply_fir_filter(audio_data, sample_rate, cutoff_freq, order)

# Plotar resposta em frequência
plot_frequency_response(fir_coeff, sample_rate)

# Plotar comparação entre áudio original e filtrado
plot_audio_comparison(audio_data, filtered_audio, sample_rate)

# Mostrar os gráficos
plt.show()
