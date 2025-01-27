import mmap
import time

# Cria um arquivo temporário para ser mapeado na memória
with open("shared_memory.dat", "wb") as f:
    f.write(b"\0" * 1024)  # Inicializa o arquivo com 1024 bytes

# Mapeia o arquivo para a memória
with open("shared_memory.dat", "r+b") as f:
    mm = mmap.mmap(f.fileno(), 0)  # Mapeia o arquivo inteiro

    # Escreve algo na memória compartilhada
    mm[0:5] = b"Hello"

    print("Processo 1: dados escritos, esperando Process 2 ler...")
    
    # Espera que o Processo 2 modifique os dados
    while mm[5:6] != b'1':
        time.sleep(1)

    print("Processo 1: Processo 2 leu e alterou a memória!")
    
    mm.close()
