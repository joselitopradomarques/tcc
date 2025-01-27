import sounddevice as sd
import numpy as np
import mmap
import time

# Parâmetros de captura
rate = 44100  # Taxa de amostragem (Hz)
channels = 2  # Número de canais (estéreo)
frame_size = 1024  # Tamanho do frame para a captura

# Obter o índice do dispositivo de captura
device_index = 3  # Aqui você especifica o índice do dispositivo hw:3,0
device_info = sd.query_devices(device_index)
print(f"Dispositivo selecionado: {device_info['name']}")

# Criar arquivo de memória compartilhada
shm = mmap.mmap(-1, frame_size * channels * 4)  # 4 bytes por amostra (32 bits)
print("Capturando áudio e enviando para memória compartilhada...")

# Função para capturar e enviar áudio
def capture_and_send():
    audio_data = np.zeros(frame_size * channels, dtype=np.int32)
    try:
        while True:
            # Captura áudio do dispositivo especificado
            audio_data = sd.rec(frame_size, samplerate=rate, channels=channels, dtype='int32', device=device_index)
            sd.wait()

            # Escrever dados na memória compartilhada
            shm.seek(0)  # Ir para o início da memória
            shm.write(audio_data.tobytes())  # Escrever dados no bloco de memória

            # Lendo os bytes que foram escritos na memória compartilhada e imprimindo-os
            shm.seek(0)  # Voltar para o início da memória compartilhada
            audio_bytes = shm.read(frame_size * channels * 4)  # Ler os dados escritos
            print("Python: Dados lidos da memória compartilhada:")
            for i in range(0, len(audio_bytes), 4):  # Lendo os bytes por 4 (tamanho de cada amostra)
                print(f"Byte {i // 4}: {int.from_bytes(audio_bytes[i:i+4], 'little')}")
            
            # Aguarda um tempo antes de capturar o próximo bloco de áudio
            time.sleep(0.1)

    except Exception as e:
        print(f"Erro durante a captura de áudio: {e}")
        shm.close()

# Iniciar captura e envio para memória compartilhada
capture_and_send()
