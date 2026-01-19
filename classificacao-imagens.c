/**
 * @file cnn_mnist.c
 * @brief Classificação MNIST via USB-Serial com modelo INT8
 */

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"

// Wrapper do professor (Você deve ter copiado esses arquivos para a pasta)
#include "tflm_wrapper.h"

// O modelo que geramos no passo 1
#include "mnist_cnn_int8_model.h" 

// --- Funções Auxiliares de Cálculo (Mantidas do exemplo do professor) ---

static int argmax_i8(const int8_t* v, int n) {
    int best = 0;
    int8_t bestv = v[0];
    for (int i = 1; i < n; i++) {
        if (v[i] > bestv) { bestv = v[i]; best = i; }
    }
    return best;
}

static int8_t quantize_f32_to_i8(float x, float scale, int zp) {
    long q = lroundf(x / scale) + zp;
    if (q < -128) q = -128;
    if (q >  127) q = 127;
    return (int8_t)q;
}

// --- MAIN ---

int main() {
    stdio_init_all();
    
    // --- MUDANÇA AQUI: TRAVA O CÓDIGO ATÉ ABRIR O SERIAL ---
    // Enquanto o cabo não estiver conectado E o monitor serial aberto no PC,
    // ele fica preso aqui. Isso garante que você não perde o primeiro print.
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    // -------------------------------------------------------

    printf("\n=== SISTEMA INICIADO COM SUCESSO ===\n");
    printf("Se você está lendo isso, o USB está funcionando.\n");

    // ... (o resto do código: tflm_init, etc) ...
    int rc = tflm_init(); 
    
    if (rc != 0) {
        printf("Falha na inicializacao TFLM: %d\n", rc);
        while(1) sleep_ms(100);
    }

    // 2. Ponteiros e Parâmetros de Quantização
    int in_bytes = 0;
    int8_t* in = tflm_input_ptr(&in_bytes);
    
    int out_bytes = 0;
    int8_t* out = tflm_output_ptr(&out_bytes);

    if (in_bytes < 784) {
        printf("Erro: Modelo espera menos que 784 bytes na entrada.\n");
        while(1) sleep_ms(100);
    }

    float in_scale = tflm_input_scale();
    int   in_zp    = tflm_input_zero_point();
    float out_scale = tflm_output_scale();
    int   out_zp    = tflm_output_zero_point();

    printf("Pronto. Envie os 784 bytes da imagem...\n");

    // 3. Loop Principal
    while (true) {
        // Leitura de 784 bytes (bloqueante)
        for (int i = 0; i < 784; i++) {
            int pixel_raw = getchar(); // Lê byte 0-255 da USB
            
            // Normaliza (0-255 -> 0.0-1.0) e depois Quantiza (-> int8)
            float pixel_norm = (float)pixel_raw / 255.0f;
            in[i] = quantize_f32_to_i8(pixel_norm, in_scale, in_zp);
        }

        // Inferência
        tflm_invoke();

        // Resultado
        int pred = argmax_i8(out, 10);
        
        // Calcula probabilidade aproximada para exibir
        float confianca = ((out[pred] - out_zp) * out_scale) * 100.0f;

        printf("PREDICAO: %d (Conf: %.1f%%)\n", pred, confianca);
    }

    return 0;
}