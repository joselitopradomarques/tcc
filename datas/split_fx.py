from pydub import AudioSegment
from pydub.silence import split_on_silence

def recortar_silencio(arquivo_entrada, arquivo_saida):
    # Carregar o arquivo WAV
    audio = AudioSegment.from_wav(arquivo_entrada)
    
    # Obter a taxa de amostragem original
    taxa_amostragem = audio.frame_rate
    
    # Dividir o áudio em segmentos com base no silêncio
    segmentos = split_on_silence(audio, silence_thresh=audio.dBFS-14)
    
    # Concatenar os segmentos de áudio
    audio_silencio_removido = AudioSegment.empty()
    for segmento in segmentos:
        audio_silencio_removido += segmento
    
    # Exportar o áudio final para o arquivo de saída com a taxa de amostragem original
    audio_silencio_removido.export(arquivo_saida, format="wav", parameters=["-ar", str(taxa_amostragem)])

# Exemplo de uso
recortar_silencio("/home/joselito/git/tcc/PureData/case19.wav", "/home/joselito/git/tcc/PureData/case19_cut.wav")