def calcular_duracao_em_segundos(freq_amostragem, quadros_por_buffer, n_buffers):
    # A duração de um buffer em segundos
    duracao_buffer = quadros_por_buffer / freq_amostragem
    
    # A duração total para n buffers
    duracao_total = duracao_buffer * n_buffers*1000
    
    return duracao_total

# Exemplo de uso:
freq_amostragem = 44100  # frequência de amostragem em Hz
quadros_por_buffer = 1024  # quantidade de quadros em um buffer
n_buffers = 1  # número de buffers

duracao = calcular_duracao_em_segundos(freq_amostragem, quadros_por_buffer, n_buffers)
print(f"A duração total de {n_buffers} buffers é {duracao:.1f} milisegundos.")
