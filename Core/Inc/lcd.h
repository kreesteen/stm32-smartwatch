/*
 * lcd.h
 *
 *  Created on: Jun 27, 2025
 *      Author: kcbitoon
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

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

#define LCD_CLEAR_DISPLAY    0b0001
#define LCD_RETURN_HOME      0b0010
#define LCD_SET_SECOND_LINE  0xC0

void LCD_Init(void);
void LCD_SendCmd(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SendStr(char *str);
void LCD_GoTo(uint8_t row, uint8_t col);

#endif /* INC_LCD_H_ */
