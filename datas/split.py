from pydub import AudioSegment
from pydub.silence import split_on_silence

def recortar_silencio(arquivo_entrada, arquivo_saida, duracao_final_ms=8000):
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
    
    # Verificar se a duração do áudio é maior que a duração final desejada
    if len(audio_silencio_removido) > duracao_final_ms:
        # Cortar o áudio para a duração desejada
        audio_silencio_removido = audio_silencio_removido[:duracao_final_ms]
    elif len(audio_silencio_removido) < duracao_final_ms:
        # Adicionar silêncio para atingir a duração desejada
        silencio_adicional = AudioSegment.silent(duration=(duracao_final_ms - len(audio_silencio_removido)))
        audio_silencio_removido += silencio_adicional
    
    # Exportar o áudio final para o arquivo de saída com a taxa de amostragem original
    audio_silencio_removido.export(arquivo_saida, format="wav", parameters=["-ar", str(taxa_amostragem)])

# Exemplo de uso
recortar_silencio("/home/joselito/git/tcc/datas/audio02.wav", "/home/joselito/git/tcc/PureData/case10_cut.wav")
