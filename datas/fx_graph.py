import numpy as np
import matplotlib.pyplot as plt

# Valores de f1 (eixo horizontal)
f1_values = np.linspace(0.2, 22050, 1000)  # Valores de 0.2 a 22050

# Cálculo da expressão fornecida
expr_values = (np.exp(-np.power((f1_values - 11000) / 1000, 2)) / 
               (np.exp(-np.power((f1_values - 11000) / 1000, 2)) + 
                np.exp((f1_values - 11000) / 2000))) * 2

# Verificação dos valores calculados
print(f'Valor mínimo de expr_values: {np.min(expr_values)}')
print(f'Valor máximo de expr_values: {np.max(expr_values)}')

# Ajuste do limite superior do eixo y
ylim_upper = np.max(expr_values) * 1.2  # Menor intervalo para melhor visualização

# Plotando o gráfico
plt.figure(figsize=(10, 6))  # Aumenta a área do gráfico para melhor visualização

plt.plot(f1_values, expr_values)  # Plotando a função

plt.xlabel('$f_c$ (Hz)')
plt.ylabel('Volume do efeito')  # Modifica o nome do eixo y

# Remover a grade
plt.grid(False)  # Desativa a grade

# Definindo a proporção igual entre os eixos
plt.gca().set_aspect('auto', adjustable='box')

# Configurando os limites dos eixos
plt.xlim(0.2, 22050)
plt.ylim(0, ylim_upper)  # Define o limite superior do eixo y reduzido

plt.show()
