import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

# Função para calcular e plotar a FFT de um arquivo de áudio
def plot_fft(arquivo, ax, label, suavizacao_tamanho=1000):
    # Carregar o arquivo de áudio
    taxa_amostragem, dados = wavfile.read(arquivo)
    
    # Caso o áudio tenha mais de uma coluna (por exemplo, estéreo), pegar apenas uma coluna
    if len(dados.shape) > 1:
        dados = dados[:, 0]
    
    # Calcular a FFT
    fft_dados = np.fft.fft(dados)
    fft_dados = np.abs(fft_dados)[:len(fft_dados)//2]  # Considerando apenas a metade positiva da FFT
    frequencias = np.fft.fftfreq(len(dados), 1 / taxa_amostragem)[:len(fft_dados)]

    # Suavização com média móvel
    fft_dados_suavizados = np.convolve(fft_dados, np.ones(suavizacao_tamanho)/suavizacao_tamanho, mode='same')
    
    # Plotar a FFT com frequência em log e magnitude linear
    ax.plot(frequencias, fft_dados_suavizados, label=label)
    ax.set_xlabel('Frequência [Hz]')
    ax.set_ylabel('Magnitude [Linear]')
    ax.set_title(f'FFT de {label}')
    
    # Ajustar para o eixo X ser logarítmico e limitar o intervalo
    ax.set_xscale('log')
    ax.set_xlim([20, 22050])  # Limitar o eixo de frequência de 20Hz a 22050Hz
    
    ax.grid(True)
    ax.legend()

# Janela 1 com dois gráficos, cada gráfico para um arquivo diferente
fig1, axs1 = plt.subplots(2, 1, figsize=(10, 12))  # 2 subgráficos na Janela 1

# Arquivos para Janela 1
arquivos_1 = [
    "/home/joselito/git/tcc/datas/audio01.wav",
    "/home/joselito/git/tcc/datas/audio02.wav"
]

# Plotar os arquivos na Janela 1 (dois gráficos)
for i, arquivo in enumerate(arquivos_1):
    plot_fft(arquivo, axs1[i], label=arquivo.split("/")[-1])

# Janela 2 com três gráficos, cada gráfico para um arquivo diferente
fig2, axs2 = plt.subplots(3, 1, figsize=(10, 18))  # 3 subgráficos na Janela 2

# Arquivos para Janela 2
arquivos_2 = [
    "/home/joselito/git/tcc/scripts/saida_filtrada_1.wav",
    "/home/joselito/git/tcc/scripts/saida_filtrada_2.wav",
    "/home/joselito/git/tcc/scripts/saida_filtrada_soma.wav"
]

# Plotar os arquivos na Janela 2 (três gráficos)
for i, arquivo in enumerate(arquivos_2):
    plot_fft(arquivo, axs2[i], label=arquivo.split("/")[-1])

# Ajustar layout para não sobrepor os subgráficos
fig1.tight_layout()
fig2.tight_layout()

# Exibir os gráficos
plt.show()
