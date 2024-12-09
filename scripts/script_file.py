import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import firwin, freqz
from scipy.fft import fft

# Função para ler arquivos WAV
def ler_wav(filename):
    fs, sinal = wavfile.read(filename)
    return fs, sinal

# Função para escrever arquivos WAV
def escrever_wav(filename, fs, sinal):
    wavfile.write(filename, fs, sinal.astype(np.int16))

# Definir parâmetros
rp = 20  # Ripple na banda passante (dB) - Não utilizado para filtro FIR
rs = 26  # Atenuação desejada na banda rejeitada (dB) - Não utilizado para filtro FIR
fc = 200  # Frequência de corte (Hz)
fs = 44100  # Frequência de amostragem (Hz)
tamanho_filtro = 10000  # Tamanho do filtro FIR (número de coeficientes)

# Nome do arquivo de entrada e saída
input_wav = '/home/joselito/git/tcc/datas/audio02.wav'  # Caminho do arquivo de entrada
output_wav = '/home/joselito/git/tcc/datas/saida_filtrada.wav'  # Caminho completo do arquivo de saída

# Função para projetar filtro FIR com janela Hamming
def hamming_filter_design(fc, fs, tamanho_filtro):
    nyquist = 0.5 * fs
    cutoff = fc / nyquist
    # Projetando o filtro FIR com janela Hamming
    b = firwin(tamanho_filtro, cutoff, window='hamming')
    a = 1  # Filtros FIR têm coeficientes de a iguais a 1
    return b, a

# Função para aplicar filtro ao sinal (mono ou multicanal)
def aplicar_filtro_multicanal(sinal, b, a):
    if sinal.ndim == 1:  # Sinal mono
        return np.apply_along_axis(lambda x: np.convolve(x, b, mode='same'), axis=0, arr=sinal)
    else:  # Sinal com múltiplos canais (por exemplo, estéreo)
        sinal_filtrado = np.zeros_like(sinal)
        for canal in range(sinal.shape[1]):
            sinal_filtrado[:, canal] = np.convolve(sinal[:, canal], b, mode='same')
        return sinal_filtrado

# Função para calcular o espectro de um sinal
def calcular_espectro(sinal, fs):
    N = len(sinal)
    T = 1.0 / fs
    yf = fft(sinal)
    xf = np.fft.fftfreq(N, T)[:N//2]
    magnitude_sinal = 2.0 / N * np.abs(yf[:N//2])
    magnitude_db_sinal = 20 * np.log10(magnitude_sinal)
    
    # Substituindo valores infinitamente negativos por -100 para a visualização
    magnitude_db_sinal[magnitude_db_sinal == -np.inf] = -100
    
    return xf, magnitude_db_sinal

# Função para plotar espectro do sinal filtrado
def plot_sinal_filtrado(b, a, sinal_filtrado, fs, fc):
    # Resposta em Frequência do Filtro
    w, h = freqz(b, a, worN=8000)
    freqs_filtro = (fs * 0.5 / np.pi) * w
    magnitude_db_filtro = 20 * np.log10(np.abs(h))
    
    # Espectro do Sinal Filtrado
    xf, magnitude_db_sinal = calcular_espectro(sinal_filtrado, fs)
    
    # Plotando
    plt.figure(figsize=(10, 6))
    plt.semilogx(freqs_filtro, magnitude_db_filtro, label='Resposta do Filtro (dB)', color='blue')
    plt.semilogx(xf, magnitude_db_sinal, label='Espectro do Sinal Filtrado (dB)', color='orange')
    plt.axvline(fc, color='green', linestyle='--', label=f'Frequência de Corte: {fc} Hz')
    plt.title('Comparação: Resposta do Filtro e Espectro do Sinal Filtrado')
    plt.xlabel('Frequência (Hz) [Escala Logarítmica]')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    plt.legend(loc='upper right')
    plt.show()

# Função para plotar o espectro do sinal antes de ser filtrado
def plot_espectro_original(sinal, fs):
    xf_original, espectro_original = calcular_espectro(sinal, fs)
    
    plt.figure(figsize=(10, 6))
    plt.semilogx(xf_original, espectro_original, label='Espectro do Sinal Original (dB)', color='purple')
    plt.title('Espectro do Sinal Original Antes da Filtragem')
    plt.xlabel('Frequência (Hz) [Escala Logarítmica]')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    plt.legend(loc='best')
    plt.show()

# Carregar arquivo WAV de entrada
fs, sinal = ler_wav(input_wav)

# Plotar o espectro do sinal original
plot_espectro_original(sinal, fs)

# Projetar filtro FIR com janela Hamming
b, a = hamming_filter_design(fc, fs, tamanho_filtro)

# Aplicar o filtro ao sinal
sinal_filtrado = aplicar_filtro_multicanal(sinal, b, a)

# Normalização do sinal filtrado para ter ganho de 0 dB na banda passante
# Calculando a média da magnitude do sinal original na banda passante
_, mag_original = calcular_espectro(sinal, fs)
mag_original_banda_passante = mag_original[mag_original < fc]  # Frequências abaixo de fc

# Calculando a média da magnitude do sinal filtrado na mesma banda
_, mag_filtrado = calcular_espectro(sinal_filtrado, fs)
mag_filtrado_banda_passante = mag_filtrado[mag_filtrado < fc]

# Calculando o fator de normalização
fator_normalizacao = np.mean(mag_original_banda_passante) / np.mean(mag_filtrado_banda_passante)

# Normalizando o sinal filtrado
sinal_filtrado_normalizado = sinal_filtrado * fator_normalizacao

# Salvar o sinal filtrado no arquivo de saída
escrever_wav(output_wav, fs, sinal_filtrado_normalizado)

# Plotar o espectro do sinal filtrado junto com a resposta em frequência do filtro
if sinal_filtrado_normalizado.ndim > 1:
    plot_sinal_filtrado(b, a, sinal_filtrado_normalizado[:, 0], fs, fc)
else:
    plot_sinal_filtrado(b, a, sinal_filtrado_normalizado, fs, fc)
