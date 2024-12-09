import matplotlib.pyplot as plt
from scipy.io.wavfile import read

# Função para carregar o arquivo WAV
def load_wav(file_path):
    sample_rate, data = read(file_path)
    return sample_rate, data

# Função para plotar os sinais
def plot_signals(signal1, signal2, sample_rate, label1, label2):
    # Definir o tempo para os gráficos
    time = [i / sample_rate for i in range(len(signal1))]

    # Criar o gráfico
    plt.figure(figsize=(10, 6))
    
    # Plotando o primeiro sinal
    plt.subplot(2, 1, 1)
    plt.plot(time, signal1, label=label1)
    plt.title(f"{label1} - Sinal no Domínio do Tempo")
    plt.xlabel("Tempo [s]")
    plt.ylabel("Amplitude")
    plt.legend()

    # Plotando o segundo sinal
    plt.subplot(2, 1, 2)
    plt.plot(time, signal2, label=label2, color='r')
    plt.title(f"{label2} - Sinal no Domínio do Tempo")
    plt.xlabel("Tempo [s]")
    plt.ylabel("Amplitude")
    plt.legend()

    plt.tight_layout()
    plt.show()

# Caminho para os arquivos WAV
file_delay = '/home/joselito/git/tcc/scripts/effect_output_delay.wav'
file_reverb = '/home/joselito/git/tcc/scripts/effect_output_reverb.wav'

# Carregar os arquivos WAV
sample_rate_delay, signal_delay = load_wav(file_delay)
sample_rate_reverb, signal_reverb = load_wav(file_reverb)

# Certificar-se de que ambos os sinais têm o mesmo número de amostras
if len(signal_delay) != len(signal_reverb):
    print("Aviso: os sinais têm tamanhos diferentes!")

# Plotar os sinais no domínio do tempo
plot_signals(signal_delay, signal_reverb, sample_rate_delay, 'Delay', 'Reverb')
