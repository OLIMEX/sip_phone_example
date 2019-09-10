
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "soc/spi_reg.h"
#include "driver/i2c.h"
#include "board_pins_config.h"
#include "stmpe610.h"

#define USE_TOUCH TOUCH_TYPE_STMPE610
extern int orientation;
int stmpe610_read_touch(int16_t *x, int16_t *y, uint8_t raw);
static void IRAM_ATTR stmpe610_write_reg(uint8_t reg, uint8_t val) ;
static int i2c_init();
static uint8_t IRAM_ATTR stmpe610_read_byte(uint8_t reg);
static uint16_t IRAM_ATTR stmpe610_read_word(uint8_t reg);
int stmpe610_get_touch(uint16_t *x, uint16_t *y, uint16_t *z);

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no more data to be read
 */
bool TP_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    bool valid = true;
   
    int16_t x = 0;
    int16_t y = 0;
    
   stmpe610_read_touch(&x, &y, 0);

   if(x != 0) {
    
        last_x = x;
        last_y = y;

 //printf("Read TP TWI. x=%d, y=%d\n",x,y);
    } else {
        x = last_x;
        y = last_y;
        valid = false;
    }

    data->point.x = x;
    data->point.y = y;
    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;

    return false;
}



int stmpe610_read_touch(int16_t *x, int16_t *y, uint8_t raw)
{
    *x = 0;
    *y = 0;
    
	int result = -1;
    int X=0, Y=0;

    uint32_t tp_calx = TP_CALX_STMPE610;
    uint32_t tp_caly = TP_CALY_STMPE610;
    uint16_t Xx, Yy, Z=0;
    result = stmpe610_get_touch(&Xx, &Yy, &Z);
    if (result == 0) return 0;
    X = Xx;
    Y = Yy;


    if (raw) {
    	*x = X;
    	*y = Y;
    	return 1;
    }
	X>>=1;
	Y>>=1;
    // Calibrate the result
	int tmp;
	int xleft   = (tp_calx >> 16) & 0x3FFF;
	int xright  = tp_calx & 0x3FFF;
	int ytop    = (tp_caly >> 16) & 0x3FFF;
	int ybottom = tp_caly & 0x3FFF;

	if (((xright - xleft) <= 0) || ((ybottom - ytop) <= 0)) return 0;

        int width = _width;
        int height = _height;
        if (_width > _height) {
            width = _height;
            height = _width;
        }
		X = ((X - xleft) * width) / (xright - xleft);
		Y = ((Y - ytop) * height) / (ybottom - ytop);

		if (X < 0) X = 0;
		if (X > width-1) X = width-1;
		if (Y < 0) Y = 0;
		if (Y > height-1) Y = height-1;

		switch (orientation) {
			case PORTRAIT:
				Y = 319 - Y;
				break;
			case PORTRAIT_FLIP:
				X = width - X - 1;
				Y = height - Y - 1;
				break;
			case LANDSCAPE:
				tmp = X;
				X = 319 - Y;
				Y = width - tmp -1;
				break;
			case LANDSCAPE_FLIP:
				tmp = X;
				X = height - Y -1;
				Y = tmp;
				break;
		}

	//Y = 319 - Y;
	// X = 319 - X;
	 if (X == 0) return 0;	

	*x = X;
	*y = Y;
	return 1;

}
// ==== STMPE610 ===============================================================

// ----- STMPE610 --------------------------------------------------------------------------

// Send 1 byte display command, display must be selected
//---------------------------------------------------------
static void IRAM_ATTR stmpe610_write_reg(uint8_t reg, uint8_t val) {

    int res = 0;
    uint8_t slave_add = STMPE610_ADDR;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, slave_add, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, reg, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, val, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
     
    
    
}

static i2c_config_t es_i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000
};

static int i2c_init()
{

    int res;
    get_i2c_pins(I2C_PORT, &es_i2c_cfg);
  //    es_i2c_cfg.sda_io_num = I2C_SDA;
  //    es_i2c_cfg.scl_io_num = I2C_SCL;   
    res = i2c_param_config(I2C_PORT, &es_i2c_cfg);
    res |= i2c_driver_install(I2C_PORT, es_i2c_cfg.mode, 0, 0, 0);
    if (res) printf("i2c_init error"); else printf("i2c_init success\n");
    return res;
}

//-----------------------------------------------
static uint8_t IRAM_ATTR stmpe610_read_byte(uint8_t reg) {

 uint8_t data = 0;
 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // first, send device address (indicating write) & register to be read
    i2c_master_write_byte(cmd, STMPE610_ADDR, 1);
    // send register we want
    i2c_master_write_byte(cmd, reg, 1);
    // Send repeated start
    i2c_master_start(cmd);
    // now send device address (indicating read) & read data
    i2c_master_write_byte(cmd, STMPE610_ADDR | 1, 1);
    i2c_master_read_byte(cmd, &data, 1);
    i2c_master_stop(cmd);
    //esp_err_t ret = 
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    //printf("i2c read byte 0x%02x: 0x%02x\r\n",reg,data);
    return data;

}

//-----------------------------------------
static uint16_t IRAM_ATTR stmpe610_read_word(uint8_t reg) {

 uint8_t data[2] = {0,0};
 uint16_t regdata = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // first, send device address (indicating write) & register to be read
    i2c_master_write_byte(cmd, STMPE610_ADDR, 1);
    // send register we want
    i2c_master_write_byte(cmd, reg, 1);
    // Send repeated start
    i2c_master_start(cmd);
    // now send device address (indicating read) & read data
    i2c_master_write_byte(cmd, STMPE610_ADDR | 1, 1);
    i2c_master_read(cmd,(uint8_t *) data,2, 0);
    i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    regdata = data[0] | ( data[1] << 8);
    return regdata;

}

//-----------------------
uint32_t stmpe610_getID()
{
    uint16_t tid = stmpe610_read_word(0);
    uint8_t tver = stmpe610_read_byte(2);
    return (tid << 8) | tver;
}

//==================
void stmpe610_Init()
{

	i2c_init();
	
    stmpe610_write_reg(STMPE610_REG_SYS_CTRL1, 0x02);        // Software chip reset
    vTaskDelay(10 / portTICK_RATE_MS);

  stmpe610_write_reg(STMPE_SYS_CTRL2, 0x0); // turn on clocks!
  stmpe610_write_reg(STMPE_TSC_CTRL, STMPE_TSC_CTRL_XYZ | STMPE_TSC_CTRL_EN); // XYZ and enable!
  stmpe610_write_reg(STMPE_INT_EN, STMPE_INT_EN_TOUCHDET);
  stmpe610_write_reg(STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_10BIT | (0x6 << 4)); // 96 clocks per conversion
  stmpe610_write_reg(STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_6_5MHZ);
  stmpe610_write_reg(STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE | STMPE_TSC_CFG_DELAY_1MS | STMPE_TSC_CFG_SETTLE_5MS);
  stmpe610_write_reg(STMPE_TSC_FRACTION_Z, 0x6);
  stmpe610_write_reg(STMPE_FIFO_TH, 1);
  stmpe610_write_reg(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
  stmpe610_write_reg(STMPE_FIFO_STA, 0);    // unreset
  stmpe610_write_reg(STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
  stmpe610_write_reg(STMPE_INT_STA, 0xFF); // reset all ints
  stmpe610_write_reg(STMPE_INT_CTRL, STMPE_INT_CTRL_POL_HIGH | STMPE_INT_CTRL_ENABLE);


}

//===========================================================
int stmpe610_get_touch(uint16_t *x, uint16_t *y, uint16_t *z)
{
  uint8_t data[5]={0,0,0,0,0};

  uint8_t reg = 0x4d;
  
  if ((stmpe610_read_byte(STMPE_TSC_CTRL) & 0x80)==0) return 0;
  
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // first, send device address (indicating write) & register to be read
    i2c_master_write_byte(cmd, STMPE610_ADDR, 1);
    // send register we want
    i2c_master_write_byte(cmd, reg, 1);
    // Send repeated start
    i2c_master_start(cmd);
    // now send device address (indicating read) & read data
    i2c_master_write_byte(cmd, STMPE610_ADDR | 1, 1);
    i2c_master_read(cmd,(uint8_t *) data,5, 0);
    i2c_master_stop(cmd);
   //esp_err_t ret = 
   i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
   *x = data[1];
  *x <<= 8;
  *x |= data[0];
  *y = data[3];
  *y <<= 8;
  *y |= data[2];
  *z = data[4];
 
	return 1;

}

// ==== STMPE610 =======================================
