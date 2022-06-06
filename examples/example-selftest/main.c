/*
 * __________________________________________________________________
 *
 * Copyright (C) [2022] by InvenSense, Inc.
 * 
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY  AND FITNESS. IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE  FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * __________________________________________________________________
 */

#include "example-selftest.h"

/* InvenSense utils */
#include "Message.h"

/* std */
#include <stdio.h>


/* --------------------------------------------------------------------------------------
 *  Example configuration
 * -------------------------------------------------------------------------------------- */

/*
 * Select UART port on which INV_MSG() will be printed.
 */
#define LOG_UART_ID INV_UART_SENSOR_CTRL

/* 
 * Timer used throughout standalone applications 
 */
#define DELAY_TIMER INV_TIMER3
#define TIMEBASE_TIMER INV_TIMER2

/* 
 * Select communication between SmartMotion and IXM426xx by setting correct SERIF_TYPE 
 */
#define SERIF_TYPE IXM42XXX_UI_SPI4
//#define SERIF_TYPE IXM42XXX_UI_I2C

/* 
 * Define msg level 
 */
#define MSG_LEVEL INV_MSG_LEVEL_DEBUG


/* --------------------------------------------------------------------------------------
 *  Forward declaration
 * -------------------------------------------------------------------------------------- */

static void SetupMCUHardware(struct inv_ixm42xxx_serif * icm_serif);
static void check_rc(int rc, const char * msg_context);
void msg_printer(int level, const char * str, va_list ap);

int inv_io_hal_read_reg(struct inv_ixm42xxx_serif * serif, uint8_t reg, uint8_t * rbuffer, uint32_t rlen)
{
	//switch (serif->serif_type) {
	//	case IXM42XXX_UI_SPI4:
	//		return inv_spi_master_read_register(INV_SPI_AP, reg, rlen, rbuffer);
	//	case IXM42XXX_UI_I2C:
	//		while(inv_i2c_master_read_register(ICM_I2C_ADDR, reg, rlen, rbuffer)) {
	//			inv_delay_us(32000); // Loop in case of I2C timeout
	//		}
	//		return 0;
	//	default:
	//		return -1;
	//}
	return 0;
}

int inv_io_hal_write_reg(struct inv_ixm42xxx_serif * serif, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen)
{
	//int rc;

	//switch (serif->serif_type) {
	//	case IXM42XXX_UI_SPI4:
	//		for(uint32_t i=0; i<wlen; i++) {
	//			rc = inv_spi_master_write_register(INV_SPI_AP, reg+i, 1, &wbuffer[i]);
	//			if(rc)
	//				return rc;
	//		}
	//		return 0;
	//	case IXM42XXX_UI_I2C:
	//		while(inv_i2c_master_write_register(ICM_I2C_ADDR, reg, wlen, wbuffer)) {
	//			inv_delay_us(32000); // Loop in case of I2C timeout
	//		}
	//		return 0;
	//	default:
	//		return -1;
	//}
	return 0;
}

/* --------------------------------------------------------------------------------------
 *  Main
 * -------------------------------------------------------------------------------------- */
 
int main(void)
{
	int rc = 0;
	struct inv_ixm42xxx_serif ixm42xxx_serif;

	/* Initialize MCU hardware */
	SetupMCUHardware(&ixm42xxx_serif);

	/* Initialize Ixm42xxx */
	rc = SetupInvDevice(&ixm42xxx_serif);
	check_rc(rc, "error while setting up INV device");
	
	/* Perform Self-Test */
	RunSelfTest();

	/* Get Low Noise / Low Power bias computed by self-tests scaled by 2^16 */
	GetBias();

	/* Add a delay so the messages get fully printed out */
	//inv_delay_us(1000000);
}


/* --------------------------------------------------------------------------------------
 *  Functions definitions
 * -------------------------------------------------------------------------------------- */

/*
 * This function initializes MCU on which this software is running.
 * It configures:
 *   - a UART link used to print some messages
 *   - interrupt priority group
 *   - a microsecond timer requested by Ixm42xxx driver to compute some delay
 *   - a serial link to communicate from MCU to Ixm42xxx
 */
static void SetupMCUHardware(struct inv_ixm42xxx_serif * icm_serif)
{
	//inv_board_hal_init();

	/* configure UART */
	//config_uart(LOG_UART_ID);

	/* Setup message facility to see internal traces from FW */
	INV_MSG_SETUP(MSG_LEVEL, msg_printer);

	INV_MSG(INV_MSG_LEVEL_INFO, "#########################");
	INV_MSG(INV_MSG_LEVEL_INFO, "#   Example Self-Test   #");
	INV_MSG(INV_MSG_LEVEL_INFO, "#########################");

	/* Init timer peripheral for delay */
	//inv_delay_init(DELAY_TIMER);
	
	/* Initialize serial inteface between MCU and Ixm42xxx */
	icm_serif->context   = 0;        /* no need */
	icm_serif->read_reg  = inv_io_hal_read_reg;
	icm_serif->write_reg = inv_io_hal_write_reg;
	icm_serif->max_read  = 1024*32;  /* maximum number of bytes allowed per serial read */
	icm_serif->max_write = 1024*32;  /* maximum number of bytes allowed per serial write */
	icm_serif->serif_type = SERIF_TYPE;
	//inv_io_hal_init(icm_serif);
}

/*
 * Helper function to check RC value and block programm execution
 */
static void check_rc(int rc, const char * msg_context)
{
	if(rc < 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "%s: error %d (%s)\r\n", msg_context, rc, inv_error_str(rc));
		while(1);
	}
}

/*
 * Printer function for message facility
 */
void msg_printer(int level, const char * str, va_list ap)
{
	static char out_str[256]; /* static to limit stack usage */
	unsigned idx = 0;
	const char * s[INV_MSG_LEVEL_MAX] = {
	    "",    // INV_MSG_LEVEL_OFF
	    "[E] ", // INV_MSG_LEVEL_ERROR
	    "[W] ", // INV_MSG_LEVEL_WARNING
	    "[I] ", // INV_MSG_LEVEL_INFO
	    "[V] ", // INV_MSG_LEVEL_VERBOSE
	    "[D] ", // INV_MSG_LEVEL_DEBUG
	};
	idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "%s", s[level]);
	if(idx >= (sizeof(out_str)))
		return;
	idx += vsnprintf(&out_str[idx], sizeof(out_str) - idx, str, ap);
	if(idx >= (sizeof(out_str)))
		return;
	idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "\r\n");
	if(idx >= (sizeof(out_str)))
		return;

	//inv_uart_mngr_puts(LOG_UART_ID, out_str, idx);
}


/* --------------------------------------------------------------------------------------
 *  Extern functions definition
 * -------------------------------------------------------------------------------------- */

/*
 * Ixm42xxx driver needs to get time in us. Let's give its implementation here.
 */
uint64_t inv_ixm42xxx_get_time_us(void)
{
	//return inv_timer_get_counter(TIMEBASE_TIMER);
	return 0;
}

/*
 * Clock calibration module needs to disable IRQ. Thus inv_helper_disable_irq is
 * defined as extern symbol in clock calibration module. Let's give its implementation
 * here.
 */
void inv_helper_disable_irq(void)
{
	//inv_disable_irq();
}

/*
 * Clock calibration module needs to enable IRQ. Thus inv_helper_enable_irq is
 * defined as extern symbol in clock calibration module. Let's give its implementation
 * here.
 */
void inv_helper_enable_irq(void)
{
	//inv_enable_irq();
}

/*
 * Ixm42xxx driver needs a sleep feature from external device. Thus inv_ixm42xxx_sleep_us
 * is defined as extern symbol in driver. Let's give its implementation here.
 */
 void inv_ixm42xxx_sleep_us(uint32_t us)
{
	//inv_delay_us(us);
}
