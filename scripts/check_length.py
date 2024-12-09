import wave

def contar_amostras_wav(filename):
    # Abre o arquivo WAV
    with wave.open(filename, 'rb') as wav_file:
        # Obtém o número de frames do arquivo (equivalente ao número total de amostras)
        num_frames = wav_file.getnframes()

    return num_frames

# Teste da função
filename = '/home/joselito/git/tcc/datas/saida_filtrada_estereo.wav'
amostras = contar_amostras_wav(filename)
print(f"O número total de amostras no arquivo WAV é: {amostras}")
