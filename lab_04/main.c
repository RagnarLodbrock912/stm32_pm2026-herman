
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
    // Enable GPIOA, AFIO and SPI1 clocks
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_AFIOEN |
                    RCC_APB2ENR_SPI1EN;

    // Clear configuration for PA2-PA7
    GPIOA->CRL &= ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2 |
                    GPIO_CRL_CNF3 | GPIO_CRL_MODE3 |
                    GPIO_CRL_CNF4 | GPIO_CRL_MODE4 |
                    GPIO_CRL_CNF5 | GPIO_CRL_MODE5 |
                    GPIO_CRL_CNF6 | GPIO_CRL_MODE6 |
                    GPIO_CRL_CNF7 | GPIO_CRL_MODE7);

    // PA2, PA3, PA4: Output Push-Pull, 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE2_1 |
                  GPIO_CRL_MODE3_1 |
                  GPIO_CRL_MODE4_1;

    // PA5 (SCK), PA7 (MOSI): Alternate Function Push-Pull
    GPIOA->CRL |= GPIO_CRL_MODE5_1 |
                  GPIO_CRL_CNF5_1 |
                  GPIO_CRL_MODE7_1 |
                  GPIO_CRL_CNF7_1;

    // PA6 (MISO): Floating Input
    GPIOA->CRL |= GPIO_CRL_CNF6_0;

    // Initial pin states
    GPIOA->BSRR = OLED_CS | OLED_RES;
    GPIOA->BRR = OLED_DC;

    // Disable SPI before configuration
    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    // Master, 8-bit, Mode 0, Software NSS
    // Baud rate = PCLK2 / 16
    SPI1->CR1 = SPI_CR1_MSTR |
                SPI_CR1_SSM |
                SPI_CR1_SSI |
                SPI_CR1_BR_1 |
                SPI_CR1_BR_0;

    // Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;
}

// =====================================
// SPI1 write
// =====================================

void SPI1_Write(uint8_t data)
{
    // Wait for empty transmit buffer
    while (!(SPI1->SR & SPI_SR_TXE)) {
    }

    *(__IO uint8_t *)&SPI1->DR = data;

    // Wait for received byte
    while (!(SPI1->SR & SPI_SR_RXNE)) {
    }

    // Clear RXNE
    (void)*(__IO uint8_t *)&SPI1->DR;

    // Wait until transmission ends
    while (SPI1->SR & SPI_SR_BSY) {
    }
}

// =====================================
// SPI1 read
// =====================================

uint8_t SPI1_Read(void)
{
    while (!(SPI1->SR & SPI_SR_TXE)) {
    }

    // Send dummy byte to generate clock
    *(__IO uint8_t *)&SPI1->DR = 0xFF;

    while (!(SPI1->SR & SPI_SR_RXNE)) {
    }

    uint8_t data = *(__IO uint8_t *)&SPI1->DR;

    while (SPI1->SR & SPI_SR_BSY) {
    }

    return data;
}

// =====================================
// SSD1306 command
// =====================================

void SSD1306_Command(uint8_t command)
{
    GPIOA->BRR = OLED_DC;  // Command mode
    GPIOA->BRR = OLED_CS;  // Select display

    SPI1_Write(command);

    GPIOA->BSRR = OLED_CS; // Deselect display
}

// =====================================
// SSD1306 data
// =====================================

void SSD1306_Data(uint8_t data)
{
    GPIOA->BSRR = OLED_DC; // Data mode
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

    SSD1306_Command(0xAE); // Display OFF

    SSD1306_Command(0xD5);
    SSD1306_Command(0x80); // Clock divide

    SSD1306_Command(0xA8);
    SSD1306_Command(0x3F); // Multiplex 64

    SSD1306_Command(0xD3);
    SSD1306_Command(0x00); // Display offset

    SSD1306_Command(0x40); // Start line

    SSD1306_Command(0x8D);
    SSD1306_Command(0x14); // Charge pump ON

    SSD1306_Command(0x20);
    SSD1306_Command(0x02); // Page addressing mode

    SSD1306_Command(0xA1); // Segment remap
    SSD1306_Command(0xC8); // COM scan direction

    SSD1306_Command(0xDA);
    SSD1306_Command(0x12); // COM pins

    SSD1306_Command(0x81);
    SSD1306_Command(0x7F); // Contrast

    SSD1306_Command(0xD9);
    SSD1306_Command(0xF1); // Pre-charge

    SSD1306_Command(0xDB);
    SSD1306_Command(0x40); // VCOMH

    SSD1306_Command(0xA4); // Normal display RAM
    SSD1306_Command(0xA6); // Normal colors

    SSD1306_Command(0x2E); // Deactivate scroll
    SSD1306_Command(0xAF); // Display ON
}

// =====================================
// Draw chessboard 8x8
// =====================================

void SSD1306_DrawChessboard(void)
{
    // Display: 128x64 pixels
    // Each square: 16x8 pixels

    for (uint8_t page = 0; page < 8; page++) {

        // Select page
        SSD1306_Command(0xB0 | page);

        // Set column to zero
        SSD1306_Command(0x00);
        SSD1306_Command(0x10);

        for (uint8_t x = 0; x < 128; x++) {

            // Alternate black and white squares
            uint8_t square = ((x / 16) + page) % 2;

            if (square) {
                SSD1306_Data(0xFF);
            } else {
                SSD1306_Data(0x00);
            }
        }
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

    SSD1306_DrawChessboard();

    while (1) {
        __NOP();
    }
}
