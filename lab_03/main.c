
#include <stdint.h>
#include <stm32f10x.h>

#define LED_PIN 12
#define BUTTON_A 1
#define BUTTON_C 2
#define DEBOUNCE_MS 20

volatile uint32_t ms_ticks = 0;
volatile uint32_t blink_phase = 0;
volatile uint32_t phase_inc = 2;
volatile uint32_t phase_limit = 1000;

int8_t frequency_level = 0;

// Прерывание каждую 1 мс
void SysTick_Handler(void) {
    ms_ticks++;

    blink_phase += phase_inc;

    if (blink_phase >= phase_limit) {
        blink_phase -= phase_limit;
        GPIOC->ODR ^= (1U << LED_PIN);
    }
}

// Изменение частоты в 2 раза
void change_frequency(int8_t direction) {
    int8_t next = frequency_level + direction;

    if (next < -6 || next > 6) {
        return;
    }

    __disable_irq();

    frequency_level = next;

    if (next >= 0) {
        phase_inc = 2U << next;
        phase_limit = 1000;
    } else {
        phase_inc = 2;
        phase_limit = 1000U << (-next);
    }

    blink_phase = 0;

    __enable_irq();
}

int __attribute__((noreturn)) main(void) {

    // Включаем тактирование GPIOA и GPIOC
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN |
                    RCC_APB2ENR_IOPAEN;

    // PC12 - выход Push-Pull, 10 MHz
    GPIOC->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12);
    GPIOC->CRH |= GPIO_CRH_MODE12_0;

    // Светодиод изначально выключен
    GPIOC->BSRR = (1U << LED_PIN);

    // PA1, PA2 - входы Pull-Up
    GPIOA->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1 |
                    GPIO_CRL_CNF2 | GPIO_CRL_MODE2);

    GPIOA->CRL |= GPIO_CRL_CNF1_1 | GPIO_CRL_CNF2_1;

    GPIOA->ODR |= (1U << BUTTON_A) |
                  (1U << BUTTON_C);

    // SysTick - 1 мс
    SystemCoreClockUpdate();

    if (SysTick_Config(SystemCoreClock / 1000U) != 0) {
        while (1) {
        }
    }

    uint8_t last_a = 0;
    uint8_t last_c = 0;
    uint8_t stable_a = 0;
    uint8_t stable_c = 0;

    uint32_t time_a = 0;
    uint32_t time_c = 0;

    while (1) {

        uint32_t now = ms_ticks;

        // Нажатие = 0
        uint8_t a = (GPIOA->IDR & (1U << BUTTON_A)) == 0;
        uint8_t c = (GPIOA->IDR & (1U << BUTTON_C)) == 0;

        // Кнопка A - частота x2
        if (a != last_a) {
            last_a = a;
            time_a = now;
        }

        if ((uint32_t)(now - time_a) >= DEBOUNCE_MS &&
            a != stable_a) {

            stable_a = a;

            if (stable_a) {
                change_frequency(1);
            }
        }

        // Кнопка C - частота /2
        if (c != last_c) {
            last_c = c;
            time_c = now;
        }

        if ((uint32_t)(now - time_c) >= DEBOUNCE_MS &&
            c != stable_c) {

            stable_c = c;

            if (stable_c) {
                change_frequency(-1);
            }
        }
    }
}
