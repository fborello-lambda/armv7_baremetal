#ifndef __UART_LIB_H
#define __UART_LIB_H

#include <stdint.h>

#define UART0_ADDR 0x10009000

#define FR_BUSY (1 << 3u)
#define LCRH_FEN (1 << 4u)
#define CR_UARTEN (1 << 0u)
#define FR_RXFE (1 << 4u)
#define FR_TXFF (1 << 5u)

typedef struct {
  uint32_t DR;
  uint32_t RSRECR;
  uint32_t RESERVED0[4];
  uint32_t FR;
  uint32_t RESERVED1;
  uint32_t ILPR;
  uint32_t IBRD;
  uint32_t FBRD;
  uint32_t LCRH;
  uint32_t CR;
} _uart_t;

// Function Definitions

void c_UART0_init();
void c_putchar(char c);
void c_puts(const char *s);
void c_putsln(const char *s);
void c_puts_hex(uint32_t val);

#endif // __UART_LIB_H
