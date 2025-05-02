#ifndef BUZZER_H
#define BUZZER_H

#include "hardware/structs/timer.h"
#include "hardware/irq.h"

// Pino do buzzer
#define BUZZER 21

// Variáveis de controle
volatile bool buzzer_estado = false;
alarm_id_t buzzer_alarm_id = -1;

//Inicializa o pino do buzzer como saída e garante que comece desligado.
void buzzer_init() {
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);
    buzzer_estado = false;
    buzzer_alarm_id = -1;
}

///Callback do alarme: alterna o buzzer a cada 2000us (simula ~500Hz)
int64_t buzzer_alarm_callback(alarm_id_t id, void *user_data) {
    buzzer_estado = !buzzer_estado;
    gpio_put(BUZZER, buzzer_estado);
    return 2000; // Reexecuta o callback a cada 2ms (500Hz)
}

//Inicia o efeito sonoro do buzzer (se não estiver tocando).
void buzzer_start_alarm() {
    if (buzzer_alarm_id < 0) {
        buzzer_alarm_id = add_alarm_in_us(2000, buzzer_alarm_callback, NULL, true);
    }
}

///Para o efeito sonoro do buzzer (se estiver tocando).
void buzzer_stop_alarm() {
    if (buzzer_alarm_id >= 0) {
        cancel_alarm(buzzer_alarm_id);
        buzzer_alarm_id = -1;
        gpio_put(BUZZER, 0);
        buzzer_estado = false;
    }
}

#endif
