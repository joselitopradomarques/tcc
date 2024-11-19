import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

# Função para carregar o arquivo wav e plotar 5 segundos
def plot_audio_file(wav_filename, duration=5):
    # Carregar o arquivo WAV
    sample_rate, data = wavfile.read(wav_filename)
    
    # Calcular o número de amostras a serem exibidas (5 segundos)
    num_samples = duration * sample_rate
    
    # Certifique-se de não ultrapassar o número total de amostras no arquivo
    if num_samples > len(data):
        num_samples = len(data)
    
    # Criar o eixo do tempo
    time = np.linspace(0, num_samples / sample_rate, num_samples, endpoint=False)
    
    # Se o áudio for estéreo (2 canais), pegamos apenas o primeiro canal
    if len(data.shape) > 1:
        data = data[:, 0]
    
    # Selecionar os dados dos primeiros 5 segundos
    data_to_plot = data[:num_samples]
    
    # Plotar os dados
    plt.figure(figsize=(10, 4))
    plt.plot(time, data_to_plot, label="Áudio (Canal 1)")
    plt.title("Áudio WAV - 5 segundos")
    plt.xlabel("Tempo [s]")
    plt.ylabel("Amplitude")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()

# Chamar a função com o nome do arquivo WAV
wav_filename = 'filtered_audio_highpass.wav'  # Substitua pelo nome do seu arquivo
plot_audio_file(wav_filename)
