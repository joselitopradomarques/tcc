import numpy as np
import matplotlib.pyplot as plt

# Função para calcular fc2 baseado em uma transformação inversa logarítmica
def calcular_frequencia_corte_log_inverso(fc1):
    # A relação logarítmica inversa
    fc2_log = np.log10(22050) - (np.log10(fc1) - np.log10(20))
    return 10**fc2_log  # Voltando para a escala linear após a transformação log

# Gerando 30 valores de fc1 logaritmicamente espaçados
fc1_values = np.logspace(np.log10(20), np.log10(22050), num=30)
fc2_values = [calcular_frequencia_corte_log_inverso(fc1) for fc1 in fc1_values]

# Plotando apenas a linha de tendência
plt.figure(figsize=(10, 6))
plt.plot(fc1_values, fc2_values, color='b')

# Definindo título e rótulos dos eixos
plt.title("Mudança Inversa de Frequência de Corte em Gráfico Log-Log")
plt.xlabel("fc1 (Hz) - Frequência de Corte Atual")
plt.ylabel("fc2 (Hz) - Nova Frequência de Corte")
plt.xscale("log")
plt.yscale("log")

# Definindo os limites dos eixos para 20 Hz a 22050 Hz
plt.xlim(20, 22050)
plt.ylim(20, 22050)

# Adicionando uma grade e exibindo o gráfico
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.tight_layout()
plt.show()
