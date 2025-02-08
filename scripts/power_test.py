import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.fftpack import fft

# Função para calcular a FFT e a potência do áudio
def compute_fft_and_power(audio_data, sample_rate):
    N = len(audio_data)  # Número de pontos
    audio_fft = fft(audio_data)  # Cálculo da FFT
    freqs = np.fft.fftfreq(N, 1 / sample_rate)  # Frequências associadas à FFT
    magnitude = np.abs(audio_fft)[:N // 2]  # Usar apenas componentes positivas
    freqs = freqs[:N // 2]  # Frequências positivas
    
    # Calcular a potência (magnitude ao quadrado)
    power = magnitude ** 2
    
    return freqs, power

# Função para carregar o arquivo WAV
def load_wav(filename):
    sample_rate, audio_data = wavfile.read(filename)
    
    # Se o áudio for estéreo, converter para mono fazendo a média dos canais
    if len(audio_data.shape) > 1:
        audio_data = np.mean(audio_data, axis=1)
    
    return sample_rate, audio_data

# Função para calcular a razão da potência entre as bandas
def calculate_power_ratio(wav_filename1, wav_filename2):
    # Carregar os dois arquivos WAV
    sample_rate1, audio_data1 = load_wav(wav_filename1)
    sample_rate2, audio_data2 = load_wav(wav_filename2)

    # Determinar o número mínimo de amostras para garantir FFTs compatíveis
    N = min(len(audio_data1), len(audio_data2))

    # Cortar os sinais para o mesmo comprimento
    audio_data1 = audio_data1[:N]
    audio_data2 = audio_data2[:N]

    # Calcular a FFT e a potência dos sinais
    freqs1, power1 = compute_fft_and_power(audio_data1, sample_rate1)
    freqs2, power2 = compute_fft_and_power(audio_data2, sample_rate2)
    
    # Definir as bandas de interesse: 20-300 Hz e 20-22050 Hz
    band_20_300 = (freqs1 >= 20) & (freqs1 <= 300)
    band_20_22050 = (freqs1 >= 20) & (freqs1 <= 22050)

    # Calcular a potência na banda de 20-300 Hz
    power_band_20_300_1 = np.sum(power1[band_20_300])
    power_band_20_300_2 = np.sum(power2[band_20_300])
    
    # Calcular a potência na banda de 20-22050 Hz
    power_band_20_22050_1 = np.sum(power1[band_20_22050])
    power_band_20_22050_2 = np.sum(power2[band_20_22050])

    # Calcular a razão da potência para os dois arquivos
    ratio1 = power_band_20_300_1 / power_band_20_22050_1
    ratio2 = power_band_20_300_2 / power_band_20_22050_2
    
    # Exibir a razão da potência
    print(f'Razão da potência (20-300 Hz / 20-22050 Hz) para {wav_filename1}: {ratio1}')
    print(f'Razão da potência (20-300 Hz / 20-22050 Hz) para {wav_filename2}: {ratio2}')
    
    # Retornar as razões
    return ratio1, ratio2

# Exemplo de uso:
wav_filename1 = '/home/joselito/git/tcc/audio_files/ref_000_cortado.wav'  # Primeiro arquivo
wav_filename2 = '/home/joselito/git/tcc/audio_files/ref_300_cortado_filtrado.wav'  # Segundo arquivo
ratio1, ratio2 = calculate_power_ratio(wav_filename1, wav_filename2)


# Visualizar os resultados
print(f'Razão da potência do primeiro arquivo: {ratio1}')
print(f'Razão da potência do segundo arquivo: {ratio2}')
