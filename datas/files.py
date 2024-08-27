import os

def listar_arquivos(diretorio):
    for root, dirs, files in os.walk(diretorio):
        for file in files:
            print(os.path.join(root, file))

# Exemplo de uso
diretorio = '/home/joselito/git/tcc'
listar_arquivos(diretorio)
