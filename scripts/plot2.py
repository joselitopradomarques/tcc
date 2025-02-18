import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as signal
import librosa
import librosa.display

# Função para carregar o arquivo WAV
def carregar_arquivo_wav(caminho_arquivo):
    # Carregar o áudio usando librosa
    sinal, taxa_amostragem = librosa.load(caminho_arquivo, sr=None, mono=False)
    return sinal, taxa_amostragem

# Função para plotar o domínio do tempo
def plotar_dominio_do_tempo(sinal, taxa_amostragem):
    tempo = np.linspace(0, len(sinal) / taxa_amostragem, num=len(sinal))
    plt.figure(figsize=(10, 6))
    plt.plot(tempo, sinal[0], label="Canal 1")
    plt.plot(tempo, sinal[1], label="Canal 2", alpha=0.7)
    plt.title("Domínio do Tempo (Primeiros 2 segundos)")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Amplitude")
    plt.legend()
    plt.grid(True)
    plt.show()

# Função para plotar a STFT
def plotar_stft(sinal, taxa_amostragem):
    n_fft = 2048  # Tamanho da janela FFT
    hop_length = 512  # Deslocamento entre janelas
    stft = librosa.stft(sinal[0], n_fft=n_fft, hop_length=hop_length)  # STFT para o canal 1
    stft_db = librosa.amplitude_to_db(np.abs(stft), ref=np.max)

    plt.figure(figsize=(10, 6))
    librosa.display.specshow(stft_db, x_axis='time', y_axis='log', sr=taxa_amostragem)
    plt.colorbar(label='Magnitude (dB)')
    plt.title("STFT (Primeiros 2 segundos)")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Frequência (Hz)")
    plt.show()

# Caminho do arquivo WAV
caminho_arquivo = "/home/joselito/git/tcc/scripts/output_filtered_00.wav"

# Carregar os primeiros 2 segundos do arquivo WAV
sinal, taxa_amostragem = carregar_arquivo_wav(caminho_arquivo)

# Cortar os primeiros 2 segundos
num_amostras = int(2 * taxa_amostragem)  # 2 segundos
sinal_2_segundos = sinal[:, :num_amostras]

# Plotar o domínio do tempo e a STFT
plotar_dominio_do_tempo(sinal_2_segundos, taxa_amostragem)
plotar_stft(sinal_2_segundos, taxa_amostragem)
