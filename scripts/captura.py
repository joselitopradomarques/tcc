import alsaaudio
import numpy as np

def capture_and_print():
    # Defina o dispositivo de captura e as configurações
    input_device = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, alsaaudio.PCM_NONBLOCK, device='hw:3,0')
    input_device.setchannels(2)
    input_device.setrate(44100)
    input_device.setformat(alsaaudio.PCM_FORMAT_S24_3LE)  # Formato S24_3LE
    input_device.setperiodsize(160)

    print("Capturando áudio do dispositivo hw:3,0...")

    while True:
        # Captura os dados de áudio
        _, audio_data = input_device.read()
        
        if len(audio_data) == 0:
            continue
        
        # Verifique se o tamanho do buffer é múltiplo de 3 antes de converter
        if len(audio_data) % 3 != 0:
            print(f"Erro: tamanho do buffer não é múltiplo de 3 (tamanho atual: {len(audio_data)})")
            continue
        
        # Converte os dados de áudio para o formato int32 (3 bytes por amostra)
        # Cada amostra tem 3 bytes (24 bits), então precisamos processá-los
        audio_data = np.frombuffer(audio_data, dtype=np.int8).reshape(-1, 3)

        # Converte os 3 bytes para um único inteiro de 32 bits
        # Considerando que a amostra está no formato little-endian
        audio_data = audio_data.dot([1, 256, 65536])

        # Exibe os dados capturados
        print(audio_data)

if __name__ == "__main__":
    capture_and_print()
