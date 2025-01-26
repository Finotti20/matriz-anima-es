#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"

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
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Função para inicializar os pinos das linhas e colunas do teclado
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

void animacao1()
{

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR1[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 0, 255, 0},
      {0, 0, 0, 0, 0},
      {255, 0, 0, 0, 255},
      {0, 255, 255, 255, 0}};
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

  sleep_ms(1000);

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG2[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 0, 255, 0},
      {0, 0, 0, 0, 0},
      {255, 0, 0, 0, 255},
      {0, 255, 255, 255, 0}};
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

  sleep_ms(1000);

  npClear(); // Limpar Buffer de pixels

  // Arrays com o desenho
  uint8_t vetorR3[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorG3[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}};
  uint8_t vetorB3[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 255, 0, 255, 0},
      {0, 0, 0, 0, 0},
      {255, 0, 0, 0, 255},
      {0, 255, 255, 255, 0}};

  // Atribui as cores dos vetores RGB aos LEDs da matriz.
  npDraw(vetorR3, vetorG3, vetorB3);

  // Escreve os dados do buffer nos LEDs.
  npWrite();
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

void handle_keypress(char key)
{
  switch (key)
  {
  case '1':
    animacao1();
    break;
  case '2':
    // animacao2();
    break;
  case '3':
    // animacao3();
    break;
  case 'A':
    // aciona_leds();
    break;
  case '4':
    animacao4();
    break;
  case '5':
    // animacao5();
    break;
  case '6':
    // animacao6();
    break;
  case 'B':
    leds_azuis();
    break;
  case '7':
    // animacao7();
    break;
  case '8':
    // animacao8();
    break;
  case '9':
    // animacao9();
    break;
  case 'C':
    // leds_vermelhos();
    break;
  case '*':
    // reboot();
    break;
  case '0':
    // animacao0();
    break;
  case '#':
    // white_leds();
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
