#include <stdint.h>
#include <stm32f10x.h>

// SSD1306 pins
#define OLED_RES (1U << 2)  // PA2
#define OLED_DC  (1U << 3)  // PA3
#define OLED_CS  (1U << 4)  // PA4

// SPI1 pins:
// PA5 - SCK
// PA6 - MISO
// PA7 - MOSI

volatile uint8_t digit = 0;

// =====================================
// Digits 0-9
// =====================================

static const uint8_t digits[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};

// =====================================
// Delay
// =====================================

void Delay_Init(void)
{
    SystemCoreClockUpdate();
}

void Delay_ms(uint32_t ms)
{
    SysTick->LOAD = SystemCoreClock / 1000U - 1U;
    SysTick->VAL = 0;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    while (ms--) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) {
        }
    }

    SysTick->CTRL = 0;
}

// =====================================
// SPI1 initialization
// =====================================

void SPI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_AFIOEN |
                    RCC_APB2ENR_SPI1EN;

    GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2 |
                    GPIO_CRL_CNF3 | GPIO_CRL_MODE3 |
                    GPIO_CRL_CNF4 | GPIO_CRL_MODE4 |
                    GPIO_CRL_CNF5 | GPIO_CRL_MODE5 |
                    GPIO_CRL_CNF6 | GPIO_CRL_MODE6 |
                    GPIO_CRL_CNF7 | GPIO_CRL_MODE7);

    // PA2, PA3, PA4: Output Push-Pull
    GPIOA->CRL |= GPIO_CRL_MODE2_1 |
                  GPIO_CRL_MODE3_1 |
                  GPIO_CRL_MODE4_1;

    // PA5, PA7: Alternate Function Push-Pull
    GPIOA->CRL |= GPIO_CRL_MODE5_1 |
                  GPIO_CRL_CNF5_1 |
                  GPIO_CRL_MODE7_1 |
                  GPIO_CRL_CNF7_1;

    // PA6: Floating Input
    GPIOA->CRL |= GPIO_CRL_CNF6_0;

    GPIOA->BSRR = OLED_CS | OLED_RES;
    GPIOA->BRR = OLED_DC;

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    // Master, 8-bit, Mode 0, Software NSS
    // Baud rate = PCLK2 / 16
    SPI1->CR1 = SPI_CR1_MSTR |
                SPI_CR1_SSM |
                SPI_CR1_SSI |
                SPI_CR1_BR_1 |
                SPI_CR1_BR_0;

    SPI1->CR1 |= SPI_CR1_SPE;
}

// =====================================
// SPI1 write
// =====================================

void SPI1_Write(uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE)) {
    }

    *(__IO uint8_t *)&SPI1->DR = data;

    while (!(SPI1->SR & SPI_SR_RXNE)) {
    }

    (void)*(__IO uint8_t *)&SPI1->DR;

    while (SPI1->SR & SPI_SR_BSY) {
    }
}

// =====================================
// SSD1306 command
// =====================================

void SSD1306_Command(uint8_t command)
{
    GPIOA->BRR = OLED_DC;
    GPIOA->BRR = OLED_CS;

    SPI1_Write(command);

    GPIOA->BSRR = OLED_CS;
}

// =====================================
// SSD1306 data
// =====================================

void SSD1306_Data(uint8_t data)
{
    GPIOA->BSRR = OLED_DC;
    GPIOA->BRR = OLED_CS;

    SPI1_Write(data);

    GPIOA->BSRR = OLED_CS;
}

// =====================================
// SSD1306 initialization
// =====================================

void SSD1306_Init(void)
{
    // Hardware reset
    GPIOA->BRR = OLED_RES;
    Delay_ms(20);

    GPIOA->BSRR = OLED_RES;
    Delay_ms(20);

    SSD1306_Command(0xAE);

    SSD1306_Command(0xD5);
    SSD1306_Command(0x80);

    SSD1306_Command(0xA8);
    SSD1306_Command(0x3F);

    SSD1306_Command(0xD3);
    SSD1306_Command(0x00);

    SSD1306_Command(0x40);

    SSD1306_Command(0x8D);
    SSD1306_Command(0x14);

    SSD1306_Command(0x20);
    SSD1306_Command(0x02);

    SSD1306_Command(0xA1);
    SSD1306_Command(0xC8);

    SSD1306_Command(0xDA);
    SSD1306_Command(0x12);

    SSD1306_Command(0x81);
    SSD1306_Command(0x7F);

    SSD1306_Command(0xD9);
    SSD1306_Command(0xF1);

    SSD1306_Command(0xDB);
    SSD1306_Command(0x40);

    SSD1306_Command(0xA4);
    SSD1306_Command(0xA6);
    SSD1306_Command(0x2E);

    // Display ON
    SSD1306_Command(0xAF);
}

// =====================================
// Clear display
// =====================================

void SSD1306_Clear(void)
{
    for (uint8_t page = 0; page < 8; page++) {

        SSD1306_Command(0xB0 | page);
        SSD1306_Command(0x00);
        SSD1306_Command(0x10);

        for (uint8_t x = 0; x < 128; x++) {
            SSD1306_Data(0x00);
        }
    }
}

// =====================================
// Draw digit (40x56 pixels)
// =====================================

void SSD1306_DrawDigit(uint8_t number)
{
    if (number > 9) {
        return;
    }

    for (uint8_t page = 0; page < 7; page++) {

        // Select page
        SSD1306_Command(0xB0 | page);

        // Start column = 44
        SSD1306_Command(0x0C);
        SSD1306_Command(0x12);

        for (uint8_t col = 0; col < 5; col++) {

            uint8_t pixel;

            if (digits[number][col] & (1U << page)) {
                pixel = 0xFF;
            } else {
                pixel = 0x00;
            }

            for (uint8_t i = 0; i < 8; i++) {
                SSD1306_Data(pixel);
            }
        }
    }
}

// =====================================
// TIM1 initialization (1 second)
// =====================================

void TIM1_Init(void)
{
    // Enable TIM1 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->CR1 = 0;
    TIM1->DIER = 0;

    // Calculate TIM1 clock frequency
    uint32_t ppre2 = (RCC->CFGR >> 11) & 0x07U;
    uint32_t divider = 1U;

    if (ppre2 >= 4U) {
        divider = 1U << (ppre2 - 3U);
    }

    uint32_t timerClock = SystemCoreClock / divider;

    if (divider != 1U) {
        timerClock *= 2U;
    }

    // Timer frequency = 10000 Hz
    TIM1->PSC = timerClock / 10000U - 1U;

    // Update event every 1 second
    TIM1->ARR = 10000U - 1U;

    TIM1->CNT = 0;
    TIM1->RCR = 0;

    // Load prescaler
    TIM1->EGR = TIM_EGR_UG;

    // Clear interrupt flag
    TIM1->SR = 0;

    // Enable update interrupt
    TIM1->DIER |= TIM_DIER_UIE;

    // Enable interrupt in NVIC
    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);
    NVIC_EnableIRQ(TIM1_UP_IRQn);

    // Start timer
    TIM1->CR1 |= TIM_CR1_CEN;
}

// =====================================
// TIM1 interrupt handler
// =====================================

void TIM1_UP_IRQHandler(void)
{
    if (TIM1->SR & TIM_SR_UIF) {

        // Clear interrupt flag
        TIM1->SR &= ~TIM_SR_UIF;

        // Increment digit
        digit = (digit + 1U) % 10U;

        // Update display
        SSD1306_DrawDigit(digit);
    }
}

// =====================================
// Main
// =====================================

int __attribute__((noreturn)) main(void)
{
    Delay_Init();
    SPI1_Init();

    Delay_ms(100);

    SSD1306_Init();
    SSD1306_Clear();

    // Display initial digit
    SSD1306_DrawDigit(0);

    // Start timer
    TIM1_Init();

    while (1) {
        __NOP();
    }
}