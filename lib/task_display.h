#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include "hardware/i2c.h"
#include "ssd1306/ssd1306.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

bool cor = true;    // Define a cor do display (preto/branco)
ssd1306_t ssd;    // Inicializa a estrutura do display

void display_init(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); //Inicializa o display
    ssd1306_config(&ssd); //Configura o display
    ssd1306_send_data(&ssd); //Envia os dados para o display

    //O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
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

#endif