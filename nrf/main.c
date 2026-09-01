#include <stdint.h>
#include "weights.h"

#define NRF_TASKS_START    (*(volatile uint32_t*) 0x4000C000)
#define NRF_TASKS_STOP     (*(volatile uint32_t*) 0x4000C004)
#define NRF_EVENTS_DATARDY (*(volatile uint32_t*) 0x4000C100)
#define NRF_TEMP_TEMP      (*(volatile int32_t*)  0x4000C508)

#define NRF_P0_IN          (*(volatile uint32_t*) 0x50000510)
#define NRF_P0_OUTSET      (*(volatile uint32_t*) 0x50000508)
#define NRF_P0_OUTCLR      (*(volatile uint32_t*) 0x5000050C)
#define NRF_P0_DIRSET      (*(volatile uint32_t*) 0x50000718)
#define NRF_P0_PIN_CNF(n)  (*(volatile uint32_t*)(0x50000700 + 4*(n)))

#define LED_1_PIN    13
#define LED_2_PIN    14
#define BUTTON_1_PIN 11

void gpio_init(void) {
    // Configure LED pins (13, 14) as GPIO outputs
    // PIN_CNF: bit 0 = DIR (1=Output), bits 2-4 = INPUT (0=Connect)
    NRF_P0_PIN_CNF(LED_1_PIN) = (1UL << 0);  // PIN_CNF13: Output
    NRF_P0_PIN_CNF(LED_2_PIN) = (1UL << 0);  // PIN_CNF14: Output
    
    // Configure all button pins (11, 12, 24, 25) as GPIO inputs with pull-up
    // PIN_CNF: bit 0 = DIR (0=Input), bits 2-4 = INPUT (0=Connect), bits 8-10 = PULL (3=Pullup)
    NRF_P0_PIN_CNF(11) = (0UL << 0) | (0UL << 2) | (3UL << 8);   // Button 1
    NRF_P0_PIN_CNF(12) = (0UL << 0) | (0UL << 2) | (3UL << 8);   // Button 2
    NRF_P0_PIN_CNF(24) = (0UL << 0) | (0UL << 2) | (3UL << 8);   // Button 3
    NRF_P0_PIN_CNF(25) = (0UL << 0) | (0UL << 2) | (3UL << 8);   // Button 4
    
    // Set LED pins as outputs in DIRSET
    NRF_P0_DIRSET = (1UL << LED_1_PIN) | (1UL << LED_2_PIN);
    
    // Set LEDs OFF (high, since active-low)
    NRF_P0_OUTSET = (1UL << LED_1_PIN) | (1UL << LED_2_PIN);
}

float read_temp(void) {
    NRF_EVENTS_DATARDY = 0;
    (void)NRF_EVENTS_DATARDY; //read-back flush for Cortex-M4 write buffer

    NRF_TASKS_START = 1;
    while (NRF_EVENTS_DATARDY == 0);
    
    NRF_EVENTS_DATARDY = 0;
    (void)NRF_EVENTS_DATARDY;
    
    int32_t raw_temp = NRF_TEMP_TEMP;
    NRF_TASKS_STOP = 1;
    
    return ((float)raw_temp / 4.0f);
}

static inline float relu(float x) {
    return (x > 0.0f) ? x : 0.0f;
}

float run_inference(float temp_c, float button_hold_sec) {
    float input[2] = {temp_c, button_hold_sec};
    float hidden[2];
    float output[1];

    for (int h = 0; h < 2; h++) {
        float sum = B1[h];
        for (int i = 0; i < 2; i++) {
            sum += input[i] * W1[i][h];
        }
        hidden[h] = relu(sum);
    }

    for (int o = 0; o < 1; o++) {
        float sum = B2[o];
        for (int h = 0; h < 2; h++) {
            sum += hidden[h] * W2[h][o];
        }
        output[o] = sum;
    }

    return output[0];
}

int main(void) {
    gpio_init();

    // LED 1 starts OFF
    NRF_P0_OUTSET = (1UL << LED_1_PIN);

    while (1) {
        // Read inputs
        float temp = read_temp();
        int button_pressed = (NRF_P0_IN & (1UL << BUTTON_1_PIN)) == 0 ? 1 : 0;
        
        // Run model
        float score = run_inference(temp, button_pressed ? 2.0f : 0.0f);

        // If condition met: briefly turn LED 1 ON
        if (score > 0.0f) {
            NRF_P0_OUTCLR = (1UL << LED_1_PIN);  // LED 1 ON
            for (volatile int i = 0; i < 5000000; i++);  // Hold for ~78ms
            NRF_P0_OUTSET = (1UL << LED_1_PIN);  // LED 1 OFF
        }

        // Wait before next cycle
        for (volatile int i = 0; i < 2000000; i++);
    }

    return 0;
}