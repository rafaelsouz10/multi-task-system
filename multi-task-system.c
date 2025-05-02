#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lib/buzzer.h"
#include "lib/display_ssd1306.h"
#include "lib/matriz_led.h"

// DEFINIÇÕES DOS PINOS
#define LED_GRENN 11 // GPIO do LED verde
#define LED_RED 13  // GPIO do LED vermelho
#define BOTAO_A 5  // GPIO do botão A

//TIPOS ENUMERADOS
typedef enum { MODO_NORMAL, MODO_NOTURNO } modo_t;   // Representa os modos do sistema
typedef enum { VERDE, AMARELO, VERMELHO } estado_t; // Representa os estados do semáforo

// FLAGS GLOBAIS 
volatile modo_t modo_atual = MODO_NORMAL;   // Flag que indica o modo atual do sistema
volatile estado_t estado_semaforo = VERDE; // Flag que indica o estado atual do semáforo

// TASK DO BOTÃO
void vBotaoTask() {
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A); // Usa pull-up interno (nível alto quando não pressionado)

    bool ultimo_estado = true;

    while (true) {
        bool estado_atual = gpio_get(BOTAO_A);

        // Detecta transição de HIGH para LOW (pressionado)
        if (!estado_atual && ultimo_estado) {
            // Alterna o modo entre NORMAL e NOTURNO
            modo_atual = (modo_atual == MODO_NORMAL) ? MODO_NOTURNO : MODO_NORMAL;

            printf("Modo alterado: %s\n", modo_atual == MODO_NORMAL ? "Normal" : "Noturno");
            
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce de 200ms
        }
        ultimo_estado = estado_atual;
        vTaskDelay(pdMS_TO_TICKS(50)); // Verifica o botão a cada 50ms
    }
}

// TASK DO SEMÁFORO
void vSemaforoTask() {
    gpio_init(LED_GRENN);
    gpio_init(LED_RED);
    gpio_set_dir(LED_GRENN, GPIO_OUT);
    gpio_set_dir(LED_RED, GPIO_OUT);

    while (true) {
        if (modo_atual == MODO_NORMAL) {
            // VERDE
            estado_semaforo = VERDE;
            gpio_put(LED_GRENN, 1); gpio_put(LED_RED, 0);
            for (int i = 0; i < 4000; i += 100) {        // Alternativa para fracionar o tempo de execução dos leds para
                if (modo_atual != MODO_NORMAL) break;   // não esperar o vtaskdelay (nesse caso 4000 ms) final acabar
                vTaskDelay(pdMS_TO_TICKS(100));        //  ao mudar para o modo noturno
            }
            if (modo_atual != MODO_NORMAL) continue; // Se não estiver no MODO NORMAL, altera para o modo norturno 

            //AMARELO
            estado_semaforo = AMARELO;
            gpio_put(LED_GRENN, 1); gpio_put(LED_RED, 1);
            for (int i = 0; i < 3000; i += 100) {
                if (modo_atual != MODO_NORMAL) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (modo_atual != MODO_NORMAL) continue;

            // VERMELHO
            estado_semaforo = VERMELHO;
            gpio_put(LED_GRENN, 0); gpio_put(LED_RED, 1);
            for (int i = 0; i < 4000; i += 100) {
                if (modo_atual != MODO_NORMAL) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }

        } else {
            // Modo Noturno
            estado_semaforo = AMARELO;

            gpio_put(LED_GRENN, 1); gpio_put(LED_RED, 1);
            for (int i = 0; i < 500; i += 100) {
                if (modo_atual != MODO_NOTURNO) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            gpio_put(LED_GRENN, 0); gpio_put(LED_RED, 0);
            for (int i = 0; i < 1500; i += 100) {
                if (modo_atual != MODO_NOTURNO) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}


// TASK DO BUZZER 
void vSomTask() {
    buzzer_init();

    while (true) {
        if (modo_atual == MODO_NORMAL) {
            switch (estado_semaforo) {
                case VERDE:
                    // 1 beep curto por segundo
                    buzzer_start_alarm();
                    vTaskDelay(pdMS_TO_TICKS(200));
                    buzzer_stop_alarm();
                    vTaskDelay(pdMS_TO_TICKS(800));
                break;
                case AMARELO:
                    // beeps rápidos (atenção)
                    for (int i = 0; i < 3; i++) {
                        buzzer_start_alarm();
                        vTaskDelay(pdMS_TO_TICKS(100));
                        buzzer_stop_alarm();
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                    vTaskDelay(pdMS_TO_TICKS(500));
                break;
                case VERMELHO:
                    // tom contínuo curto (pare)
                    buzzer_start_alarm();
                    vTaskDelay(pdMS_TO_TICKS(500));
                    buzzer_stop_alarm();
                    vTaskDelay(pdMS_TO_TICKS(1500));
                break;
            }
        } else {
            // Modo noturno: beep lento a cada 2s
            buzzer_start_alarm();
            vTaskDelay(pdMS_TO_TICKS(200));
            buzzer_stop_alarm();
            vTaskDelay(pdMS_TO_TICKS(1800));
        }
    }
}

// TASK DO DISPLAY
void vDisplayTask(){
    display_init();

    bool mostrar_figura = true;

    while (true) {
        ssd1306_fill(&ssd, 0); // limpa o display

        // Modo no topo
        ssd1306_draw_string(&ssd, (modo_atual == MODO_NORMAL) ? "MODO NORMAL" : "MODO NOTURNO", 0, 0);

        // Coordenadas
        const int x_quad = 4;      // O X do quadrado será sempre o mesmo
        const int x_texto = 20;   // Os textos permanecem estáticos
        const int tam_quad = 10; // Tamanho do quadrado 10x10

        // Y para cada texto correspondente a cor e também alterá a posição do quadrado
        const int y_verde    = 16;
        const int y_amarelo  = 30;
        const int y_vermelho = 44;

        // Desenha os nomes sempre
        ssd1306_draw_string(&ssd, "VERDE", x_texto, y_verde + 2);
        ssd1306_draw_string(&ssd, "AMARELO", x_texto, y_amarelo + 2);
        ssd1306_draw_string(&ssd, "VERMELHO", x_texto, y_vermelho + 2);

        // Desenha apenas ao lado da cor ativa do semáforo
        int y_quad = 0;

        if (estado_semaforo == VERDE) y_quad = y_verde;
        else if (estado_semaforo == AMARELO) y_quad = y_amarelo;
        else if (estado_semaforo == VERMELHO) y_quad = y_vermelho;

        // mostrar_figura varia entre true e false para o quadrado ficar piscando a cada 500 ms
        if (mostrar_figura)  ssd1306_rect(&ssd, y_quad, x_quad, tam_quad, tam_quad, 1, 1);

        ssd1306_send_data(&ssd);

        mostrar_figura = !mostrar_figura; // Atualiza o estado para a figura do quadrado ficar piscando
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// TASK DA MATRIZ RGB
void vMatrizTask() {
    npInit(LED_PIN); // Inicializa a matriz WS2812
    bool mostrar_figura = true;

    while (1) {
        npClear(); // Apaga tudo antes de cada ciclo

        if (mostrar_figura) {
            switch (estado_semaforo) {
                case VERDE:
                    npSetLED(getIndex(1, 1), 0, 255, 0); 
                    npSetLED(getIndex(2, 1), 0, 255, 0);
                    npSetLED(getIndex(3, 1), 0, 255, 0);
                break;
                case AMARELO:
                    npSetLED(getIndex(1, 2), 255, 255, 0); 
                    npSetLED(getIndex(2, 2), 255, 255, 0);
                    npSetLED(getIndex(3, 2), 255, 255, 0);
                break;
                case VERMELHO:
                    npSetLED(getIndex(1, 3), 255, 0, 0);
                    npSetLED(getIndex(2, 3), 255, 0, 0);
                    npSetLED(getIndex(3, 3), 255, 0, 0);
                break;
            }
        }
        npWrite(); // Envia os dados para a matriz
        mostrar_figura = !mostrar_figura; // alterna o estado de piscar
        vTaskDelay(pdMS_TO_TICKS(500)); // Pisca a cada 500ms
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events){
    reset_usb_boot(0, 0);
}

int main() {
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    // Criação das tasks
    xTaskCreate(vBotaoTask, "Botao", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vSemaforoTask, "Semaforo", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(vSomTask, "Som", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(vMatrizTask, "Matriz", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler(); // Inicia o escalonador do FreeRTOS
}
