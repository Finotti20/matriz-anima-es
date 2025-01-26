#ifndef ANIMACAO3_H
#define ANIMACAO3_H

#include <stdint.h>
#include "ws2812.pio.h"
#include "pico/stdlib.h"

// Declaração das funções necessárias para LEDs
void npDraw(uint8_t vetorR[5][5], uint8_t vetorG[5][5], uint8_t vetorB[5][5]);
void npWrite();
void npClear();

// Função da animação 3
void animacao3() {
    const uint8_t totalFrames = 5; // Número de frames
    const uint32_t fps = 5;       // FPS ajustável
    const uint32_t frameDelay = 1000 / fps; // Intervalo entre frames em milissegundos

    // Frames para a animação do coração
    uint8_t frameR[5][5][5] = {
        { // Frame 1
            {  0,   255, 0, 255,   0 },
            {255,   0,   255,   0, 255 },
            {255,   0,   0,   0, 255 },
            {  0, 255,   0, 255,   0 },
            {  0,   0, 255,   0,   0 }
        },
        { // Frame 2
            {  0,   0,   0,   0,   0 },
            {  0, 255,   0, 255,   0 },
            {255,   0,   0,   0, 255 },
            {255,   0,   0,   0, 255 },
            {  0, 255, 255, 255,   0 }
        },
        { // Frame 3
            {  0,   0,   0,   0,   0 },
            {  0,   0, 255, 255,   0 },
            {255,   0,   0,   0, 255 },
            {  0, 255,   0, 255,   0 },
            {  0,   0, 255,   0,   0 }
        },
        { // Frame 4
            {  0,   0,   0,   0,   0 },
            {  0, 255,   0, 255,   0 },
            {255,   0,   0,   0, 255 },
            {255,   0,   0,   0, 255 },
            {  0, 255, 255, 255,   0 }
        },
        { // Frame 5
            {  0,   255, 0, 255,   0 },
            {255,   0,   255,   0, 255 },
            {255,   0,   0,   0, 255 },
            {  0, 255,   0, 255,   0 },
            {  0,   0, 255,   0,   0 }
        }
    };

    uint8_t frameG[5][5][5] = {0}; // Sem verde em todos os frames
    uint8_t frameB[5][5][5] = {0}; // Sem azul em todos os frames

    // Loop para exibir os frames da animação
    for (uint8_t f = 0; f < totalFrames; f++) {
        npDraw(frameR[f], frameG[f], frameB[f]); // Atualiza os LEDs com o frame atual
        npWrite();
        sleep_ms(frameDelay); // Aguarda o intervalo do FPS
        npClear();            // Limpa os LEDs antes do próximo frame
    }
}

#endif // ANIMACAO3_H
