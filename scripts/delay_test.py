from pydub import AudioSegment
import numpy as np
import matplotlib.pyplot as plt

# Carregar as duas amostras WAV
audio1 = AudioSegment.from_wav('/home/joselito/git/tcc/datas/audio02.wav')
audio2 = AudioSegment.from_wav('/home/joselito/git/tcc/datas/saida_filtrada_delay.wav')

# Converter as amostras para arrays NumPy
samples1 = np.array(audio1.get_array_of_samples())
samples2 = np.array(audio2.get_array_of_samples())

# Garantir que as amostras tenham o mesmo tamanho
min_length = min(len(samples1), len(samples2))
samples1 = samples1[:min_length]
samples2 = samples2[:min_length]

# Calcular a diferença entre as amostras
difference = samples1 - samples2

# Plotar a diferença no domínio do tempo
plt.figure(figsize=(10, 6))
plt.plot(difference)
plt.title('Diferença entre as amostras WAV')
plt.xlabel('Amostras')
plt.ylabel('Amplitude')
plt.grid(True)
plt.show()
