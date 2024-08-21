import numpy as np
import matplotlib.pyplot as plt

# Valores de fc_2 (eixo horizontal)
fc_2_values = np.linspace(1, 22050, 1000)  # Começar de 1 para evitar log(0)

# Cálculo da expressão para fc_1 (eixo vertical)
fc_1_values = 22050 - np.sqrt(22050**2 - (fc_2_values - 22050)**2)

# Plotando o gráfico
plt.figure(figsize=(8, 8))  # Proporção igual entre os eixos

plt.plot(fc_2_values, fc_1_values)  # Removido o rótulo

plt.xlabel('$fc_1$ (Hz)')
plt.ylabel('$fc_2$ (Hz)')

# Remover o título e a grade
plt.grid(False)  # Desativa a grade

# Definindo a proporção igual entre os eixos
plt.gca().set_aspect('equal', adjustable='box')

# Configurando os limites dos eixos para que a origem (0,0) coincida com a ponta inferior esquerda
plt.xlim(1, 22050)
plt.ylim(1, np.max(fc_1_values))

plt.show()
