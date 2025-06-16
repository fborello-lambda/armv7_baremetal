#include "inc/uart.h"

__attribute__((section(".text"))) void c_UART0_init() {
  _uart_t *const UART0 = (_uart_t *)UART0_ADDR;

  UART0->CR &= ~CR_UARTEN;
  while ((UART0->FR & FR_BUSY) != 0) {
  }
  UART0->LCRH &= ~LCRH_FEN;

  // Set baud rate to 115200:
  // PrimeCell® UART (PL011) Technical Reference Manual Page 56 // 3-10
  // Baud Rate Divisor = (24E6)/(16×115200) = 13.0208333
  // This means IBRD = 13 and m = 0.020833.
  // Therefore, the fractional part is, FBRD = integer((0.020833×64)+0.5) =
  // int(1.833) = 1

  UART0->IBRD = 13;
  UART0->FBRD = 1;

  // Set word size to 8 bits, no parity, one stop bit (8-N-1).
  UART0->LCRH |= (0b11 << 5); // WLEN
  UART0->LCRH &= !(1 << 1);   // PEN
  UART0->LCRH &= !(1 << 2);   // EPS
  UART0->LCRH &= !(1 << 7);   // SPS
  UART0->LCRH &= !(1 << 3);   // STP2

  UART0->LCRH |= LCRH_FEN;
  UART0->CR |= CR_UARTEN;
}

__attribute__((section(".text"))) void c_putchar(char c) {
  _uart_t *const UART0 = (_uart_t *)UART0_ADDR;

  while ((UART0->FR & FR_TXFF) != 0) {
  }
  UART0->DR = c;
}

__attribute__((section(".text"))) void c_puts(const char *s) {
  while (*s) {
    c_putchar(*s++);
  }
}

__attribute__((section(".text"))) void c_putsln(const char *s) {
  c_puts(s);
  c_puts("\n");
}

__attribute__((section(".text"))) void c_puts_hex(uint32_t val) {
  char buf[9];
  for (int i = 0; i < 8; i++) {
    buf[7 - i] = "0123456789ABCDEF"[val & 0xF];
    val >>= 4;
  }
  buf[8] = '\0';
  c_puts("0x");
  c_puts(buf);
}
