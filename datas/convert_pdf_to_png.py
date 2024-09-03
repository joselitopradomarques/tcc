from pdf2image import convert_from_path
import os

def pdf_to_png(pdf_path, output_folder):
    # Converte o PDF em uma lista de imagens PIL (uma para cada página)
    images = convert_from_path(pdf_path)
    
    # Garante que o diretório de saída exista
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    
    for i, image in enumerate(images):
        # Define o caminho de saída para cada imagem PNG
        image_path = os.path.join(output_folder, f'page_{i + 1}.png')
        # Salva a imagem como PNG
        image.save(image_path, 'PNG')
        print(f'Página {i + 1} salva como {image_path}')

# Exemplo de uso
pdf_path = '/home/joselito/git/tcc/texto/figuras/'  # Substitua pelo caminho do seu arquivo PDF
output_folder = '/home/joselito/git/tcc/texto/figuras/'  # Substitua pelo diretório onde deseja salvar as imagens
pdf_to_png(pdf_path, output_folder)
