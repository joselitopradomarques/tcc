import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, lfilter, freqz
import wave
import struct
import scipy.io.wavfile as wav

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

# Função para salvar o áudio filtrado em um novo arquivo WAV
def save_filtered_audio(filename, sample_rate, filtered_data):
    # Converte o áudio filtrado para 16 bits
    filtered_data_int16 = np.int16(filtered_data)

    # Salva o áudio filtrado
    wav.write(filename, sample_rate, filtered_data_int16)

# Função para visualizar a resposta em frequência do filtro
def plot_frequency_response(fir_coeff, sample_rate):
    w, h = freqz(fir_coeff, worN=8000)
    plt.plot(0.5 * sample_rate * w / np.pi, np.abs(h), 'b')
    plt.axvline(cutoff_freq, color='k')
    plt.xlim(20, 0.5 * sample_rate)  # Limitar para a faixa de frequência audível (20 Hz até Nyquist)
    plt.xscale('log')  # Definir o eixo de frequência em escala logarítmica
    plt.title("Resposta em Frequência do Filtro Passa-Alta")
    plt.xlabel('Frequência [Hz]')
    plt.ylabel('Magnitude')
    plt.grid(True)

# Função para plotar o áudio original vs áudio filtrado
def plot_audio_comparison(original, filtered, sample_rate, duration=2):
    # Calcula o número de amostras correspondentes à duração de 2 segundos
    num_samples = int(duration * sample_rate)

    # Normalizar os dados de áudio para o intervalo [-1, 1] usando o valor máximo absoluto entre ambos os áudios
    max_amplitude = max(np.max(np.abs(original[:num_samples])), np.max(np.abs(filtered[:num_samples])))
    original_normalized = original[:num_samples] / max_amplitude
    filtered_normalized = filtered[:num_samples] / max_amplitude

    # Define o tempo normalizado para 2 segundos
    time = np.arange(num_samples) / sample_rate

    # Plota o áudio original e filtrado para os primeiros 2 segundos
    plt.figure(figsize=(10, 6))

    # Plot do áudio original
    plt.subplot(2, 1, 1)
    plt.plot(time, original_normalized)
    plt.title("Áudio Original (2 segundos)")
    plt.xlabel("Tempo [s]")
    plt.ylabel("Amplitude Normalizada")

    # Plot do áudio filtrado
    plt.subplot(2, 1, 2)
    plt.plot(time, filtered_normalized)
    plt.title("Áudio Filtrado (2 segundos)")
    plt.xlabel("Tempo [s]")
    plt.ylabel("Amplitude Normalizada")

    plt.tight_layout()
    plt.show()

# Testando o filtro com uma ordem específica
filename = '/home/joselito/git/tcc/audio_files/ref_000_cortado.wav'  # Substitua com o caminho do seu arquivo WAV
cutoff_freq = 300  # Frequência de corte em Hz (ajuste conforme necessário)
order = 129  # Ordem do filtro (ajuste conforme necessário)

# Carregar áudio
sample_rate, audio_data = load_audio(filename)

# Aplicar filtro e plotar resultados
print(f"Aplicando filtro com ordem {order}...")
filtered_audio, fir_coeff = apply_fir_filter(audio_data, sample_rate, cutoff_freq, order)

# Plotar resposta em frequência com escala logarítmica no eixo das frequências
plot_frequency_response(fir_coeff, sample_rate)

# Plotar comparação entre áudio original e filtrado (apenas 2 segundos)
plot_audio_comparison(audio_data, filtered_audio, sample_rate)

# Salvar o áudio filtrado em um novo arquivo WAV
output_filename = '/home/joselito/git/tcc/audio_files/ref_300_cortado_filtrado.wav'  # Caminho do arquivo de saída
save_filtered_audio(output_filename, sample_rate, filtered_audio)
print(f"Áudio filtrado salvo em: {output_filename}")

# Mostrar os gráficos
plt.show()
