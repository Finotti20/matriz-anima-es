#include <stdio.h>
#include "pico/stdlib.h"

#define ROWS 4
#define COLS 4
#define MATRIX_SIZE 5 

const uint8_t row_pins[ROWS] = {8, 7, 6, 5};
const uint8_t col_pins[COLS] = {4, 3, 2, 1};

const char key_map[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

void init_gpio()
{

    for (int i = 0; i < ROWS; i++)
    {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_OUT);
        gpio_put(row_pins[i], 1); // Linha inicialmente em HIGH
    }
    for (int i = 0; i < COLS; i++)
    {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_up(col_pins[i]); // Ativa pull-up nas colunas
    }
}

char scan_keypad()
{
    for (int row = 0; row < ROWS; row++)
    {
        gpio_put(row_pins[row], 0); // Configura a linha atual como LOW.
        for (int col = 0; col < COLS; col++)
        {
            if (gpio_get(col_pins[col]) == 0)
            {
                while (gpio_get(col_pins[col]) == 0)
                    ;
                gpio_put(row_pins[row], 1);
                return key_map[row][col];
            }
        }
        gpio_put(row_pins[row], 1); 
    }
    return '\0';
}

int ledMatrix[MATRIX_SIZE][MATRIX_SIZE] = {
    {0, 1, 2, 3, 4},
    {5, 6, 7, 8, 9},
    {10, 11, 12, 13, 14},
    {15, 16, 17, 18, 19},
    {20, 21, 22, 23, 24}
};

void initializeLedMatrix() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int pin = ledMatrix[i][j];
            gpio_set_dir(pin, GPIO_OUT); 
            gpio_put(pin, 0); 
        }
    }
}

void handle_keypress(char key) {
    switch (key) {
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
            break;
        case '9':
            animacao9();
            break;
        case 'C':
            leds_vermelhos();
            break;
        case '*':
            reboot();
            break;
        case '0':
            animacao0();
            break;
        case '#':
            white_leds();
            break;
        case 'D':
            leds_verdes();
            break;
        default:
            break;
    }
}

int main()
{
    stdio_init_all();
    init_gpio();
    initializeLedMatrix();

    while (true) {
        char key = scan_keypad();
        if (key != '\0') {
            handle_keypress(key);
        }
    }
}
