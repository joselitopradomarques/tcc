import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, freqz

# Definir parâmetros
fs = 44100  # Frequência de amostragem (Hz)
fc = 1000   # Frequência de corte (Hz)
ordens = [10, 20, 30, 50, 70, 100]  # Diferentes ordens do filtro
janela = 'hamming'  # Usar janela Hamming
atenuacao_desejada = -26  # Atenuação desejada na banda rejeitada (em dB)

# Função para projetar filtro FIR
def fir_filter_design(numtaps, window, fc, fs):
    if numtaps % 2 == 0:
        numtaps += 1  # Garantir que seja ímpar
    coeficientes = firwin(numtaps, fc, window=window, fs=fs, pass_zero=False)
    return coeficientes

# Função para calcular banda de transição
def calcular_banda_transicao(coeficientes, fs):
    # Obter resposta em frequência
    w, h = freqz(coeficientes, worN=8000)
    freqs = (fs * 0.5 / np.pi) * w  # Converter frequência de radianos para Hz
    magnitude_db = 20 * np.log10(np.abs(h))  # Converter magnitude para dB
    
    # Encontrar a frequência onde a atenuação é de -26 dB
    idx_atenuacao = np.argmax(magnitude_db <= atenuacao_desejada)
    if idx_atenuacao == 0:  # Se não encontrou, retorna valor grande
        return np.inf
    frequencia_atenuacao = freqs[idx_atenuacao]
    
    # Banda de transição: diferença entre a frequência de atenuação e a frequência de corte
    banda_transicao = frequencia_atenuacao - fc
    return banda_transicao

# Função para plotar a resposta em frequência
def plot_frequency_response(coeficientes, fs, numtaps):
    w, h = freqz(coeficientes, worN=8000)
    freqs = (fs * 0.5 / np.pi) * w  # Converter frequência de radianos para Hz
    magnitude_db = 20 * np.log10(abs(h))  # Converter magnitude para dB
    
    # Plotar a resposta em frequência
    plt.semilogx(freqs, magnitude_db, label=f"Ordem: {numtaps}")
    plt.axvline(fc, color='g', linestyle='--')  # Mantém a linha da frequência de corte, mas sem legenda
    plt.axhline(atenuacao_desejada, color='r', linestyle='--')  # Mantém a linha de -26 dB, sem legenda
    plt.title('Resposta em Frequência (Escala Logarítmica e Amplitude em dB)')
    plt.xlabel('Frequência (Hz)')
    plt.ylabel('Magnitude (dB)')
    plt.grid(which='both', linestyle='--', linewidth=0.5)

# Aplicar filtros e calcular banda de transição para cada ordem
plt.figure(figsize=(10, 6))

for ordem in ordens:
    coeficientes = fir_filter_design(ordem, janela, fc, fs)
    banda_transicao = calcular_banda_transicao(coeficientes, fs)
    
    # Imprimir banda de transição
    print(f'Ordem {ordem}: Banda de transição = {banda_transicao:.2f} Hz')
    
    # Plotar a resposta em frequência
    plot_frequency_response(coeficientes, fs, ordem)

plt.legend(loc='upper right')
plt.show()
