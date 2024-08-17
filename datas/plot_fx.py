import numpy as np
import matplotlib.pyplot as plt

# Definindo a função
def expr(f):
    term1 = np.exp(-((f - 11000) / 1000) ** 2)
    term2 = np.exp((f - 11000) / 2000)
    return (term1 / (term1 + term2)) * 2

# Intervalo de f
f = np.linspace(0, 24000, 1000)  # de 0 a 24000 Hz com 1000 pontos

# Calculando a expressão
y = expr(f)

# Plotando o gráfico
plt.plot(f, y)
plt.title('Plot da Expressão')
plt.xlabel('f (Hz)')
plt.ylabel('Amplitude')
plt.grid(True)
plt.show()
