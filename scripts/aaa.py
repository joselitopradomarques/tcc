import numpy as np
import matplotlib.pyplot as plt

ORDER = 121
CUTOFF_FREQUENCY = 0.0008333  # Frequência de corte normalizada para 20 Hz com fs = 48 kHz


# Função para gerar o filtro passa-altas com a janela de Hamming
def generate_high_pass_filter(order, cutoff_frequency):
    # Frequência de corte
    wc = 2 * np.pi * cutoff_frequency
    filter = np.zeros(order)
    sum_filter = 0.0

    # Geração dos coeficientes
    for n in range(order):
        if n == (order - 1) // 2:
            filter[n] = 1 - wc / np.pi  # Coeficiente central (0)
        else:
            filter[n] = np.sin(wc * (n - (order - 1) // 2)) / (np.pi * (n - (order - 1) // 2))  # Resposta impulsiva

        # Aplicando a janela de Hamming
        filter[n] *= 0.54 - 0.46 * np.cos(2 * np.pi * n / (order - 1))

        # Somando para normalizar a resposta
        sum_filter += filter[n]

    # Normalizar os coeficientes
    filter /= sum_filter
    return filter

# Gerando o filtro passa-altas
filter = generate_high_pass_filter(ORDER, CUTOFF_FREQUENCY)

# Imprimindo os coeficientes
print("Coeficientes do filtro passa-altas com janela de Hamming:")
for i, coef in enumerate(filter):
    print(f"coef[{i}] = {coef:.6f}")

# Plotando o gráfico dos coeficientes
plt.plot(filter)
plt.title('Coeficientes do Filtro Passa-Altas com Janela de Hamming')
plt.xlabel('Índice')
plt.ylabel('Coeficiente')
plt.grid(True)
plt.show()
