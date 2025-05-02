#ifndef TASK_SEMAFORO_H
#define TASK_SEMAFORO_H

// DEFINIÇÕES DOS PINOS
#define LED_GREEN 11 // GPIO do LED verde
#define LED_RED 13  // GPIO do LED vermelho

//TIPOS ENUMERADOS
typedef enum { MODO_NORMAL, MODO_NOTURNO } modo_t;   // Representa os modos do sistema
typedef enum { VERDE, AMARELO, VERMELHO } estado_t; // Representa os estados do semáforo

// FLAGS GLOBAIS 
volatile modo_t modo_atual = MODO_NORMAL;   // Flag que indica o modo atual do sistema
volatile estado_t estado_semaforo = VERDE; // Flag que indica o estado atual do semáforo

// TASK DO SEMÁFORO
void vSemaforoTask() {
    gpio_init(LED_GREEN);
    gpio_init(LED_RED);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_RED, GPIO_OUT);

    while (true) {
        if (modo_atual == MODO_NORMAL) {
            // VERDE
            estado_semaforo = VERDE;
            gpio_put(LED_GREEN, 1); gpio_put(LED_RED, 0);
            for (int i = 0; i < 4000; i += 100) {        // Alternativa para fracionar o tempo de execução dos leds para
                if (modo_atual != MODO_NORMAL) break;   // não esperar o vtaskdelay (nesse caso 4000 ms) final acabar
                vTaskDelay(pdMS_TO_TICKS(100));        //  ao mudar para o modo noturno
            }
            if (modo_atual != MODO_NORMAL) continue; // Se não estiver no MODO NORMAL, altera para o modo norturno 

            //AMARELO
            estado_semaforo = AMARELO;
            gpio_put(LED_GREEN, 1); gpio_put(LED_RED, 1);
            for (int i = 0; i < 3000; i += 100) {
                if (modo_atual != MODO_NORMAL) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (modo_atual != MODO_NORMAL) continue;

            // VERMELHO
            estado_semaforo = VERMELHO;
            gpio_put(LED_GREEN, 0); gpio_put(LED_RED, 1);
            for (int i = 0; i < 4000; i += 100) {
                if (modo_atual != MODO_NORMAL) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }

        } else {
            // Modo Noturno
            estado_semaforo = AMARELO;

            gpio_put(LED_GREEN, 1); gpio_put(LED_RED, 1);
            for (int i = 0; i < 500; i += 100) {
                if (modo_atual != MODO_NOTURNO) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            gpio_put(LED_GREEN, 0); gpio_put(LED_RED, 0);
            for (int i = 0; i < 1500; i += 100) {
                if (modo_atual != MODO_NOTURNO) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}

#endif