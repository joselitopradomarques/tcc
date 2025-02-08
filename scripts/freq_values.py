import numpy as np

# Definindo os valores mínimos e máximos para as frequências
fc1_min = 20.0
fc1_max = 24000.0
N = 11  # Número de pontos

# Gerando 32 pontos igualmente espaçados na escala logarítmica
frequencias_log = np.logspace(np.log10(fc1_min), np.log10(fc1_max), N)

print("\nFrequências em Escala Logarítmica:")
for i, freq in enumerate(frequencias_log):
    print(f"Índice {i + 1}: {freq:.2f} Hz")
