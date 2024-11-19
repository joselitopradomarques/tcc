# Desenvolvimento do Hardware

## 1. Desenvolvimento de Esquemas Elétricos
- **Objetivo**: Criar esquemas elétricos detalhados para interligar todos os componentes.
- **Tarefas**:
  - Diagramar conexões entre a Raspberry Pi e os conversores AD/DA.
    -- Diagrama contendo apenas a conexão dos conversores AD/DA ao GPIO
    -- Informar o protocolo utilizado
  - Criar o esquema para conectar os controles (botões, sliders) aos pinos GPIO.
    -- Diagrama contendo apenas os controles: botões e sliders ao GPIO
    -- Informar o protocolo utilizado
  - Criar um diagrama geral com todos componentes sem detalhes das conexões
  - Criar um diagrama para cada relação entre a Raspberry Pi e componentes

## 2. Prototipagem de Circuito em Protoboard
- **Objetivo**: Montar uma versão prototipada do circuito para testes preliminares.
- **Tarefas**:
  - Montar o circuito em uma **protoboard** para testar cada relação com componente externo.
    -- Testar aquisição do controle On/Off de Efeito
    -- Testar aquisição de controle de frequência
    -- Testar aquisição de controle de efeito
    -- Testar aquisição de sinais digitais
    -- Testar saída do áudio analógico

## 3. Integração dos Controles Físicos
- **Objetivo**: Integrar os controles físicos (botões, sliders) ao sistema.
- **Tarefas**:
  - Conectar os controles aos pinos GPIO da Raspberry Pi.
  - Programar a leitura dos controles no software para ajustar parâmetros de filtros e efeitos.

## 4. Testes de Funcionalidade do Hardware
- **Objetivo**: Testar o hardware para verificar seu funcionamento completo.
- **Tarefas**:
  - Testar cada funcionalidade acima em funcionamento com o software.

## 5. Otimização e Refinamento do Hardware
- **Objetivo**: Otimizar a performance do hardware, garantindo estabilidade.
- **Tarefas**:
  - Organizar cabos e fiação para minimizar ruídos e interferências.
  - Otimizar a alimentação para garantir funcionamento estável.

## 6. Documentação do Hardware
- **Objetivo**: Documentar o desenvolvimento do hardware para manutenção e referência futura.
- **Tarefas**:
  - Documentar os esquemas elétricos e componentes utilizados.
  - Fornecer instruções para montagem e manutenção.
  - Descrever a interação entre hardware e software.


- Opções para conversão ADC 

1) Interface de áudio USB Behringer U-Control UCA222 
Preço: 299
Procotolo: USB
Link: https://www.amazon.com.br/Interface-de-%C3%81udio-UCA222-Behringer/dp/B0023BYDHK?source=ps-sl-shoppingads-lpcontext&ref_=fplfs&psc=1&smid=A3ISYVBX8D20DH

2) Módulo Aquisição WM8782s da Cirrus Logic
Preço: 167
Protocolo: I2S
https://pt.aliexpress.com/item/1005006488091513.html?spm=a2g0o.detail.pcDetailTopMoreOtherSeller.2.66e9Bn0FBn0FhC&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40050.354490.0&scm_id=1007.40050.354490.0&scm-url=1007.40050.354490.0&pvid=d7fb2187-74de-4fe9-b865-362747c71b30&_t=gps-id:pcDetailTopMoreOtherSeller,scm-url:1007.40050.354490.0,pvid:d7fb2187-74de-4fe9-b865-362747c71b30,tpp_buckets:668%232846%238110%231995&pdp_npi=4%40dis%21BRL%2199.76%2161.86%21%21%2116.37%2110.15%21%402103209b17307310441096123ead8a%2112000037389495740%21rec%21BR%212330304721%21X&utparam-url=scene%3ApcDetailTopMoreOtherSeller%7Cquery_from%3A

3) PCM Chips em I2S
 Em torno de 40 cada uma 
