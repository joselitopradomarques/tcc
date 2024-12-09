import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import firwin
from scipy.fft import fft

# Definir parâmetros
fs = 44100  # Frequência de amostragem (Hz)
fc = 300     # Frequência de corte (Hz)
# Ajustar a ordem e janela para maior atenuação
ordem = 500
janela = 'blackman'


# Nome do arquivo de entrada e saída
input_wav = '/home/joselito/git/tcc/datas/audio02.wav'  # Caminho do arquivo de entrada
output_wav = '/home/joselito/git/tcc/datas/saida_filtrada.wav'  # Caminho completo do arquivo de saída
output_txt = '/home/joselito/git/tcc/datas/valores_brutos.txt'  # Caminho completo do arquivo de saída .txt

# Função para normalizar o sinal entre -1 e 1, assumindo sinal de 16 bits
def normalizar_sinal(sinal):
    max_possible_val = 32767  # Máximo valor absoluto possível em 16 bits
    sinal_normalizado = sinal / max_possible_val  # Normalizar em função do valor máximo de 16 bits
    return sinal_normalizado

# Função para desnormalizar o sinal após a filtragem
def desnormalizar_sinal(sinal_normalizado):
    max_possible_val = 32767  # Retorna ao valor máximo de 16 bits
    return sinal_normalizado * max_possible_val


# Função para ler e escrever arquivos WAV
def ler_wav(filename):
    fs, sinal = wavfile.read(filename)
    return fs, sinal

def escrever_wav(filename, fs, sinal):
    wavfile.write(filename, fs, sinal.astype(np.int16))

# Função para projetar filtro FIR
def fir_filter_design(numtaps, window, fc, fs):
    if numtaps % 2 == 0:
        numtaps += 1  # Garantir que seja ímpar
    coeficientes = firwin(numtaps, fc, window=window, fs=fs, pass_zero=False)
    return coeficientes

# Função para aplicar filtro FIR ao sinal, processando canais separadamente
def aplicar_filtro_multicanal(sinal, coeficientes):
    if sinal.ndim == 1:  # Sinal mono
        return np.convolve(sinal, coeficientes, mode='same')
    else:  # Sinal com múltiplos canais (por exemplo, estéreo)
        sinal_filtrado = np.zeros_like(sinal)
        for canal in range(sinal.shape[1]):
            sinal_filtrado[:, canal] = np.convolve(sinal[:, canal], coeficientes, mode='same')
        return sinal_filtrado
    
# Função para imprimir os coeficientes no formato C
def imprimir_coeficientes_em_c(coeficientes):
    coeficientes_c = ', '.join([f'{coef:.8f}' for coef in coeficientes])
    print(f'float coeficientes[ORDEM] = {{ {coeficientes_c} }};')
# Código principal
# 1. Projetar o filtro FIR
coeficientes = fir_filter_design(ordem, janela, fc, fs)

# 2. Imprimir os coeficientes no formato adequado para código em C
imprimir_coeficientes_em_c(coeficientes)
# Função para plotar o espectro linear do sinal
def plot_spectrum_linear(sinal, fs, title):
    N = len(sinal)
    T = 1.0 / fs
    yf = fft(sinal)
    xf = np.fft.fftfreq(N, T)[:N//2]
    magnitude = 2.0 / N * np.abs(yf[:N//2])
    
    plt.figure(figsize=(10, 6))
    plt.plot(xf, magnitude)
    plt.title(title)
    plt.xlabel('Frequência (Hz)')
    plt.ylabel('Magnitude (Linear)')
    plt.grid(True)
    plt.show()

# Função para plotar a composição em frequência (espectro) em dB com escala logarítmica
def plot_spectrum_db_log(sinal, fs, title):
    N = len(sinal)
    T = 1.0 / fs
    yf = fft(sinal)
    xf = np.fft.fftfreq(N, T)[:N//2]
    magnitude = 2.0 / N * np.abs(yf[:N//2])
    magnitude_db = 20 * np.log10(magnitude)  # Converte para dB
    
    # Evitar problemas logarítmicos com valores zero ou negativos
    magnitude_db[magnitude_db == -np.inf] = -100  # Definir um valor mínimo para dB
    
    plt.figure(figsize=(10, 6))
    plt.semilogx(xf, magnitude_db)  # Frequência em escala logarítmica
    plt.title(title)
    plt.xlabel('Frequência (Hz) [Escala Logarítmica]')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    plt.show()

# Função para plotar o sinal no domínio do tempo
def plot_time_signal(sinal, fs, title):
    plt.figure(figsize=(10, 6))
    plt.plot(np.arange(len(sinal)) / fs, sinal)
    plt.title(title)
    plt.xlabel('Tempo (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.show()

# Função para salvar os valores brutos do sinal em um arquivo de texto
def salvar_valores_brutos_em_txt(sinal, filename):
    # Abrir o arquivo para escrita
    with open(filename, 'w') as f:
        # Escrever os valores de cada amostra do sinal
        for i in range(sinal.shape[0]):
            f.write(f'{sinal[i, 0]}\n')  # Salvando apenas o primeiro canal, se for estéreo

# Função para extrair e plotar 2 segundos do sinal filtrado no domínio do tempo
def plot_2s_filtered_signal(sinal_filtrado, fs, title):
    # Definir a quantidade de amostras para 2 segundos
    samples_2s = int(2 * fs)  # 2 segundos de sinal (fs = frequência de amostragem)
    
    # Garantir que o sinal tenha pelo menos 2 segundos de dados
    if len(sinal_filtrado) < samples_2s:
        print("O sinal filtrado é menor que 2 segundos.")
        sinal_filtrado_2s = sinal_filtrado  # Usar o sinal inteiro se for menor que 2 segundos
    else:
        sinal_filtrado_2s = sinal_filtrado[:samples_2s]  # Pegar apenas os primeiros 2 segundos
    
    # Plotar o sinal no domínio do tempo
    plt.figure(figsize=(10, 6))
    plt.plot(np.arange(len(sinal_filtrado_2s)) / fs, sinal_filtrado_2s)
    plt.title(title)
    plt.xlabel('Tempo (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.show()

# Carregar arquivo WAV de entrada
fs, sinal = ler_wav(input_wav)

# 1. Salvar os valores brutos do sinal em um arquivo de texto
salvar_valores_brutos_em_txt(sinal, output_txt)
print(f'Valores brutos do sinal salvos em: {output_txt}')

# 2. Plotar o sinal original (valores brutos)
plot_time_signal(sinal[:, 0], fs, 'Sinal Original (Valores Brutos)')

# 3. Plotar o espectro do sinal original sem normalização
plot_spectrum_db_log(sinal[:, 0], fs, 'Espectro do Sinal Original (Sem Normalização)')

# 4. Normalizar o sinal e plotar o espectro
sinal_normalizado = normalizar_sinal(sinal)

# Plotar espectro do sinal normalizado em dB (escala log)
plot_spectrum_db_log(sinal_normalizado[:, 0], fs, 'Espectro do Sinal Normalizado (dB)')

# Plotar espectro do sinal normalizado em escala linear
plot_spectrum_linear(sinal_normalizado[:, 0], fs, 'Espectro do Sinal Normalizado (Linear)')

# Projetar filtro FIR
coeficientes = fir_filter_design(ordem, janela, fc, fs)

# Aplicar o filtro ao sinal normalizado
sinal_filtrado = aplicar_filtro_multicanal(sinal_normalizado, coeficientes)
plot_spectrum_db_log(sinal_filtrado[:, 0], fs, 'Espectro do Sinal Normalizado Filtrado (dB)')
# 5. Desnormalizar o sinal filtrado
sinal_filtrado_desnormalizado = desnormalizar_sinal(sinal_filtrado)

# 6. Plotar o espectro do sinal filtrado desnormalizado
plot_spectrum_db_log(sinal_filtrado_desnormalizado[:, 0], fs, 'Espectro do Sinal Filtrado Desnormalizado')

# 7. Salvar o arquivo WAV filtrado e desnormalizado
escrever_wav(output_wav, fs, sinal_filtrado_desnormalizado)

# 8. Plotar os primeiros 2 segundos do sinal filtrado no domínio do tempo
plot_2s_filtered_signal(sinal_filtrado_desnormalizado[:, 0], fs, '2 Segundos do Sinal Filtrado no Domínio do Tempo')
