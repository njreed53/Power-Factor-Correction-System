#include "stm32f3xx_hal.h"

extern I2C_HandleTypeDef hi2c1;


void LCD_Init(void);
void LCD_Enable(void);
void LCD_Start(void);
void LCD_Stop(void);
void LCD_WriteControl(uint8_t control_byte);
void LCD_WriteData(uint8_t data_byte);
void LCD_PrintString(char const string[]);
void LCD_Position(uint8_t row, uint8_t col);
void LCD_PutChar(char output_char);
void LCD_WrDataNyb(uint8_t data_nybble);
void LCD_WrCtrlNyb(uint8_t control_nybble);



#define LCD_CLEAR_DISPLAY_CMD (0x01u)
#define LCD_DISPLAY_4BIT_CMD (0x02u)
#define LCD_MOVE_CURSOR_HOME_CMD (0x02u)
#define LCD_DISPLAY_8BIT_CMD (0x03u)
#define LCD_RESET_CURSOR_POSITION_CMD (0x03u)
#define LCD_MOVE_CURSOR_LEFT_CMD (0x04u)
#define LCD_MOVE_CURSOR_RIGHT_CMD (0x06u)
#define LCD_CURSOR_AUTO_INCREMENT_CMD (0x06u)
#define LCD_TURN_OFF_DISPLAY_AND_CURSOR_CMD (0x08u)
#define LCD_TURN_ON_DISPLAY_BUT_CURSOR_OFF_CMD (0x0Cu)
#define LCD_WINK_CURSOR_CMD (0x0Du)
#define LCD_TURN_ON_DISPLAY_AND_CURSOR_CMD (0x0Eu)
#define LCD_BLINK_CURSOR_CMD (0x0Fu)
#define LCD_SHIFT_CURSOR_LEFT_CMD (0x10u)
#define LCD_SHIFT_CURSOR_RIGHT_CMD (0x14u)
#define LCD_SCROLL_DISPLAY_LEFT_CMD (0x18u)
#define LCD_SCROLL_DISPLAY_RIGHT_CMD (0x1Eu)
#define LCD_MODE_2_ROWS_5_BY_10_CMD (0x2Cu)

#define LCD_ROW_0_ST (0x80u)
#define LCD_ROW_1_ST (0xC0u)
#define LCD_ROW_2_ST (0x94u)
#define LCD_ROW_3_ST (0xD4u)

#define LCD_LONGEST_CMD_US (0x651u)
#define LCD_WAIT_CYCLE (0x10u)
#define LCD_READY_DELAY ((LCD_LONGEST_CMD_US * 4u)/(LCD_WAIT_CYCLE))


#define LCD_ClearDisplay() LCD_WriteControl(LCD_CLEAR_DISPLAY_CMD);

#define RS_MASK (0x01u);
#define RW_MASK (0x02u);
#define E_MASK (0x04u);
#define BL_MASK (0x08u);
#define DATA_MASK (0xF0u);


static const uint16_t lcd_addr = (0x27 << 1); //STM is dumb and makes us shift it
static const uint32_t I2C_TIMEOUT = 100;
static const uint32_t DELAY = 2000;

uint8_t bl_on = 1;
uint8_t is_lcd_initialized = 0;


void LCD_WrCtrlNyb(uint8_t control_nybble)
{
    uint8_t exp_byte = 0x00;
    uint32_t ix = 0;
    
    exp_byte &= ~RS_MASK;
   
    exp_byte &= ~RW_MASK;
     
    if (bl_on){
        exp_byte |= BL_MASK;
    }
    else{
        exp_byte &= ~BL_MASK;
    }
    exp_byte |= (control_nybble & 0x0F) << 4;
     
    exp_byte |= E_MASK;

    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr, &exp_byte, 1, I2C_TIMEOUT);
    for (ix = 0; ix < DELAY; ix++); //STM doesn't have a micro-second delay

    exp_byte &= ~E_MASK;

    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr, &exp_byte, 1, I2C_TIMEOUT);
    for (ix = 0; ix < DELAY; ix++);

}

void LCD_WrDataNyb(uint8_t data_nybble)
{
	uint8_t exp_byte = 0x00;
	uint32_t ix = 0;
    
    exp_byte |= RS_MASK;
    
    exp_byte &= ~RW_MASK;
    
    if (bl_on){
        exp_byte |= BL_MASK;
    }
    else{
        exp_byte &= ~BL_MASK;
    }

    exp_byte |= (data_nybble & 0x0F) << 4;
    exp_byte |= E_MASK;
    
    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr, &exp_byte, 1, I2C_TIMEOUT);
    for (ix = 0; ix < DELAY; ix++);
    
    exp_byte &= ~E_MASK;
    
    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr , &exp_byte, 1, I2C_TIMEOUT);
    for (ix = 0; ix < DELAY; ix++);
}

void LCD_WriteData(uint8_t data_byte)
{
    LCD_WrDataNyb((data_byte) >> 4);
    LCD_WrDataNyb((data_byte & 0x0F));    
}

void LCD_WriteControl(uint8_t control_byte)
{
    LCD_WrCtrlNyb((control_byte) >> 4);
    LCD_WrCtrlNyb((control_byte & 0x0F));    
}

void LCD_Init(void)
{
	HAL_Delay(40);
    LCD_WrCtrlNyb(LCD_DISPLAY_8BIT_CMD);
	HAL_Delay(5);
    LCD_WrCtrlNyb(LCD_DISPLAY_8BIT_CMD);
	HAL_Delay(15);
    LCD_WrCtrlNyb(LCD_DISPLAY_8BIT_CMD);
	HAL_Delay(1);
    LCD_WrCtrlNyb(LCD_DISPLAY_4BIT_CMD);
	HAL_Delay(5);
    
	LCD_WriteControl(LCD_CURSOR_AUTO_INCREMENT_CMD);
	LCD_WriteControl(LCD_TURN_ON_DISPLAY_AND_CURSOR_CMD);
	LCD_WriteControl(LCD_MODE_2_ROWS_5_BY_10_CMD);
	LCD_WriteControl(LCD_TURN_OFF_DISPLAY_AND_CURSOR_CMD);
	LCD_WriteControl(LCD_CLEAR_DISPLAY_CMD);
	LCD_WriteControl(LCD_TURN_ON_DISPLAY_BUT_CURSOR_OFF_CMD);
	LCD_WriteControl(LCD_RESET_CURSOR_POSITION_CMD);
    
	HAL_Delay(5);
}

void LCD_Enable(void)
{
	LCD_WriteControl(LCD_TURN_ON_DISPLAY_BUT_CURSOR_OFF_CMD);
}

void LCD_Start(void)
{
	if (is_lcd_initialized == 0)
	{
		LCD_Init();
		is_lcd_initialized = 1;
	}

	LCD_Enable();
}

void LCD_Stop(void)
{
	LCD_WriteControl(LCD_TURN_OFF_DISPLAY_AND_CURSOR_CMD);
}

void LCD_Position(uint8_t row, uint8_t col)
{
	switch (row)
	{
		case 0:
			LCD_WriteControl(LCD_ROW_0_ST + col);
			break;
		case 1:
			LCD_WriteControl(LCD_ROW_1_ST + col);
			break;
		case 2:
			LCD_WriteControl(LCD_ROW_2_ST + col);
			break;
		case 3:
			LCD_WriteControl(LCD_ROW_3_ST + col);
			break;
		default:
			break;
	}
}

void LCD_PrintString(char const string[])
{
	uint8_t idx = 1;
	char current = *string;

	while ('\0' != current)
	{
		LCD_WriteData((uint8_t) current);
		current = string[idx];
		idx++;
	}
}

void LCD_PutChar(char output_char)
{
	LCD_WriteData((uint8_t) output_char);
}
