import os
import numpy as np
import matplotlib.pyplot as plt
from pydub import AudioSegment

# Função para carregar e extrair 5 segundos de áudio de um arquivo
def load_audio(filename, duration_ms=5000):
    # Carregar o arquivo de áudio
    audio = AudioSegment.from_wav(filename)
    # Cortar os primeiros 5 segundos
    audio = audio[:duration_ms]
    # Converter para um array numpy
    samples = np.array(audio.get_array_of_samples())
    
    # Se o arquivo for estéreo, pegar apenas um canal (como exemplo)
    if audio.channels == 2:
        samples = samples[::2]  # Pegando apenas um dos canais (canal esquerdo, por exemplo)
    
    return samples, audio.frame_rate

# Função para plotar o gráfico do áudio no domínio do tempo
def plot_audio_time_domain(samples, sample_rate, filename):
    # Gerar o vetor de tempo com base na taxa de amostragem
    time = np.linspace(0, len(samples) / sample_rate, num=len(samples))
    
    # Criar o gráfico
    plt.figure(figsize=(10, 4))
    plt.plot(time, samples, label='Sinal de Áudio', color='b')
    plt.title(f'Audiograma - {filename}')
    plt.xlabel('Tempo [s]')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.tight_layout()
    
    # Exibir o gráfico
    plt.show()

# Caminho do diretório onde estão os arquivos WAV
audio_directory = '/home/joselito/git/tcc/datas/'

# Listar todos os arquivos que seguem o padrão hpf_*.wav no diretório especificado
audio_files = [f for f in os.listdir(audio_directory) if f.startswith('hpf_') and f.endswith('.wav')]

# Processar cada arquivo e gerar o gráfico
for file in audio_files:
    try:
        # Construir o caminho completo do arquivo
        file_path = os.path.join(audio_directory, file)
        # Carregar o áudio e obter as amostras e a taxa de amostragem
        samples, sample_rate = load_audio(file_path)
        # Plotar o gráfico no domínio do tempo
        plot_audio_time_domain(samples, sample_rate, file)
    except Exception as e:
        print(f"Erro ao processar o arquivo {file}: {e}")
