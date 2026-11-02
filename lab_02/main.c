#include <stdint.h>
#include <stm32f10x.h>

void delay(uint32_t ticks) {
    for (volatile uint32_t i = 0; i < ticks; i++) {
        __NOP();
    }
}

int __attribute__((noreturn)) main(void) {

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // PC12 output push-pull, 10 MHz
    GPIOC->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12);
    GPIOC->CRH |= GPIO_CRH_MODE12_0;

	while (1) {
		GPIOC->ODR &= ~(1U << 12);
		delay(50000000);

		GPIOC->ODR |= (1U << 12);
		delay(50000000);
	}
}