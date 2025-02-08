from pydub import AudioSegment
from pydub.silence import detect_nonsilent

# Função para remover o silêncio do início e do final do arquivo WAV
def remove_silence(input_wav, output_wav, silence_thresh=-50, min_silence_len=100):
    """
    Remove o silêncio do início e do final de um arquivo de áudio WAV.
    
    Args:
        input_wav (str): Caminho para o arquivo WAV de entrada.
        output_wav (str): Caminho para salvar o arquivo WAV sem silêncio.
        silence_thresh (int): Limite de detecção de silêncio em dBFS. Amplitude abaixo deste valor será considerada silêncio.
        min_silence_len (int): Comprimento mínimo de silêncio em milissegundos para ser considerado.
    """
    # Carregar o arquivo de áudio
    audio = AudioSegment.from_wav(input_wav)

    # Detectar trechos que não são silêncio
    nonsilent_ranges = detect_nonsilent(audio, min_silence_len=min_silence_len, silence_thresh=silence_thresh)

    if nonsilent_ranges:
        # Determinar os limites do áudio sem silêncio
        start_trim = nonsilent_ranges[0][0]  # Início do áudio significativo
        end_trim = nonsilent_ranges[-1][1]   # Final do áudio significativo
        
        # Recortar o áudio para remover o silêncio do início e do final
        trimmed_audio = audio[start_trim:end_trim]
        
        # Salvar o áudio sem silêncio
        trimmed_audio.export(output_wav, format="wav")
        print(f"Silêncio removido. Arquivo salvo como: {output_wav}")
    else:
        print("Nenhuma faixa não silenciosa detectada. O arquivo de áudio é totalmente silencioso.")

# Exemplo de uso
input_wav = '/home/joselito/git/tcc/audio_files/ref_300.wav'  # Substitua pelo seu arquivo de entrada
output_wav = '/home/joselito/git/tcc/audio_files/ref_300_cortado.wav'  # Caminho para salvar o arquivo sem silêncio

remove_silence(input_wav, output_wav, silence_thresh=-50, min_silence_len=100)
