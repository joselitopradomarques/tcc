import numpy as np
import matplotlib.pyplot as plt

# Definindo a função original (terceiro quadrante)
def expr(f):
    return 24000 - np.sqrt(24000**2 - (f - 24000)**2)

# Definindo o círculo completo
def circle(f):
    return np.sqrt(24000**2 - (f - 24000)**2) + 24000

# Intervalo de f para o terceiro quadrante
f_third_quadrant = np.linspace(0, 24000, 1000)

# Intervalo de f para o círculo completo
f_circle = np.linspace(0, 48000, 2000)

# Calculando os valores
y_third_quadrant = expr(f_third_quadrant)
y_circle_upper = circle(f_circle)  # Parte superior do círculo
y_circle_lower = 2*24000 - y_circle_upper  # Parte inferior do círculo

# Criando a figura e os eixos
plt.figure(figsize=(8, 8))

# Plotando o círculo completo
plt.plot(f_circle, y_circle_upper, 'b--', label='Parte superior do círculo')
plt.plot(f_circle, y_circle_lower, 'b--', label='Parte inferior do círculo')

# Plotando a equação do terceiro quadrante
plt.plot(f_third_quadrant, y_third_quadrant, 'r', label='Terceiro quadrante')

# Configurações do gráfico
plt.title('Comparação da Equação com o Círculo Completo')
plt.xlabel('f (Hz)')
plt.ylabel('y (Hz)')
plt.axhline(24000, color='gray', linestyle='--')  # Linha central horizontal
plt.axvline(24000, color='gray', linestyle='--')  # Linha central vertical
plt.legend()
plt.grid(True)
plt.axis('equal')  # Para garantir que o círculo pareça circular
plt.show()
