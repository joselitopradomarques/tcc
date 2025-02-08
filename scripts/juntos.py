import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.fftpack import fft
from scipy.signal import firwin, lfilter, freqz
import wave
import struct
import os

# Função para carregar o arquivo WAV usando a biblioteca wave
def load_audio(filename):
    with wave.open(filename, 'rb') as wav_file:
        sample_rate = wav_file.getframerate()
        num_frames = wav_file.getnframes()
        num_channels = wav_file.getnchannels()
        frames = wav_file.readframes(num_frames)
        audio_data = np.array(struct.unpack('<' + 'h' * (num_frames * num_channels), frames))
        
        if num_channels == 2:
            audio_data = (audio_data[::2] + audio_data[1::2]) / 2

    return sample_rate, audio_data

# Função para aplicar o filtro FIR
def apply_fir_filter(data, sample_rate, cutoff_freq, order):
    nyquist = 0.5 * sample_rate
    normal_cutoff = cutoff_freq / nyquist
    order = 33 if order % 2 == 0 else order  # Garante que a ordem seja ímpar
    fir_coeff = firwin(order, normal_cutoff, pass_zero=False, window='hamming')
    filtered_data = lfilter(fir_coeff, 1.0, data)
    
    return filtered_data, fir_coeff

# Função para calcular a FFT e a potência do áudio
def compute_fft_and_power(audio_data, sample_rate):
    N = len(audio_data)
    audio_fft = fft(audio_data)
    freqs = np.fft.fftfreq(N, 1 / sample_rate)
    magnitude = np.abs(audio_fft)[:N // 2]
    freqs = freqs[:N // 2]
    power = magnitude ** 2
    return freqs, power

# Função para calcular a razão da potência entre as bandas
def calculate_power_ratio(wav_filename1, wav_filename2):
    sample_rate1, audio_data1 = load_audio(wav_filename1)
    sample_rate2, audio_data2 = load_audio(wav_filename2)
    N = min(len(audio_data1), len(audio_data2))
    audio_data1 = audio_data1[:N]
    audio_data2 = audio_data2[:N]
    freqs1, power1 = compute_fft_and_power(audio_data1, sample_rate1)
    freqs2, power2 = compute_fft_and_power(audio_data2, sample_rate2)
    
    band_20_300 = (freqs1 >= 20) & (freqs1 <= 300)
    band_20_22050 = (freqs1 >= 20) & (freqs1 <= 22050)

    power_band_20_300_1 = np.sum(power1[band_20_300])
    power_band_20_300_2 = np.sum(power2[band_20_300])
    
    power_band_20_22050_1 = np.sum(power1[band_20_22050])
    power_band_20_22050_2 = np.sum(power2[band_20_22050])

    ratio1 = power_band_20_300_1 / power_band_20_22050_1
    ratio2 = power_band_20_300_2 / power_band_20_22050_2
    
    return ratio1, ratio2

# Função para salvar o áudio filtrado
def save_filtered_audio(filename, sample_rate, filtered_data):
    filtered_data_int16 = np.int16(filtered_data)
    wavfile.write(filename, sample_rate, filtered_data_int16)

# Função principal para ajustar a ordem do filtro
def find_min_order_to_satisfy_condition(wav_filename, target_ratio2):
    cutoff_freq = 300
    order = 3  # Ordem inicial ímpar mínima
    sample_rate, audio_data = load_audio(wav_filename)
    
    while True:
        filtered_audio, fir_coeff = apply_fir_filter(audio_data, sample_rate, cutoff_freq, order)
        
        # Salva o áudio filtrado temporariamente
        temp_filtered_filename = '/tmp/filtered_audio.wav'
        save_filtered_audio(temp_filtered_filename, sample_rate, filtered_audio)
        
        _, ratio2 = calculate_power_ratio(wav_filename, temp_filtered_filename)
        
        if ratio2 < target_ratio2:
            print(f"A ordem mínima encontrada é {order} com razão de potência {ratio2}.")
            break
        order += 2  # Aumenta a ordem mantendo-a ímpar

# Exemplo de uso
wav_filename = '/home/joselito/git/tcc/audio_files/ref_000_cortado.wav'  # Caminho para o arquivo de áudio original
target_ratio2 = 0.45598988835607557  # Valor alvo da razão de potência
find_min_order_to_satisfy_condition(wav_filename, target_ratio2)
