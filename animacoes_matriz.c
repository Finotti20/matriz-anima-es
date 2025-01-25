#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

// ---------------- TECLADO - INICIO ----------------

// Definição do tamanho do teclado matricial (4x4)
#define LINHAS 4
#define COLUNAS 4

// Definição dos pinos GPIO das linhas e colunas do teclado matricial
#define L1 9
#define L2 8
#define L3 7
#define L4 6
#define C1 5
#define C2 4
#define C3 3
#define C4 2

// Arrays com os pinos correspondentes às linhas e colunas
const uint8_t pinos_linhas[LINHAS] = {L1, L2, L3, L4};
const uint8_t pinos_colunas[COLUNAS] = {C1, C2, C3, C4};

// Mapeamento das teclas do teclado matricial
const char teclas[LINHAS][COLUNAS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// Função para inicializar os pinos das linhas e colunas do teclado
void iniciar_keypad() {
    for (uint8_t i = 0; i < LINHAS; i++) {
        gpio_init(pinos_linhas[i]);
        gpio_set_dir(pinos_linhas[i], GPIO_OUT);
        gpio_put(pinos_linhas[i], 0);
    }
    for (uint8_t i = 0; i < COLUNAS; i++) {
        gpio_init(pinos_colunas[i]);
        gpio_set_dir(pinos_colunas[i], GPIO_IN);
        gpio_pull_down(pinos_colunas[i]);
    }
}

// Função para ler a tecla pressionada no teclado matricial
char ler_keypad() {
    for (uint8_t linha = 0; linha < LINHAS; linha++) {
        gpio_put(pinos_linhas[linha], 1);
        for (uint8_t coluna = 0; coluna < COLUNAS; coluna++) {
            if (gpio_get(pinos_colunas[coluna])) {
                gpio_put(pinos_linhas[linha], 0);
                return teclas[linha][coluna];
            }
        }
        gpio_put(pinos_linhas[linha], 0);
    }
    return '\0';
}

// ---------------- TECLADO - FIM ----------------

// ---------------- WS2812 - INICIO ----------------

// Definição do número de LEDs e pino
#define LED_COUNT 25
#define LED_PIN 22

// Estrutura de pixels RGB
typedef struct {
    uint8_t G, R, B;
} npLED_t;

// Buffer de LEDs
npLED_t leds[LED_COUNT];

// Variáveis de controle da máquina PIO
PIO np_pio;
uint sm;

// Inicialização da máquina PIO
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }
    ws2812_program_init(np_pio, sm, offset, pin, 800000.f);
    for (uint i = 0; i < LED_COUNT; i++) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Define a cor de um LED
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Limpa todos os LEDs
void npClear() {
    for (uint i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 0, 0, 0);
    }
}

// Escreve os dados no buffer de LEDs
void npWrite() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

// ---------------- WS2812 - FIM ----------------

// ---------------- Animações - INICIO ----------------

// Animação 3 - Coração com 5 frames
void animacao3() {
    uint8_t vetorFrames[5][5][5] = {
        {
            {0, 255, 0, 255, 0},
            {255, 0, 255, 0, 255},
            {255, 0, 0, 0, 255},
            {0, 255, 0, 255, 0},
            {0, 0, 255, 0, 0}
        },
        {
            {0, 0, 0, 0, 0},
            {0, 255, 0, 255, 0},
            {0, 0, 255, 0, 0},
            {0, 255, 0, 255, 0},
            {0, 0, 0, 0, 0}
        },
        {
            {0, 0, 0, 0, 0},
            {255, 0, 255, 0, 255},
            {0, 255, 0, 255, 0},
            {255, 0, 255, 0, 255},
            {0, 0, 0, 0, 0}
        },
        {
            {0, 255, 0, 255, 0},
            {255, 0, 255, 0, 255},
            {255, 0, 0, 0, 255},
            {0, 255, 0, 255, 0},
            {0, 0, 255, 0, 0}
        },
        {
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0}
        }
    };
    for (int frame = 0; frame < 5; frame++) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                npSetLED((4 - i) * 5 + j, vetorFrames[frame][i][j], 0, 0);
            }
        }
        npWrite();
        sleep_ms(200); // Controle de FPS
    }
}

// ---------------- Animações - FIM ----------------

// ---------------- Função de handle_keypress - INICIO ----------------

void handle_keypress(char key) {
    switch (key) {
        case '3':
            animacao3();
            break;
        default:
            npClear();
            npWrite();
            break;
    }
}

// ---------------- Função de handle_keypress - FIM ----------------

// Função principal
int main() {
    stdio_init_all();
    iniciar_keypad();
    npInit(LED_PIN);
    npClear();

    while (true) {
        sleep_ms(20);
        char tecla = ler_keypad();
        if (tecla != '\0') {
            handle_keypress(tecla);
        }
    }
    return 0;
}
