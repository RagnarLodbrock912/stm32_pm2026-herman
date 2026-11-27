#include <stdint.h>
#include <stm32f10x.h>

#define UART_BAUDRATE 1200U

// UART initialization
void UART_Init(void)
{
    SystemCoreClockUpdate();

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_AFIOEN |
                    RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~(GPIO_CRH_MODE9 |
                    GPIO_CRH_CNF9 |
                    GPIO_CRH_MODE10 |
                    GPIO_CRH_CNF10);

    GPIOA->CRH |= GPIO_CRH_MODE9_1 |
                  GPIO_CRH_CNF9_1 |
                  GPIO_CRH_CNF10_0;

    uint32_t ppre2 = (RCC->CFGR >> 11) & 7U;
    uint32_t divider = 1U;

    if (ppre2 >= 4U) {
        divider = 1U << (ppre2 - 3U);
    }

    uint32_t pclk2 = SystemCoreClock / divider;

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    USART1->BRR = (pclk2 + UART_BAUDRATE / 2U)
                  / UART_BAUDRATE;

    USART1->CR1 = USART_CR1_UE |
                  USART_CR1_TE |
                  USART_CR1_RE;
}

// Send character
void UART_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) {
    }

    USART1->DR = (uint8_t)c;
}

// Send string
void UART_SendString(const char *str)
{
    while (*str != '\0') {
        UART_SendChar(*str++);
    }
}

// Receive one character
char UART_ReceiveChar(void)
{
    while (!(USART1->SR & USART_SR_RXNE)) {
    }

    return (char)(USART1->DR & 0xFFU);
}

// Send integer
void UART_SendInt(int32_t number)
{
    char buffer[12];
    uint32_t index = 0;
    uint32_t value;

    if (number < 0) {
        UART_SendChar('-');
        value = 0U - (uint32_t)number;
    } else {
        value = (uint32_t)number;
    }

    do {
        buffer[index++] = (char)('0' + value % 10U);
        value /= 10U;
    } while (value != 0U);

    while (index > 0U) {
        UART_SendChar(buffer[--index]);
    }
}

// Main
int __attribute__((noreturn)) main(void)
{
    int32_t a = 0;
    int32_t b = 0;
    int64_t result = 0;

    char operation = 0;

    uint8_t hasA = 0;
    uint8_t hasB = 0;
    uint8_t negativeA = 0;
    uint8_t negativeB = 0;
    uint8_t error = 0;

    UART_Init();

    UART_SendString("\r\nSTM32 UART Calculator\r\n");
    UART_SendString("Send one character at a time\r\n");
    UART_SendString("Press = to calculate\r\n");
    UART_SendString("Press C to clear\r\n\r\n");
    UART_SendString("> ");

    while (1) {

        char c = UART_ReceiveChar();

        // Ignore line endings and spaces
        if (c == '\r' || c == '\n' || c == ' ') {
            continue;
        }

        // Clear calculator
        if (c == 'C' || c == 'c') {

            a = 0;
            b = 0;
            operation = 0;

            hasA = 0;
            hasB = 0;
            negativeA = 0;
            negativeB = 0;
            error = 0;

            UART_SendString("\r\nCleared\r\n> ");
            continue;
        }

        // Display received character
        UART_SendChar(c);

        // Read digit
        if (c >= '0' && c <= '9') {

            int32_t digit = (int32_t)(c - '0');

            if (!error) {

                if (operation == 0) {

                    if (a > (INT32_MAX - digit) / 10) {
                        error = 1;
                    } else {
                        a = a * 10 + digit;
                        hasA = 1;
                    }

                } else {

                    if (b > (INT32_MAX - digit) / 10) {
                        error = 1;
                    } else {
                        b = b * 10 + digit;
                        hasB = 1;
                    }
                }
            }
        }

        // Read operation
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '/') {

            if (c == '-' && !hasA && operation == 0) {
                negativeA = 1;
            }

            else if (c == '-' && operation != 0 && !hasB) {
                negativeB = 1;
            }

            else if (hasA && operation == 0) {
                operation = c;
            }

            else {
                error = 1;
            }
        }

        // Calculate result
        else if (c == '=') {

            UART_SendString("\r\n");

            if (error || !hasA || !hasB || operation == 0) {

                UART_SendString("Error: invalid expression\r\n");

            } else {

                if (negativeA) {
                    a = -a;
                }

                if (negativeB) {
                    b = -b;
                }

                uint8_t valid = 1;

                switch (operation) {

                    case '+':
                        result = (int64_t)a + (int64_t)b;
                        break;

                    case '-':
                        result = (int64_t)a - (int64_t)b;
                        break;

                    case '*':
                        result = (int64_t)a * (int64_t)b;
                        break;

                    case '/':
                        if (b == 0) {

                            UART_SendString(
                                "Error: division by zero\r\n"
                            );

                            valid = 0;

                        } else {
                            result = (int64_t)(a / b);
                        }
                        break;

                    default:
                        valid = 0;
                        break;
                }

                if (valid) {

                    if (result > INT32_MAX ||
                        result < INT32_MIN) {

                        UART_SendString(
                            "Error: integer overflow\r\n"
                        );

                    } else {

                        UART_SendString("Result: ");
                        UART_SendInt((int32_t)result);
                        UART_SendString("\r\n");
                    }
                }
            }

            // Reset for next expression
            a = 0;
            b = 0;
            result = 0;
            operation = 0;

            hasA = 0;
            hasB = 0;
            negativeA = 0;
            negativeB = 0;
            error = 0;

            UART_SendString("\r\n> ");
        }

        else {
            error = 1;
        }
    }
}