#include "lcd.h"
#include "main.h"

#include "stdint.h"

#define D4_PIN GPIO_PIN_8
#define D5_PIN GPIO_PIN_9
#define D6_PIN GPIO_PIN_10
#define D7_PIN GPIO_PIN_11
#define DATA_PORT GPIOC

#define RS_PIN GPIO_PIN_15
#define RS_PORT GPIOA

#define RW_PIN GPIO_PIN_6
#define RW_PORT GPIOC

#define E_PIN GPIO_PIN_2
#define E_PORT GPIOD

void LCD_Pulse(void) {
    HAL_GPIO_WritePin(E_PORT, E_PIN, 1);
    HAL_Delay(10);
    HAL_GPIO_WritePin(E_PORT, E_PIN, 0);
    HAL_Delay(10);
}

void LCD_PutNibble(uint8_t nibble) {
    HAL_GPIO_WritePin(DATA_PORT, D4_PIN, (nibble >> 0) & 1);
    HAL_GPIO_WritePin(DATA_PORT, D5_PIN, (nibble >> 1) & 1);
    HAL_GPIO_WritePin(DATA_PORT, D6_PIN, (nibble >> 2) & 1);
    HAL_GPIO_WritePin(DATA_PORT, D7_PIN, (nibble >> 3) & 1);
}

void LCD_SendCmd(uint8_t cmd) {
    HAL_GPIO_WritePin(RS_PORT, RS_PIN, 0); // RS = 0 for command
    HAL_GPIO_WritePin(RW_PORT, RW_PIN, 0); // RW = 0 for write

    LCD_PutNibble(cmd >> 4);
    LCD_Pulse();
    LCD_PutNibble(cmd & 0x0F);
    LCD_Pulse();
    HAL_Delay(50);
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(RS_PORT, RS_PIN, 1); // RS = 1 for data
    HAL_GPIO_WritePin(RW_PORT, RW_PIN, 0); // RW = 0 for write

    LCD_PutNibble(data >> 4);
    LCD_Pulse();
    LCD_PutNibble(data & 0x0F);
    LCD_Pulse();

    HAL_GPIO_WritePin(RS_PORT, RS_PIN, 0);
    HAL_Delay(10);

}

void LCD_SendStr(char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

void LCD_Init(void) {
    HAL_Delay(60); // Wait after power-up

    // SAFETY: ensure RW and E are low before starting anything
	HAL_GPIO_WritePin(RW_PORT, RW_PIN, 0); // RW = 0
	HAL_GPIO_WritePin(RS_PORT, RS_PIN, 0); // RS = 0
	HAL_GPIO_WritePin(E_PORT, E_PIN, 0);   // E = 0

	HAL_Delay(10);

	// Begin init sequence
    LCD_PutNibble(0b0011); LCD_Pulse(); HAL_Delay(10);
    LCD_PutNibble(0b0011); LCD_Pulse(); HAL_Delay(10);
    LCD_PutNibble(0b0011); LCD_Pulse(); HAL_Delay(10);
    LCD_PutNibble(0b0010); LCD_Pulse(); HAL_Delay(10); // Switch to 4-bit mode

    LCD_SendCmd(0b00101000); // Function Set: 4-bit, 2 lines, 5x8 dots
    LCD_SendCmd(0b00001000); // Display off
    LCD_SendCmd(0b00000001);
    HAL_Delay(2);
    LCD_SendCmd(0b00000110); // Entry Mode Set
    LCD_SendCmd(0b00001100); // Display ON, Cursor ON, Blink ON
}

void LCD_GoTo(uint8_t row, uint8_t col) {
    uint8_t address;

    // Calculate the DDRAM address based on the row
    switch (row) {
        case 0:
            address = 0x00; // Address for the first line
            break;
        case 1:
            address = 0x40; // Address for the second line
            break;
        default:
            address = 0x00;
    }

    // Add the column offset and send the command
    address += col;
    LCD_SendCmd(0x80 | address); // 0x80 is the command to set DDRAM address
}
