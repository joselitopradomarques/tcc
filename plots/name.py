import os
import shutil

# Caminho para o diretório onde os arquivos estão localizados
diretorio = '/home/joselito/git/tcc/plots/'

# Mudar para o diretório especificado
os.chdir(diretorio)

# Listar todos os arquivos que terminam com '.png'
arquivos = [f for f in os.listdir(diretorio) if f.endswith('.png')]

# Ordenar arquivos por nome
arquivos.sort()

# Índice inicial
indice = 24

# Copiar e criar novos arquivos com a nova nomenclatura
for i in range(1, 9):
    signal_file = f'signal_time_domain_case_{i}.png'
    heatmap_file = f'heatmap_spectral_case_{i}.png'
    
    # Verificar se o arquivo de sinal existe
    if signal_file in arquivos:
        novo_nome_signal = f'fig{indice}.png'
        shutil.copy(signal_file, novo_nome_signal)
        indice += 1

    # Verificar se o arquivo de heatmap existe
    if heatmap_file in arquivos:
        novo_nome_heatmap = f'fig{indice}.png'
        shutil.copy(heatmap_file, novo_nome_heatmap)
        indice += 1

print("Cópiamento dos arquivos concluído.")
