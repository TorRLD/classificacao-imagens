import serial
import time
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt

# --- CONFIGURAÇÃO ---
SERIAL_PORT = 'COM13'  # Confirme se sua porta ainda é essa
BAUD_RATE = 115200

def main():
    # 1. Carrega dados de TESTE
    print("Carregando dataset MNIST...")
    (_, _), (x_test, y_test) = tf.keras.datasets.mnist.load_data()

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
        print(f"Conectado na {SERIAL_PORT}. Reiniciando placa...")
        ser.dtr = False # Truque para resetar alguns Arduinos/RP2040
        time.sleep(1)
        ser.dtr = True
        time.sleep(2) # Espera o boot
    except Exception as e:
        print(f"Erro ao abrir serial: {e}")
        return

    # Limpa buffers antigos
    ser.reset_input_buffer()

    while True:
        input("\n[ENTER] para enviar imagem e gerar gráfico...")
        
        # Sorteia imagem
        idx = np.random.randint(0, len(x_test))
        img = x_test[idx]
        label_real = y_test[idx]
        
        # Prepara envio
        img_flat = img.flatten().astype(np.uint8)
        
        print(f"--> Enviando número: {label_real}")
        
        # Envia e Força saída (flush)
        ser.write(img_flat.tobytes())
        ser.flush()

        # Aguarda resposta
        predicao_texto = "Sem Resposta"
        confianca_texto = ""
        
        start_time = time.time()
        while time.time() - start_time < 3:
            if ser.in_waiting:
                linha = ser.readline().decode('utf-8', errors='ignore').strip()
                if "PREDICAO" in linha:
                    print(f"<-- {linha}")
                    predicao_texto = linha
                    break
        
        # --- GERAÇÃO DO GRÁFICO PARA O RELATÓRIO ---
        plt.figure(figsize=(4, 4))
        plt.imshow(img, cmap='gray', vmin=0, vmax=255)
        
        # Monta um título bonito para o print
        titulo = f"Real: {label_real} | {predicao_texto}"
        plt.title(titulo, fontsize=10, color='blue')
        plt.axis('off') # Tira os eixos x/y para ficar limpo
        
        print("Exibindo imagem... (Feche a janela para continuar)")
        plt.show() # O código pausa aqui até você fechar a janela

if __name__ == "__main__":
    main()