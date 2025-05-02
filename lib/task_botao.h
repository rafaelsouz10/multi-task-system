#ifndef TASK_BOTAO_H
#define TASK_BOTAO_H

#define BOTAO_A 5  // GPIO do botão A

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

#endif