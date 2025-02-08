import numpy as np

# Definindo os limites
fc_min = 20.0  # Frequência mínima em Hz
fc_max = 664.19  # Frequência de ponto de igualdade em Hz

# Gerando 10 valores espaçados logaritmicamente entre fc_min e fc_max
frequencias_atual = np.logspace(np.log10(fc_min), np.log10(fc_max), 10)

# Função para calcular a nova frequência com a transformação log inversa
def calcular_nova_frequencia_corte(frequencia_corte_atual, fc_min=20.0, fc_max=22050.0):
    fc2_log = np.log10(fc_max) - (np.log10(frequencia_corte_atual) - np.log10(fc_min))
    return 10**fc2_log

# Calculando as novas frequências para cada frequência atual
frequencias_novas = [calcular_nova_frequencia_corte(fc_atual) for fc_atual in frequencias_atual]

# Criando a lista de pares
pares_frequencias = list(zip(frequencias_atual, frequencias_novas))

# Exibindo os pares
for i, (fc1, fc2) in enumerate(pares_frequencias):
    print(f"Par {i+1}: Frequência Atual = {fc1:.2f} Hz, Nova Frequência = {fc2:.2f} Hz")
