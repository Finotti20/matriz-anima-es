#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "ws2812.pio.h"
#include <stdlib.h>
#include <unistd.h>
// ---------------- TECLADO - INICIO ----------------

// Definição do tamanho do teclado matricial (4x4)
#define LINHAS 4
#define COLUNAS 4
#define NUM_LINHAS 5
#define NUM_COLUNAS 5

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
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Função para inicializar os pinos das linhas e colunas do teclado

void NpDraw(int linha, int coluna, int r, int g, int b) {
    // Lógica para controlar o LED da matriz (linha, coluna) com a cor (r, g, b)
    // Esta parte dependeria da biblioteca ou hardware que você estiver utilizando.
    printf("LED[%d][%d] aceso com cor RGB(%d, %d, %d)\n", linha, coluna, r, g, b);
}

// Função para atualizar a matriz de LEDs
void NpWrite() {
    // Atualiza a matriz de LEDs (este código seria específico para o seu hardware)
    printf("Atualizando a matriz de LEDs...\n");
}

void iniciar_keypad()
{
  uint8_t i;
  // Configura os pinos das linhas como saída e define o estado inicial como 0
  for (i = 0; i < LINHAS; i++)
  {
    gpio_init(pinos_linhas[i]);
    gpio_set_dir(pinos_linhas[i], GPIO_OUT);
    gpio_put(pinos_linhas[i], 0);
  }
  // Configura os pinos das colunas como entrada e ativa o pull-down interno
  for (i = 0; i < COLUNAS; i++)
  {
    gpio_init(pinos_colunas[i]);
    gpio_set_dir(pinos_colunas[i], GPIO_IN);
    gpio_pull_down(pinos_colunas[i]);
  }
}

void acende_led(int linha, int coluna) {
    // Aqui você deve implementar o código que acende o LED (linha, coluna)
    // Este é um exemplo genérico
    NpDraw(linha, coluna, 255, 255, 255);  // LED aceso com cor branca (RGB)
    NpWrite();  // Atualiza a matriz de LEDs
}

// Função para apagar um LED na posição (linha, coluna)
void apaga_led(int linha, int coluna) {
    // Aqui você deve implementar o código que apaga o LED (linha, coluna)
    // Este é um exemplo genérico
    NpDraw(linha, coluna, 0, 0, 0);  // LED apagado (preto)
    NpWrite();  // Atualiza a matriz de LEDs
}

// Função para ler a tecla pressionada no teclado matricial
char ler_keypad()
{
  uint8_t linha;
  uint8_t coluna;
  // Itera pelas linhas e verifica se há sinal em alguma coluna
  for (linha = 0; linha < LINHAS; linha++)
  {
    gpio_put(pinos_linhas[linha], 1); // Ativa a linha atual
    for (coluna = 0; coluna < COLUNAS; coluna++)
    {
      if (gpio_get(pinos_colunas[coluna]))
      {                                   // Verifica se há sinal na coluna
        gpio_put(pinos_linhas[linha], 0); // Desativa a linha atual
        return teclas[linha][coluna];     // Retorna a tecla pressionada
      }
    }
    gpio_put(pinos_linhas[linha], 0); // Desativa a linha atual
  }
  return '\0'; // Retorna '\0' se nenhuma tecla foi pressionada
}

// ---------------- TECLADO - FIM ----------------

// ---------------- WS2812 - INICIO ----------------

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 22

// Definição de pixel GRB
struct pixel_t
{
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

// Inicializa a máquina PIO para controle da matriz de LEDs.
void npInit(uint pin)
{

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2812_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0)
  {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  ws2812_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

// Atribui uma cor RGB a um LED.
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

// Limpa o buffer de pixels.
void npClear()
{
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

// Escreve os dados do buffer nos LEDs.
void npWrite()
{
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

// ---------------- WS2812 - FIM ----------------

// ---------------- Desenhar - INICIO ----------------

void npDraw(uint8_t vetorR[5][5], uint8_t vetorG[5][5], uint8_t vetorB[5][5])
{
  int i, j, idx;
  for (i = 0; i < 5; i++)
  {
    idx = (4 - i) * 5;
    for (j = 4; j >= 0; j--)
    {
      npSetLED(idx + (4 - j), vetorR[i][j], vetorG[i][j], vetorB[i][j]);
    }
  }
}

// ---------------- Desenhar - FIM ----------------

// Início da animação tecla 1

void animacao1()
{

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 0, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB3[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 0, 0, 0, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255}};
  uint8_t vetorB4[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB5[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255}};
  npDraw(vetorR1, vetorG1, vetorB1); // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npWrite();                         // Escreve os dados do buffer nos LEDs.
  sleep_ms(256);                     // Parar um pouco para formar a animação
  npDraw(vetorR1, vetorG1, vetorB2);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG1, vetorB3);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG1, vetorB2);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG1, vetorB1);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG1, vetorB4);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG1, vetorB5);
  npWrite();
  sleep_ms(256);
  npClear(); // Limpar buffer (Atribuir 0 a todos os leds)
  npWrite();
}

// Início da animação tecla 2
void animacao2()
{
  printf("Animacao 2 acionada!\n"); // Adiciona a mensagem
  npClear();                        // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB1[5][5] = {
      {255, 255, 255, 255, 255},
      {0, 0, 0, 0, 255}, // INVERTIDA
      {255, 0, 255, 255, 255},
      {255, 0, 0, 0, 255}, // INVERTIDA
      {255, 255, 255, 255, 255}};
  uint8_t vetorB2[5][5] = {
      {255, 255, 255, 255, 0},
      {0, 0, 0, 0, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 0, 0, 0},
      {255, 255, 255, 255, 0}};

  uint8_t vetorB3[5][5] = {
      {255, 255, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {255, 255, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {255, 255, 255, 0, 0}};
  uint8_t vetorR3[5][5] = {
      {0, 0, 0, 0, 255},
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 255},
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 255}};

  uint8_t vetorB4[5][5] = {
      {255, 255, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {255, 255, 0, 0, 0},
      {0, 0, 0, 255, 0},
      {255, 255, 0, 0, 0}};
  uint8_t vetorR4[5][5] = {
      {0, 0, 0, 255, 255},
      {0, 255, 0, 0, 0},
      {0, 0, 0, 255, 255},
      {0, 255, 0, 0, 0},
      {0, 0, 0, 255, 0}};
  uint8_t vetorR5[5][5] = {
      {0, 0, 255, 255, 255},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 255, 255},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0}};
  uint8_t vetorR6[5][5] = {
      {0, 255, 255, 255, 0},
      {255, 0, 0, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 0, 0, 255, 0},
      {0, 255, 0, 0, 0}};
  uint8_t vetorR7[5][5] = {
      {255, 255, 255, 0, 0},
      {0, 255, 0, 0, 255},
      {255, 255, 255, 0, 0},
      {0, 0, 0, 0, 255},
      {255, 0, 0, 0, 0}};
  uint8_t vetorR8[5][5] = {
      {255, 255, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {255, 255, 0, 0, 0},
      {
          0,
          0,
          0,
          0,
      },
      {0, 0, 0, 0, 0}};
  uint8_t vetorG8[5][5] = {
      {0, 0, 0, 0, 255},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 255},
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 255}};
  uint8_t vetorG9[5][5] = {
      {0, 0, 0, 255, 255},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 255, 255},
      {0, 255, 0, 0, 0},
      {0, 0, 0, 255, 255}};
  uint8_t vetorG10[5][5] = {
      {0, 0, 255, 255, 255},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 255, 255},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 255, 255}};
  uint8_t vetorG11[5][5] = {
      {0, 255, 255, 255, 255},
      {255, 0, 0, 0, 0},
      {0, 255, 255, 255, 255},
      {0, 0, 0, 255, 0},
      {0, 255, 255, 255, 255}};
  uint8_t vetorG12[5][5] = {
      {255, 255, 255, 255, 0},
      {0, 255, 0, 0, 0},
      {255, 255, 255, 255, 0},
      {0, 0, 0, 0, 255},
      {255, 255, 255, 255, 0}};
  uint8_t vetorG13[5][5] = {
      {255, 255, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {255, 255, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {255, 255, 255, 0, 0}};

  uint8_t vetorG14[5][5] = {
      {255, 255, 0, 0, 0},
      {0, 0, 0, 255, 0},
      {255, 255, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {255, 255, 0, 0, 0}};
  uint8_t vetorG15[5][5] = {
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 255},
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {255, 0, 0, 0, 0}};
  uint8_t vetorG16[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorR9[5][5] = {
      {255, 0, 0, 0, 0},
      {0, 0, 0, 255, 0},
      {255, 0, 0, 0, 0},
      {
          0,
          0,
          0,
          0,
      },
      {0, 0, 0, 0, 0}};
  uint8_t vetorR10[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 255},
      {0, 0, 0, 0, 0},
      {
          0,
          0,
          0,
          0,
      },
      {0, 0, 0, 0, 0}};
  uint8_t vetorR11[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {
          0,
          0,
          0,
          0,
      },
      {0, 0, 0, 0, 0}};
  uint8_t vetorB5[5][5] = {
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 255},
      {255, 0, 0, 0, 0}};
  uint8_t vetorB6[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Arrays com o desenho
  uint8_t vetorR17[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG17[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB17[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG18[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 0, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorR18[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 0, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB19[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 0, 0, 0, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255}};
  uint8_t vetorR20[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 255, 255, 255, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG21[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255}};
  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR1, vetorG1, vetorB2);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR3, vetorG1, vetorB3);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR4, vetorG1, vetorB4);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR5, vetorG1, vetorB5);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR6, vetorG1, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR7, vetorG1, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR8, vetorG8, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR9, vetorG9, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR10, vetorG10, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR11, vetorG11, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR11, vetorG12, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR11, vetorG13, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR11, vetorG14, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR11, vetorG15, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(600); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR11, vetorG16, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(200); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR17, vetorG17, vetorB17);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(128); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR18, vetorG18, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(128); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR1, vetorG1, vetorB19);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(128); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR20, vetorG1, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(128); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
  npClear();     // Limpar Buffer de pixels
  npDraw(vetorR1, vetorG21, vetorB6);
  // Escreve os dados do buffer nos LEDs.
  npWrite();
  sleep_ms(128); // Pausa a execução do programa por 1000 milissegundos (1 segundo)
}


// Início da animação tecla 3 

void animacao3()
{
  printf("Animacao 3 acionada!\n"); // Adiciona a mensagem

  npClear(); // Limpar Buffer de pixels

   // Frame 1
    uint8_t frame1R[5][5] = {
        {0, 255, 0, 255, 0},
        {255, 0, 255, 0, 255},
        {255, 0, 0, 0, 255},
        {0, 255, 0, 255, 0},
        {0, 0, 255, 0, 0}};

    // Frame 2
    uint8_t frame2R[5][5] = {
        {0, 0, 255, 0, 0},
        {0, 255, 0, 255, 0},
        {255, 0, 0, 0, 255},
        {0, 255, 0, 255, 0},
        {0, 0, 255, 0, 0}};

    // Frame 3
    uint8_t frame3R[5][5] = {
        {255, 0, 0, 0, 255},
        {0, 255, 0, 255, 0},
        {0, 0, 255, 0, 0},
        {0, 255, 0, 255, 0},
        {255, 0, 0, 0, 255}};

    // Frame 4
    uint8_t frame4R[5][5] = {
        {0, 255, 0, 255, 0},
        {255, 0, 255, 0, 255},
        {0, 255, 0, 255, 0},
        {0, 0, 255, 0, 0},
        {0, 0, 0, 0, 0}};

    // Frame 5
    uint8_t frame5R[5][5] = {
        {0, 0, 0, 0, 0},
        {0, 255, 0, 255, 0},
        {255, 0, 0, 0, 255},
        {0, 255, 0, 255, 0},
        {0, 0, 255, 0, 0}};

    // Vetores G e B (mantêm apagados, apenas cor vermelha usada)
    uint8_t frameG[5][5] = {0};
    uint8_t frameB[5][5] = {0};

    // Array de frames
    uint8_t *frames[5] = {frame1R[0], frame2R[0], frame3R[0], frame4R[0], frame5R[0]};

    // Executa os frames
    for (int i = 0; i < 5; i++)
    {
        // Desenha o frame atual
        npDraw((uint8_t(*)[5])frames[i], frameG, frameB);

        // Escreve no LED
        npWrite();

        // Pausa entre frames
        sleep_ms(1000);
    }

    // Limpa os LEDs ao final
    npClear();
}


// Início da animação tecla 4

void animacao4()
{

  printf("Animacao 4 acionada!\n"); // Adiciona a mensagem

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG1[5][5] = {
      {108, 108, 108, 108, 0},
      {108, 0, 0, 0, 108},
      {108, 108, 108, 108, 0},
      {108, 0, 0, 0, 108},
      {108, 108, 108, 108, 0}};
  uint8_t vetorB1[5][5] = {
      {255, 255, 255, 255, 0},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 0},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução do programa por 1000 milissegundos (1 segundo)

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR2[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 0, 0, 0, 255}};
  uint8_t vetorG2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR2, vetorG2, vetorB2);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução do programa por 1000 milissegundos (1 segundo)

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR3[5][5] = {
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1}};
  uint8_t vetorG3[5][5] = {
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1}};
  uint8_t vetorB3[5][5] = {
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR3, vetorG3, vetorB3);

  // Escreve os dados do buffer nos LEDs.
  npWrite(0, 29, 141);

  sleep_ms(1000); // Pausa a execução do programa por 1000 milissegundos (1 segundo)

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR4[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG4[5][5] = {
      {0, 0, 108, 0, 0},
      {0, 0, 108, 0, 0},
      {0, 0, 108, 0, 0},
      {0, 0, 108, 0, 0},
      {0, 0, 108, 0, 0}};
  uint8_t vetorB4[5][5] = {
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR4, vetorG4, vetorB4);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução do programa por 1000 milissegundos (1 segundo)

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR5[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 0, 0, 0, 255}};
  uint8_t vetorG5[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB5[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR5, vetorG5, vetorB5);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução do programa por 1000 milissegundos (1 segundo)

  npClear(); // Limpar Buffer de pixels
}

void animacao5()
{

  printf("Animacao 5 acionada!\n");

  // Vetores para cada letra da palavra "CEPEDI"
  uint8_t vetorR[6][5][5] = {
      // Letra C
      {
          {0, 255, 255, 255, 0},
          {255, 0, 0, 0, 255},
          {255, 0, 0, 0, 0},
          {255, 0, 0, 0, 255},
          {0, 255, 255, 255, 0}},
      // Letra E
      {
          {255, 255, 255, 255, 255},
          {0, 0, 0, 0, 255},
          {255, 255, 255, 255, 0},
          {0, 0, 0, 0, 255},
          {255, 255, 255, 255, 255}},
      // Letra P
      {
          {255, 255, 255, 255, 255},
          {255, 0, 0, 0, 255},
          {255, 255, 255, 255, 255},
          {0, 0, 0, 0, 255},
          {255, 0, 0, 0, 0}},
      // Letra E (repetida)
      {
          {255, 255, 255, 255, 255},
          {0, 0, 0, 0, 255},
          {255, 255, 255, 255, 0},
          {0, 0, 0, 0, 255},
          {255, 255, 255, 255, 255}},
      // Letra D
      {
          {255, 255, 255, 255, 0},
          {255, 0, 0, 0, 255},
          {255, 0, 0, 0, 255},
          {255, 0, 0, 0, 255},
          {255, 255, 255, 255, 0}},
      // Letra I
      {
          {0, 255, 255, 255, 0},
          {0, 0, 255, 0, 0},
          {0, 0, 255, 0, 0},
          {0, 0, 255, 0, 0},
          {0, 255, 255, 255, 0}}};

  // Vetores de cor G e B inicializados como zeros
  uint8_t vetorG[5][5] = {{0}};
  uint8_t vetorB[5][5] = {{0}};

  for (int letra = 0; letra < 6; letra++)
  {
    // Exibe cada letra da palavra "CEPEDI" na matriz
    npDraw(vetorR[letra], vetorG, vetorB);
    npWrite();
    sleep_ms(1000); // Mostra cada letra por 1 segundo

    // Limpa a matriz antes de mostrar a próxima letra
    npClear();
    npWrite();
    sleep_ms(200); // Breve pausa entre as letras
  }
}

// animação tecla 6
void animacao6()
{

  npClear(); // impa buffer de pixels

  // Arrays com os desenhos
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG1[5][5] = {
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB1[5][5] = {
      {255, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB2[5][5] = {
      {0, 255, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB3[5][5] = {
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG2[5][5] = {
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB4[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG3[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB5[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG4[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB6[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0}};
  uint8_t vetorG5[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 255, 0, 0},
      {0, 0, 255, 0, 0}};
  // escrita e atuliazacao de desenhos
  npDraw(vetorR1, vetorG1, vetorB1); // atribui as cores dos vetores RGB aos LEDs da matriz.
  npWrite();                         // escreve os dados do buffer nos LEDs.
  sleep_ms(128);                     // tempo para formar a animação
  npDraw(vetorR1, vetorG1, vetorB2);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG2, vetorB3);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG3, vetorB4);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG4, vetorB5);
  npWrite();
  sleep_ms(128);
  npDraw(vetorR1, vetorG5, vetorB6);
  npWrite();
  sleep_ms(128);
  npClear(); // limpa buffer (apaga todos os leds)
  npWrite();
}

void animacao7()
{
  npClear(); // Limpar Buffer de pixels

  // Frames da animação
  uint8_t vetorR[5][5] = {0};
  uint8_t vetorG[5][5] = {0};
  uint8_t vetorB[5][5] = {0};

  // Loop para criar 5 frames com a "sombra" percorrendo da esquerda para a direita
  for (int coluna = 0; coluna < 5; coluna++)
  {
    // Determina a cor da sombra para este frame
    uint8_t r = 0, g = 0, b = 0;
    if (coluna % 3 == 0)
    {
      g = 255; // Verde
    }
    else if (coluna % 3 == 1)
    {
      b = 255; // Azul
    }
    else
    {
      r = 255; // Vermelho
    }

    // Limpar os vetores antes de cada frame
    for (int i = 0; i < 5; i++)
    {
      for (int j = 0; j < 5; j++)
      {
        vetorR[i][j] = 0;
        vetorG[i][j] = 0;
        vetorB[i][j] = 0;
      }
    }

    // Adiciona a "sombra" na coluna atual com a cor correspondente
    for (int linha = 0; linha < 5; linha++)
    {
      vetorR[linha][coluna] = r;
      vetorG[linha][coluna] = g;
      vetorB[linha][coluna] = b;
    }

    // Atualiza a matriz de LEDs com o frame atual
    npDraw(vetorR, vetorG, vetorB);
    npWrite();

    // Delay entre os frames
    sleep_ms(300);
  }

  // Adiciona o LED percorrendo as bordas para formar um quadrado
  uint8_t r = 255, g = 255, b = 255; // Branco para o LED que percorre as bordas
  for (int i = 0; i < 20; i++)
  {
    npClear();

    // Define a posição do LED na borda
    int linha = 0, coluna = 0;
    if (i < 5)
    {
      linha = 0;
      coluna = i; // Parte superior
    }
    else if (i < 10)
    {
      linha = i - 5;
      coluna = 4; // Lado direito
    }
    else if (i < 15)
    {
      linha = 4;
      coluna = 14 - i; // Parte inferior
    }
    else
    {
      linha = 19 - i;
      coluna = 0; // Lado esquerdo
    }

    // Atualiza o vetor com a posição do LED
    vetorR[linha][coluna] = r;
    vetorG[linha][coluna] = g;
    vetorB[linha][coluna] = b;

    // Desenha o LED na borda
    npDraw(vetorR, vetorG, vetorB);
    npWrite();
    sleep_ms(150);
  }

  // Limpar a matriz após a animação
  npClear();
  npWrite();
}

// Início da animação tecla 9

// Animação 8: LEDs acendem em linha e depois apagam, criando um movimento sequencial.
void animacao8() {
    int i, j;

    // Sequência de acendimento de LEDs da esquerda para a direita, linha por linha
    for (i = 0; i < NUM_LINHAS; i++) {
        for (j = 0; j < NUM_COLUNAS; j++) {
            acende_led(i, j);  // Acende o LED
            usleep(100000);  // Pausa de 0.1 segundos (100ms)
            apaga_led(i, j);  // Apaga o LED
        }
    }

    // Sequência de acendimento de LEDs da direita para a esquerda, linha por linha
    for (i = 0; i < NUM_LINHAS; i++) {
        for (j = NUM_COLUNAS - 1; j >= 0; j--) {
            acende_led(i, j);  // Acende o LED
            usleep(100000);  // Pausa de 0.1 segundos
            apaga_led(i, j);  // Apaga o LED
        }
    }

    // Sequência de acendimento de LEDs de cima para baixo
    for (j = 0; j < NUM_COLUNAS; j++) {
        for (i = 0; i < NUM_LINHAS; i++) {
            acende_led(i, j);  // Acende o LED
            usleep(100000);  // Pausa de 0.1 segundos
            apaga_led(i, j);  // Apaga o LED
        }
    }

    // Sequência de acendimento de LEDs de baixo para cima
    for (j = 0; j < NUM_COLUNAS; j++) {
        for (i = NUM_LINHAS - 1; i >= 0; i--) {
            acende_led(i, j);  // Acende o LED
            usleep(100000);  // Pausa de 0.1 segundos
            apaga_led(i, j);  // Apaga o LED
        }
    }
}

void animacao9()
{
  printf("Animacao 9 acionada!\n"); // Mensagem indicando a execução

  npClear(); // Limpar Buffer de pixels

  // Array com o desenho inicial (número 9 em RGB)
  uint8_t vetorR1[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255},
      {0, 0, 0, 0, 255},
      {255, 255, 255, 255, 255}};

  uint8_t vetorG1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  uint8_t vetorB1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução por 1 segundo

  npClear(); // Limpa o buffer novamente para próximo quadro

  // Segunda parte da animação para reforçar o "efeito"
  uint8_t vetorR2[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255},
      {0, 0, 0, 0, 255},
      {0, 0, 0, 0, 255}};

  uint8_t vetorG2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  uint8_t vetorB2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR2, vetorG2, vetorB2);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução por 1 segundo

  npClear(); // Limpa o buffer novamente para próxima etapa

  // Finaliza com o "brilho total" do número 9
  uint8_t vetorR3[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255},
      {0, 0, 0, 0, 255},
      {0, 0, 0, 0, 255}};

  uint8_t vetorG3[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  uint8_t vetorB3[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 0, 0, 0, 255},
      {255, 255, 255, 255, 255},
      {0, 0, 0, 0, 255},
      {0, 0, 0, 0, 255}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR3, vetorG3, vetorB3);

  // Escreve os dados do buffer nos LEDs.
  npWrite();

  sleep_ms(1000); // Pausa a execução por 1 segundo

  npClear(); // Limpa o buffer ao finalizar
}

void leds_azuis()
{
  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB1[5][5] = {
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve
  npWrite();
}

void leds_vermelhos()
{
  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {205, 205, 205, 205, 205},
      {205, 205, 205, 205, 205},
      {205, 205, 205, 205, 205},
      {205, 205, 205, 205, 205},
      {205, 205, 205, 205, 205}};
  uint8_t vetorG1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve
  npWrite();
}

void white_leds()
{
  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51}};
  uint8_t vetorG1[5][5] = {
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51}};
  uint8_t vetorB1[5][5] = {
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51},
      {51, 51, 51, 51, 51}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve
  npWrite();
}

void leds_verdes()
{
  printf("Leds verdes acionado!\n"); // Mensagem indicando a execução
  npClear();                         // Limpar Buffer de pixels

  uint8_t intensidade = (50 * 255) / 100; // Calculando a intensidade em 50%

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG1[5][5] = {
      {intensidade, intensidade, intensidade, intensidade, intensidade},
      {intensidade, intensidade, intensidade, intensidade, intensidade},
      {intensidade, intensidade, intensidade, intensidade, intensidade},
      {intensidade, intensidade, intensidade, intensidade, intensidade},
      {intensidade, intensidade, intensidade, intensidade, intensidade}};
  uint8_t vetorB1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR1, vetorG1, vetorB1);

  // Escreve
  npWrite();
}

void aciona_leds()
{
  npClear(); // Limpa o buffer de pixels, desligando todos os LEDs
  npWrite(); // Atualiza a matriz de LEDs para refletir a mudança
}

void handle_keypress(char key)
{
  switch (key)
  {
  case '1':
    animacao1();
    break;
  case '2':
    animacao2();
    break;
  case '3':
     animacao3();
    break;
  case 'A':
    aciona_leds();
    break;
  case '4':
    animacao4();
    break;
  case '5':
    animacao5();
    break;
  case '6':
    animacao6();
    break;
  case 'B':
    leds_azuis();
    break;
  case '7':
    animacao7();
    break;
  case '8':
    animacao8();
    usleep(500000);
    break;
  case '9':
    animacao9();
    break;
  case 'C':
    leds_vermelhos();
    break;
  case '*':
    printf("Entrando no modo BOOTSEL. Aguarde...\n");
    sleep_ms(500);        // Pequena pausa para garantir a saída do texto
    reset_usb_boot(0, 0); // Reinicia no modo BOOTSEL
    break;
  case '0':
    // animacao0();
    break;
  case '#':
    white_leds();
    break;
  case 'D':
    // leds_verdes();
    break;
  default:
    break;
  }
}

int main()
{
  char tecla;
  int i, j, idx;
  stdio_init_all();

  // Inicializa o teclado matricial
  iniciar_keypad();

  // Inicializa matriz de LEDs NeoPixel.
  npInit(LED_PIN);
  npClear();

  while (true)
  {
    sleep_ms(20);         // Aguarda 20 milissegundos para melhor funcionamento do simulador
    tecla = ler_keypad(); // Lê as teclas do keypad
    if (tecla != '\0')
    {
      printf("Tecla pressionada: %c\n", tecla);
      handle_keypress(tecla);
      sleep_ms(180);
    }
  }
}