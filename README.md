# TCC - Implementação de Equipamento de Mixagem para DJs com Controle Automatizado de Filtros e Efeitos

Este repositório contém o código-fonte e os arquivos necessários para o projeto de TCC intitulado **"Implementação de Equipamento de Mixagem para DJs com Controle Automatizado de Filtros e Efeitos"**. O objetivo do projeto é desenvolver um sistema de mixagem de áudio que permita o controle centralizado e automatizado de filtros e efeitos sonoros, proporcionando uma experiência interativa e profissional para DJs.

### Hardware Necessário

Os detalhes sobre as conexões e a implementação do hardware estão descritos na seção **Metodologia - Implementação em Hardware** do arquivo pdf.

## Estrutura do Repositório

Abaixo estão as principais pastas e arquivos do repositório:

- `scripts/`: Contém os scripts e bibliotecas em C criadas para a implementação do código para o sistema de mixagem de áudio.
- `texto/`: Pasta que contém os arquivos LaTeX, bem como figuras e gráficos.
- `README.md`: Este arquivo, que fornece uma visão geral do projeto.

# Descrição e Função de Cada Arquivo da Biblioteca

- **`scripts/`**: Contém os scripts principais do sistema de mixagem de áudio em C.
  - **`princ.c`**: Arquivo principal que inicializa o definições iniciais de variáveis. Além disso, a leitura dos sinais, criação de buffers e a chamada do processamento é realizada nessa função.
  - **`proc.c`**: Onde se encontra a maioria das funções implementadas. Desde leitura e escrita (para teste) de arquivos wav, geração de buffers, processamento geral do sinal, liberação de buffers e aplicação do filtro implementado.
  - **`adc.c`**: Contém funções para leitura de potenciômetro e de botão.
  - **`audio.c`**: Responsável pela inicialização, configuração e reprodução a partir do dispositivo de áudio na Raspberry Pi.
  - **`filt.c`**: Lida com criação de estruturas necessárias para o filtro como lista de frequências e coeficientes a serem utilizados no filtro FIR. 
  - **`reverb.c`**: Implementação do efeito de reverb.
  - **`delay.c`**: Implementação do efeito de delay.
  - **`gerar_silencio.c`**: Função que gera arquivo wav em silêncio para ser utilizado em teste de filtragem.
  - **`filter_ex.c`**: Implementação isolada do filtro e aplicada a um arquivo local. Função de teste de filtro.
---

# Como Compilar e Executar o Programa

1. **Compilação**:
   Para compilar o programa, primeiro certifique-se de ter os pacotes necessários instalados, como `gcc`, `alsa/asoundlib` e ` WiringPi` (ou outras bibliotecas necessárias para processamento de áudio). Execute o seguinte comando para compilar:

   ```bash
    gcc -o princ princ.c proc.c delay.c adc.c filt.c audio.c reverb.c -lpthread -lm -lwiringPi -lasound
    ./princ

# Vídeo de Teste

Assista ao vídeo de teste do sistema em funcionamento, onde é demonstrada a utilização do equipamento de mixagem, incluindo os efeitos e filtros aplicados ao áudio.
https://youtu.be/0udQA0VLunk

