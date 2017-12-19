/*
 * main.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */


#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "uart.h"
#include "i2c.h"
#include "dipswitches.h"
#include "accelerometer.h"
#include "encoder.h"
#include "system_tick.h"
#include "table.h"
#include "led.h"
#include "relais.h"
#include "crc3bit.h"
#include "alarms.h"

/*
 * condizione normale:--> PRIORITA' 2
 *        VERDE LAMPEGGIA     ROSSO SPENTO
 * errore soglia ampiezza superata: --> PRIORITA' 2 led rosso, distinguere X ed Y? STACCARE RELAIS SE ALLARME ATTIVO, LED RIMANE IMPOSTATO COSI' PER 1 MINUTO
 *    X   VERDE LAMPEGGIA     ROSSO LAMPEGGIO LENTO
 *    Y   VERDE LAMPEGGIA     ROSSO LAMPEGGIO VELOCE
 *    X+Y VERDE LAMPEGGIA     ROSSO ACCESO FISSO
 * errore scheda--> PRIORITA' 0
 *    - I2C: lettura accelerometro o eeprom, STACCARE RELAIS SE ALLARME ATTIVO, LED RIMANE IMPOSTATO COSI' FINCHE' MACCHINA SPENTA E RIACCESA
 *    - watchdog reset                       STACCARE RELAIS SE ALLARME ATTIVO, LED RIMANE IMPOSTATO COSI' FINCHE' MACCHINA SPENTA E RIACCESA
 *    X   VERDE SPENTO        ROSSO ACCESO FISSO
 * errore dipswitch impostato su entry non valida --> PRIORITA' 1
 *    X   VERDE ACCESO FISSO ROSSO ACCESO FISSO   STACCARE RELAIS SE ALLARME ATTIVO, LED RIMANE IMPOSTATO COSI' FINCHE' DIPSWITCH OK
 *
 *
 * invertire uscita relais, attivo a OFF
 * dipswitch: posizione 0 non è mai ammessa, si parte da 1 in su
 * - 5 bit bastano per definire la tabella, gli altri 3 si usano per validare i 5 bit
 *
 */

// 1.0
// tabella delle soglie portata in area codice, aggiunta label
//
// versione 0.3
// - aggiunta gestione watch dog
// versione 0.2
// - lampeggio LED come da specifiche riceute da Przemek email 19 aprile 2017
// - cambiato formato tabella EEPROM che adesso include: speed X, speed Y, ampiezza X ed ampiezza Y
// - DIP1: i primi 5 dip switch selezionano l'indice tabella, da 1 a 31, gli ultimi 3 impostano il CRC
//   INDICE: 1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//   CRC   : 3  6  5  7  4  1  2  5  6   3  0  2  1  4  7  1  2  7  4  6  5  0  3  4  7  2  1  3  0  5  6
//

// 1.1 aggiunti tre modelli in tabella: LM85, LM85_EM e D2W30
//
// 1.2 aggiunto modello LM85_EM2 abilitato da presenza di define "def_DE_LONGO_VALUES"
//
// 1.3 modificati valori LM85 e LM85_EM (indici 16 e 17), abilitata da define "def_NUOVE_MOLLE_LM_100_VALUES", come da email di richiesta da Silvano Guiotto in data 11 Dec 2017
//     modello LM85_EM2 disabilitato, eliminata define "def_DE_LONGO_VALUES"
//
// 0.4 ricompilata in modo eeprom+seriale senza watchdog
//
#define DEF_FIRMWARE_VERSION_STRING "IMESA 1496 accelerometer app: 0.4 "__DATE__" "__TIME__

//using internal oscillator
const uint32_t OscRateIn = 0;

STATIC const PINMUX_GRP_T pinmuxing[] = {
// I2C
	{(uint32_t)IOCON_PIO0_4,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_4 used for SCL */
	{(uint32_t)IOCON_PIO0_5,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_5 used for SDA */
// DIP SWITCH 1
	{(uint32_t)IOCON_PIO0_3,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO0_3: DIP1-1
	{(uint32_t)IOCON_PIO0_6,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO0_6: DIP1-2
	{(uint32_t)IOCON_PIO1_8,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO1_9: DIP1-3
	{(uint32_t)IOCON_PIO1_9,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO1_9: DIP1-4
	{(uint32_t)IOCON_PIO3_0,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_0: DIP1-5
	{(uint32_t)IOCON_PIO3_1,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_1: DIP1-6
	{(uint32_t)IOCON_PIO3_2,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_2: DIP1-7
	{(uint32_t)IOCON_PIO3_3,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_3: DIP1-8
// IN_ENC PIO1_0
	{(uint32_t)IOCON_PIO1_0,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
// UART PIO1_6..7
	{(uint32_t)IOCON_PIO1_6,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_6 used for RXD */
	{(uint32_t)IOCON_PIO1_7,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_7 used for TXD */
	{(uint32_t)IOCON_PIO1_5,  (IOCON_FUNC1)}, /* PIO1_5 used for CTS */
// DIP SWITCH 2: PIO2_0..3
	{(uint32_t)IOCON_PIO2_0,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
	{(uint32_t)IOCON_PIO2_1,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
	{(uint32_t)IOCON_PIO2_2,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
	{(uint32_t)IOCON_PIO2_3,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
// red LED
	{(uint32_t)IOCON_PIO3_4,  (IOCON_FUNC0)}, // PIO3_4: red LED
// green LED
	{(uint32_t)IOCON_PIO3_5,  (IOCON_FUNC0)}, // PIO3_5: green LED
// relais PIO1_11
	{(uint32_t)IOCON_PIO1_11,  (IOCON_FUNC0)},
// encoder inputs PIO2_4..11
	{(uint32_t)IOCON_PIO2_4,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_5,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_6,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_7,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_8,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_9,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_10,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_11,  (IOCON_FUNC0)},
#if 0
	{(uint32_t)IOCON_PIO0_1,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_1 used for CLKOUT */
	{(uint32_t)IOCON_PIO0_2,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_PULLUP)},/* PIO0_2 used for SSEL */
	{(uint32_t)IOCON_PIO0_3,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_3 used for USB_VBUS */
	{(uint32_t)IOCON_PIO0_4,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_4 used for SCL */
	{(uint32_t)IOCON_PIO0_5,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_5 used for SDA */
	{(uint32_t)IOCON_PIO0_6,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_6 used for USB_CONNECT */
	{(uint32_t)IOCON_PIO0_8,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_8 used for MISO */
	{(uint32_t)IOCON_PIO0_9,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_9 used for MOSI */
	{(uint32_t)IOCON_PIO0_11, (IOCON_FUNC2 | IOCON_RESERVED_BIT_6 | IOCON_ADMODE_EN      | IOCON_FILT_DIS)},   /* PIO0_11 used for AD0 */
	{(uint32_t)IOCON_PIO1_6,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_6 used for RXD */
	{(uint32_t)IOCON_PIO1_7,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_7 used for TXD */
	{(uint32_t)IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO2_11 used for SCK */
#endif
};

/* Sets up system pin muxing */
void Board_SetupMuxing(void)
{
	/* Enable IOCON clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

	Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));
}

typedef struct _type_handle_rom_entry
{
	unsigned int is_index_valid;
	unsigned int is_valid;
	uint8_t idx;
	type_rom_table_entry value;
}type_handle_rom_entry;



typedef enum
{
	enum_status_control_idle = 0,
	enum_status_control_check_speed_and_amplitude,
	enum_status_control_signal_alarm,
	enum_status_control_wait_signal_alarm,
	enum_status_control_wait_after_signal_alarm,
	enum_status_control_numof,
}enum_status_control;

#define def_num_shift_length_hist_amp 8
#define def_length_hist_amp (1 << def_num_shift_length_hist_amp)

#define numbit_dipsw1_crc 3
#define mask_dipsw1_crc (0xE0)
#define num_shiftright_dipsw1_crc 5

#define mask_dipsw1_value (0x1f)
#define num_shiftright_dipsw1_value 0

typedef struct _type_struct_control
{
	enum_status_control status;
	unsigned int enabled;
	type_handle_rom_entry rom_entry;
	uint8_t dipsw1;
	uint32_t table_entry_index_value;
	uint32_t table_entry_index_crc;
	uint32_t amplitude_um_X;
	uint32_t amplitude_um_Y;
	uint32_t idx_hist_amplitude;
	uint32_t hist_amplitude_um_X[def_length_hist_amp];
	uint32_t hist_amplitude_um_Y[def_length_hist_amp];
	int32_t amplitude_last_alarm_um_X;
	int32_t amplitude_last_alarm_um_Y;
	uint32_t pause_alarm_elapsed_ms, pause_validate_alarm_elapsed_ms;
	uint64_t base_alarm_ms, base_validate_alarm_ms;
	uint32_t maybe_alarm;
	uint32_t relais;

	uint32_t alarm_active;
	uint32_t alarm_mask;
	uint32_t alarm_num_X, alarm_num_Y;
}type_struct_control;

#ifdef def_enable_correlation

#define def_max_values_correlation 512
typedef struct _type_correlation
{
	unsigned int idx;
	int32_t xvalues[def_max_values_correlation];
	int32_t yvalues[def_max_values_correlation];
	int32_t correlation;
	uint64_t old_tick;
}type_correlation;
#endif

typedef struct _type_main_info
{
	type_encoder_main_info encoder;
	type_accelerometer_main_info accelerometer;
	uint64_t prev_update_freq_ms;
	uint64_t uptime_ms;
	type_struct_control control;
	uint32_t green_led;
	uint32_t cur_max_ampX, cur_max_ampY;
	uint64_t base_cur_max_amp;
	//uint64_t last_reset_minmaxacc_ms;
	int32_t differentialXmms2;
	int32_t differentialYmms2;
#ifdef def_enable_correlation
	type_correlation correlation;
#endif
	uint32_t cur_delta_maxmin;
	uint32_t from_watchdog_reset;

}type_main_info;

type_main_info main_info;

#define def_ANSI_reset 		"\x1b[0m"
#define def_ANSI_Black 		"\x1b[30m"
#define def_ANSI_Red 		"\x1b[31m"
#define def_ANSI_Green 		"\x1b[32m"
#define def_ANSI_Yellow 	"\x1b[33m"
#define def_ANSI_Blue 		"\x1b[34m"
#define def_ANSI_Magenta 	"\x1b[35m"
#define def_ANSI_Cyan 		"\x1b[36m"
#define def_ANSI_White 		"\x1b[37m"


#define def_ANSI_Background_Black	"\x1b[40m"
#define def_ANSI_Background_Red		"\x1b[41m"
#define def_ANSI_Background_Green	"\x1b[42m"
#define def_ANSI_Background_Yellow	"\x1b[43m"
#define def_ANSI_Background_Magenta	"\x1b[45m"
#define def_ANSI_Background_Cyan	"\x1b[46m"
#define def_ANSI_Background_White	"\x1b[47m"



#define def_ANSI_Bold 		"\x1b[1m"
#define def_ANSI_Underline 	"\x1b[4m"
#define def_ANSI_Reversed	"\x1b[7m"

#define def_ANSI_Home "\x1b[2J"
#define def_ANSI_ClearEndOfLine "\x1b[K"
#define def_ANSI_goto_line_column_format "\x1b[%u;%uf"
#define tx_uart_conststr(s) tx_uart((uint8_t*)s, strlen(s))



typedef enum
{
	enum_print_status_idle = 0,
	enum_print_status_init,
	enum_print_status_update,
	enum_print_status_numof
}enum_print_status;

typedef enum
{
	enum_print_menu_view = 0,
	enum_print_menu_numof
}enum_print_menu;


typedef struct _type_uptime
{
	uint32_t uptime_ms;
	uint32_t uptime_s;
	uint32_t uptime_min;
	uint32_t uptime_hours;
}type_uptime;
typedef struct _type_handle_print_menu
{
	enum_print_status status;
	enum_print_menu menu;
	type_uptime uptime;
	char line[129];
	unsigned int log_mode;

}type_handle_print_menu;
static type_handle_print_menu handle_print_menu;

static void vANSI_goto_line_column(char *s, size_t sizes, unsigned int line, unsigned int column)
{
	int n = snprintf(s, sizes, def_ANSI_goto_line_column_format, line, column);
	tx_uart((uint8_t*)s, (n <= sizes)? n : sizes);
}

typedef enum
{
	MENU_TITLE_ROW = 1,
	MENU_ENCODER_UPTIME_ROW,
	MENU_ENCODER_ROW,
	MENU_ACCELEROMETER_ROW_X,
	MENU_ACCELEROMETER_ROW_Y,
	MENU_DIPSW_ROW_TITLE,
	MENU_DIPSW_ROW_DATA,
	MENU_ROM_ROW_DATA,
	MENU_CONTROL_X_ROW_DATA,
	MENU_CONTROL_Y_ROW_DATA,


	MENU_CONTROL_UNUSED,
	MENU_CONTROL_HELP,
}enum_menu_rows;

#define MENU_TITLE_COLUMN 1
#define MENU_ENCODER_COLUMN 1
#define MENU_ENCODER_STATS_COLUMN 14
#define MENU_ACCELEROMETER_COLUMN_DATA 1
#define MENU_ACCELEROMETER_COLUMN_STATS 14
#define MENU_DIPSW_COLUMN_DATA 1
#define MENU_DIPSW_COLUMN_DATA2 16
#define MENU_ROM_COLUMN_DATA 1
#define MENU_ENCODER_UPTIME_COL 1
#define MENU_CONTROL_COLUMN_DATA 1

void check_rx_log_mode(void)
{
	uint8_t c = 0;
	unsigned int nrx = rx_uart(&c, 1);
	if (nrx)
	{
		handle_print_menu.log_mode = 0;
		tx_uart_conststr("\r\n\r\n************************************************\r\n");
		tx_uart_conststr(def_ANSI_Home);
		tx_uart_conststr(def_ANSI_reset);
	}
}

void check_rx_chars(void)
{
	uint8_t c = 0;
	unsigned int nrx = rx_uart(&c, 1);
	if (nrx)
	{
		c = toupper(c);
		if (c == 'R')
		{
			handle_print_menu.status = enum_print_status_idle;
		}
		else if (c == 'C')
		{
			reset_encoder_stats();
			//reset_accelerometer_stats();
		}
		else if (c == 'X')
		{
			while(1)
			{

			}
		}
		else if (c == 'L')
		{
			handle_print_menu.log_mode = 1;
			tx_uart_conststr("\r\n\r\n============================================\r\n");
		}
		else if (c == 'W')
		{
			Chip_WWDT_Feed(LPC_WWDT);
			handle_print_menu.status = enum_print_status_idle;

			tx_uart_conststr(def_ANSI_Home);
			tx_uart_conststr(def_ANSI_reset);
			tx_uart_conststr(def_ANSI_Background_White);
			tx_uart_conststr(def_ANSI_Black);
			type_handle_rom_entry *p = &main_info.control.rom_entry;


			vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),1, 1);
			int n_chars;
			n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "ROM TABLE index:%-3u (%-12s)   speed[rpm]:%-4u   speed[rpm]:%-4u   Xamp[um]:%-6u   Yamp[um]:%-6u"
					,p->idx
					,p->is_valid ? "OK VALID": "**INVALID**"
					,p->value.threshold_rpm_X
					,p->value.threshold_rpm_Y
					,p->value.threshold_amplitude_X_um
					,p->value.threshold_amplitude_Y_um
					);
			if (n_chars > sizeof(handle_print_menu.line))
			{
				n_chars = sizeof(handle_print_menu.line);
			}
			tx_uart((uint8_t*)handle_print_menu.line, n_chars);
			tx_uart_conststr(def_ANSI_ClearEndOfLine);


			vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), 2, 1);
			n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "Input values: Esc to abort, CR to confirm");
			if (n_chars > sizeof(handle_print_menu.line))
			{
				n_chars = sizeof(handle_print_menu.line);
			}
			tx_uart((uint8_t*)handle_print_menu.line, n_chars);
			tx_uart_conststr(def_ANSI_ClearEndOfLine);

			vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), 3, 1);
			n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "SpeedX[rpm] ");
			if (n_chars > sizeof(handle_print_menu.line))
			{
				n_chars = sizeof(handle_print_menu.line);
			}
			tx_uart((uint8_t*)handle_print_menu.line, n_chars);
			tx_uart_conststr(def_ANSI_ClearEndOfLine);

			unsigned int is_input = 1;
			unsigned int num_values = 0;
			unsigned int is_abort = 0;
			char c_my_str[128];
			unsigned int idx_char = 0;
			unsigned int rpmX = 0;
			unsigned int rpmY = 0;
			long th_um[2];
			memset(th_um, 0, sizeof(th_um));
			while(is_input && ! is_abort)
			{
				Chip_WWDT_Feed(LPC_WWDT);
				unsigned int nrx = rx_uart(&c, 1);
				if (nrx)
				{
					switch(c)
					{
						case 0x1b:
						{
							is_abort = 1;
							break;
						}
						case 0x0d:
						{
							switch(num_values)
							{
								case 0:
								{
									rpmX = atoi(c_my_str);
									idx_char = 0;
									break;
								}
								case 1:
								{
									rpmY = atoi(c_my_str);
									idx_char = 0;
									break;
								}
								case 2:
								{
									th_um[0] = atol(c_my_str);
									idx_char = 0;
									break;
								}
								case 3:
								{
									th_um[1] = atol(c_my_str);
									idx_char = 0;
									break;
								}
							}
							if (++num_values >= 4)
							{
								is_input = 0;
							}
							vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), 3 + num_values, 1);
							char *pc = "";
							switch(num_values)
							{
								case 1:
								default:
								{
									pc = "SpeedY[rpm] ";
									break;
								}
								case 2:
								{
									pc = "Xth[um] ";
									break;
								}
								case 3:
								{
									pc = "Yth[um] ";
									break;
								}
							}
							int n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "%s", pc);
							if (n_chars > sizeof(handle_print_menu.line))
							{
								n_chars = sizeof(handle_print_menu.line);
							}
							tx_uart((uint8_t*)handle_print_menu.line, n_chars);
							tx_uart_conststr(def_ANSI_ClearEndOfLine);

							break;
						}
						default:
						{
							if (idx_char + 2 < sizeof(c_my_str))
							{
								c_my_str[idx_char++] = c;
								tx_uart(&c, 1);
							}
							break;
						}
					}
				}
			}
			if (!is_abort)
			{
				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), 7, 1);
				n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "SpeedX[rpm] = %u, SpeedY[rpm] = %u, Xth[um]=%lu, Yth[um]=%lu: CONFIRM? (y/n)", rpmX, rpmY, th_um[0], th_um[1]);
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);

				unsigned int is_input = 1;
				unsigned int is_abort = 0;

				while(is_input && ! is_abort)
				{
					Chip_WWDT_Feed(LPC_WWDT);
					unsigned int nrx = rx_uart(&c, 1);
					if (nrx)
					{
						switch(c)
						{
							case 'y':
							case 'Y':
							{
								is_input = 0;
								break;
							}
							default:
							{
								is_abort = 1;
								break;
							}
						}
					}
				}
				if (is_abort)
				{
					tx_uart_conststr(def_ANSI_Background_Red);
					tx_uart_conststr(def_ANSI_White);
					vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), 8, 1);
					n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "*** ABORTED !!!***");
					if (n_chars > sizeof(handle_print_menu.line))
					{
						n_chars = sizeof(handle_print_menu.line);
					}
					tx_uart((uint8_t*)handle_print_menu.line, n_chars);
					tx_uart_conststr(def_ANSI_ClearEndOfLine);
					for (n_chars = 0; n_chars < 10; n_chars++)
					{
						Chip_WWDT_Feed(LPC_WWDT);
						delay_ms(200);
					}
				}
				else
				{
					type_rom_table_entry e;
					e.threshold_rpm_X = rpmX;
					e.threshold_rpm_Y = rpmY;
					e.threshold_amplitude_X_um= th_um[0];
					e.threshold_amplitude_Y_um= th_um[1];

					unsigned int is_OK = is_OK_write_rom_table_entry(main_info.control.table_entry_index_value, &e);
					if (!is_OK)
					{
						tx_uart_conststr(def_ANSI_Background_Red);
						tx_uart_conststr(def_ANSI_White);
					}
					else
					{
						tx_uart_conststr(def_ANSI_Background_Green);
						tx_uart_conststr(def_ANSI_Black);
					}
					vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), 8, 1);
					n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "%s", is_OK ? "       DATA WRITTEN OK": "*******ERROR WRITING DATA");
					if (n_chars > sizeof(handle_print_menu.line))
					{
						n_chars = sizeof(handle_print_menu.line);
					}
					tx_uart((uint8_t*)handle_print_menu.line, n_chars);
					tx_uart_conststr(def_ANSI_ClearEndOfLine);
					for (n_chars = 0; n_chars < 10; n_chars++)
					{
						Chip_WWDT_Feed(LPC_WWDT);
						delay_ms(150);
					}
					main_info.control.rom_entry.is_valid = 0;
				}

			}
		}
	}
	//Chip_UART_TXDisable();
}


void print_info(void)
{

	uint64_t now_ms = get_tick_count();
	int32_t delta_ms = now_ms - main_info.prev_update_freq_ms;
	if (handle_print_menu.log_mode)
	{
		if (delta_ms < 100)
		{
			return;
		}
	}
	else
	{
		if (delta_ms < 300)
		{
			return;
		}
	}
/*	if (!ui_acc_stats_valid())
	{
		return;
	}*/

	{
		main_info.green_led = !main_info.green_led;
		//set_led(enum_LED_green, main_info.green_led);
	}
	main_info.prev_update_freq_ms = now_ms;
	main_info.uptime_ms = now_ms;

#ifndef __PRODUCTION_NO_EEPROM_NO_SERIAL__
	if (handle_print_menu.log_mode)
	{
		check_rx_log_mode();
		if (!handle_print_menu.log_mode)
		{
			return;
		}
		type_struct_control *p_control = &main_info.control;

		uint32_t rpm = (main_info.encoder.freq_mHz * 60) / 1000 ;
		int n_chars = 0;
		n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "%u %i %u %i %u\r\n"
				,rpm, main_info.differentialXmms2, p_control->amplitude_um_X, main_info.differentialYmms2, p_control->amplitude_um_Y
				);
		if (n_chars > sizeof(handle_print_menu.line))
		{
			n_chars = sizeof(handle_print_menu.line);
		}
		tx_uart((uint8_t*)handle_print_menu.line, n_chars);
		return;
	}
#endif

#ifndef __PRODUCTION_NO_EEPROM_NO_SERIAL__
//#error compiling to eeprom+serial board!
	check_rx_chars();
#endif


	switch(handle_print_menu.status)
	{
		case enum_print_status_idle:
		default:
		{
			tx_uart_conststr(def_ANSI_Home);
			tx_uart_conststr(def_ANSI_reset);
			handle_print_menu.status = enum_print_status_init;
			break;
		}
		case enum_print_status_init:
		{
			handle_print_menu.status = enum_print_status_update;
			switch(handle_print_menu.menu)
			{
				case enum_print_menu_view:
				default:
				{
					tx_uart_conststr(def_ANSI_Home);
					tx_uart_conststr(def_ANSI_reset);
					vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line), MENU_TITLE_ROW, MENU_TITLE_COLUMN);
					tx_uart_conststr(DEF_FIRMWARE_VERSION_STRING);
					tx_uart_conststr(def_ANSI_reset);
					break;
				}
			}
			break;
		}
		case enum_print_status_update:
		{
			{
				tx_uart_conststr(def_ANSI_reset);
				tx_uart_conststr(def_ANSI_Background_White);
				tx_uart_conststr(def_ANSI_Black);

				type_uptime *p = &handle_print_menu.uptime;
				p->uptime_ms += delta_ms;
				while(p->uptime_ms >= 1000)
				{
					p->uptime_s++;
					p->uptime_ms -= 1000;
				}
				while(p->uptime_s >= 60)
				{
					p->uptime_min++;
					p->uptime_s -= 60;
				}
				while(p->uptime_min >= 60)
				{
					p->uptime_hours++;
					p->uptime_min -= 60;
				}

				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_ENCODER_UPTIME_ROW, MENU_ENCODER_UPTIME_COL);
				int n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "uptime: %uh %02umin %02us %03ums %s"
						,p->uptime_hours
						,p->uptime_min
						,p->uptime_s
						,p->uptime_ms
						,main_info.from_watchdog_reset ? "WATCHDOG!":" "
						);
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);
			}
			{
				tx_uart_conststr(def_ANSI_reset);
				tx_uart_conststr(def_ANSI_Background_White);
				tx_uart_conststr(def_ANSI_Black);


				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_ENCODER_ROW, MENU_ENCODER_COLUMN);
				int n_chars = 0;
				type_encoder_main_info *p = &main_info.encoder;
				n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "Speed          current[rpm  ]: %-5u min[rpm  ]: %-5u MAX[rpm  ]: %-5u #OK:%-9u #ERR1:%-4u #ERR2:%-4u"
						,(p->freq_mHz * 60) / 1000
						,p->min_freq_Hz * 60
						,p->max_freq_Hz * 60
						,p->num_updates
						,p->num_err_too_high_input_freq
						,p->num_err_get_encoder
						);
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);

			}

			{
				tx_uart_conststr(def_ANSI_reset);
				tx_uart_conststr(def_ANSI_Background_White);
				tx_uart_conststr(def_ANSI_Black);
				{
					type_accelerometer_main_info *p = &main_info.accelerometer;
					int n_chars = 0;

					vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_ACCELEROMETER_ROW_X, MENU_ACCELEROMETER_COLUMN_DATA);
					n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "Acceleration X current[mm/s2]:%-+6i min[mm/s2]:%-+6i MAX[mm/s2]:%-+6i"
							,main_info.differentialXmms2
							,p->min_acc_mms2[0]
							,p->max_acc_mms2[0]
							);
					if (n_chars > sizeof(handle_print_menu.line))
					{
						n_chars = sizeof(handle_print_menu.line);
					}
					tx_uart((uint8_t*)handle_print_menu.line, n_chars);
					tx_uart_conststr(def_ANSI_ClearEndOfLine);

					vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_ACCELEROMETER_ROW_Y, MENU_ACCELEROMETER_COLUMN_DATA);
					n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "             Y current[mm/s2]:%-+6i min[mm/s2]:%-+6i MAX[mm/s2]:%-+6i #OK:%-9u #ERR1:%-4u #ERR2:%-4u"
							,main_info.differentialYmms2
							,p->min_acc_mms2[1]
							,p->max_acc_mms2[1]
							,p->num_readOK
							,p->numerr_send1
							,p->numerr_send2
							);
					if (n_chars > sizeof(handle_print_menu.line))
					{
						n_chars = sizeof(handle_print_menu.line);
					}
					tx_uart((uint8_t*)handle_print_menu.line, n_chars);
					tx_uart_conststr(def_ANSI_ClearEndOfLine);

				}
			}

			{
				tx_uart_conststr(def_ANSI_reset);
				tx_uart_conststr(def_ANSI_Background_White);
				tx_uart_conststr(def_ANSI_Black);
				uint8_t dipsw1 = main_info.control.dipsw1;

				int n_chars = 0;

				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_DIPSW_ROW_TITLE, MENU_DIPSW_COLUMN_DATA);
				n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "Dip-switch     12345678");
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);
				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_DIPSW_ROW_DATA, MENU_DIPSW_COLUMN_DATA2);
				n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "%c%c%c%c%c%c%c%c"
						,(dipsw1 & 0x1) ? '1' : '0'
						,(dipsw1 & 0x2) ? '1' : '0'
						,(dipsw1 & 0x4) ? '1' : '0'
						,(dipsw1 & 0x8) ? '1' : '0'
						,(dipsw1 & 0x10) ? '1' : '0'
						,(dipsw1 & 0x20) ? '1' : '0'
						,(dipsw1 & 0x40) ? '1' : '0'
						,(dipsw1 & 0x80) ? '1' : '0'
						);
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);
			}
			{
				tx_uart_conststr(def_ANSI_reset);
				tx_uart_conststr(def_ANSI_Background_White);
				tx_uart_conststr(def_ANSI_Black);
				type_handle_rom_entry *p = &main_info.control.rom_entry;


				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_ROM_ROW_DATA, MENU_ROM_COLUMN_DATA);
				int n_chars;
				if (!p->is_index_valid)
				{
					if (main_info.control.table_entry_index_value == 0)
					{
						n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "%s", "ERROR: INVALID INDEX 0 SELECTED");
					}
					else
					{
						uint32_t ok_crc = calc_crc_3bit_of_6bit_number( main_info.control.table_entry_index_value);
						n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "ERROR: INDEX %u SELECTED, DIP 678: %c%c%c (WRONG! MUST BE %c%c%c)"
								, main_info.control.table_entry_index_value
								, (main_info.control.table_entry_index_crc & 1) ? '1' : '0'
								, (main_info.control.table_entry_index_crc & 2) ? '1' : '0'
								, (main_info.control.table_entry_index_crc & 4) ? '1' : '0'
								, (ok_crc & 1) ? '1' : '0'
								, (ok_crc & 2) ? '1' : '0'
								, (ok_crc & 4) ? '1' : '0'
						);
					}
				}
				else
				{
					n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "ROM TABLE %s index:%-3u (%-12s)   speedX[rpm]:%-4u   speedY[rpm]:%-4u   Xamp[um]:%-6u   Yamp[um]:%-6u"
							,p_get_rom_table_last_found_label()
							,p->idx
							,p->is_valid ? "OK VALID": "**INVALID**"
							,p->value.threshold_rpm_X
							,p->value.threshold_rpm_Y
							,p->value.threshold_amplitude_X_um
							,p->value.threshold_amplitude_Y_um
							//,main_info.correlation.correlation
							);
				}
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);
			}
			{
				uint32_t alarm_is_active = 0;
				tx_uart_conststr(def_ANSI_reset);
				type_struct_control *p_control = &main_info.control;
				if (p_control->alarm_active && (p_control->alarm_mask&1))
				{
					tx_uart_conststr(def_ANSI_Background_Red);
					tx_uart_conststr(def_ANSI_White);
					alarm_is_active = 1;
				}
				else
				{
					tx_uart_conststr(def_ANSI_Background_Green);
					tx_uart_conststr(def_ANSI_Black);
					alarm_is_active = 0;
				}


				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_CONTROL_X_ROW_DATA, MENU_CONTROL_COLUMN_DATA);
				int n_chars;
				n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "X ALARM CONTROL: %s  Xamp[um]:%-6u last alarm Xamp[um]:%-6u #alarm:%-5u MAX Xamp[um]:%-6u"
						,alarm_is_active ? "*** ALARM ACTIVE ***": "  alarm not active  "
						,p_control->amplitude_um_X
						,p_control->amplitude_last_alarm_um_X
						,p_control->alarm_num_X
						,main_info.cur_max_ampX
						);
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);

				if (p_control->alarm_active && (p_control->alarm_mask&2))
				{
					tx_uart_conststr(def_ANSI_Background_Red);
					tx_uart_conststr(def_ANSI_White);
					alarm_is_active = 1;
				}
				else
				{
					tx_uart_conststr(def_ANSI_Background_Green);
					tx_uart_conststr(def_ANSI_Black);
					alarm_is_active = 0;
				}
				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_CONTROL_Y_ROW_DATA, MENU_CONTROL_COLUMN_DATA);
				n_chars = snprintf(handle_print_menu.line, sizeof(handle_print_menu.line), "Y ALARM CONTROL: %s  Yamp[um]:%-6u last alarm Yamp[um]:%-6u #alarm:%-5u MAX Yamp[um]:%-6u"
						,alarm_is_active ? "*** ALARM ACTIVE ***": "  alarm not active  "
						,p_control->amplitude_um_Y
						,p_control->amplitude_last_alarm_um_Y
						,p_control->alarm_num_Y
						,main_info.cur_max_ampY
						);
				if (n_chars > sizeof(handle_print_menu.line))
				{
					n_chars = sizeof(handle_print_menu.line);
				}
				tx_uart((uint8_t*)handle_print_menu.line, n_chars);
				tx_uart_conststr(def_ANSI_ClearEndOfLine);

			}
			{
				tx_uart_conststr(def_ANSI_reset);
				tx_uart_conststr(def_ANSI_Reversed);
				tx_uart_conststr(def_ANSI_Yellow);
				vANSI_goto_line_column(handle_print_menu.line, sizeof(handle_print_menu.line),MENU_CONTROL_HELP, 1);
				tx_uart_conststr("HELP:   R refresh    W write new data     C clear stats     L toggle log");
				tx_uart_conststr(def_ANSI_ClearEndOfLine);

			}

			break;
		}
	}
#if 0
	tx_uart_conststr("\x1b[2J");
	tx_uart_conststr(def_ANSI_reset);
	tx_uart_conststr("\x1b[1;1f");
	tx_uart_conststr(def_ANSI_Red);
	tx_uart_conststr(def_ANSI_Bold);
	tx_uart_conststr("IMESA 1486 accelerometer app");
	tx_uart_conststr(def_ANSI_reset);
	tx_uart_conststr("\x1b[2;1f");
	static int index_loop;
	char line[64];
	snprintf(line, sizeof(line), "idx=%u", ++index_loop);
	tx_uart_conststr(line);
	tx_uart_conststr("\x1b[3;1f");
#endif
#if 0
	{
		main_info.prev_update_freq_ms = now_ms;

		char status_encoder[128];
		int n_chars = 0;
		type_encoder_main_info *p = &main_info.encoder;
		n_chars = snprintf(status_encoder, sizeof(status_encoder), "freq [Hz]: %u (%s), nloop: %u, err_hi_freq: %u, err_get_enc: %u; "
				,p->freq_Hz
				,p->valid ? "OK":"invalid"
				,p->num_updates
				,p->num_err_too_high_input_freq
				,p->num_err_get_encoder
				);
		if (n_chars > sizeof(status_encoder))
		{
			n_chars = sizeof(status_encoder);
		}
		tx_uart((uint8_t*)status_encoder, n_chars);
	}

	{
		type_accelerometer_main_info *p = &main_info.accelerometer;
		int n_chars = 0;
		char status_accelerometer[128];
		n_chars = snprintf(status_accelerometer, sizeof(status_accelerometer), "acc_X: %i [mm/s2], acc_Y: %i [mm/s2], nreadOK: %u, nreadERR1: %u, nreadERR2: %u; "
				,p->acc_mms2[0]
				,p->acc_mms2[1]
				,p->num_readOK
				,p->numerr_send1
				,p->numerr_send2
				);
		if (n_chars > sizeof(status_accelerometer))
		{
			n_chars = sizeof(status_accelerometer);
		}
		tx_uart((uint8_t*)status_accelerometer, n_chars);
	}
	tx_uart((uint8_t*)"\r\n", 2);
#endif
}

static int uint32_tcmp(const void *pa, const void *pb)
{
	uint32_t ia = *(int32_t*)pa;
	uint32_t ib = *(int32_t*)pb;
	if (ia == ib)
	{
		return 0;
	}
	if (ia > ib)
	{
		return 1;
	}
	return -1;
}

void do_control(void)
{
	uint64_t now_ms = get_tick_count();
#if 0
	if (now_ms - main_info.last_reset_minmaxacc_ms >= 1000)
	{
		main_info.last_reset_minmaxacc_ms = now_ms;
		reset_accelerometer_stats();
	}
	main_info.cur_delta_maxmin = now_ms - main_info.last_reset_minmaxacc_ms;
#endif

#ifdef def_enable_correlation
	{
		type_correlation * p = &main_info.correlation;
		if (now_ms != p->old_tick)
		{
			p->old_tick = now_ms;
			p->xvalues[p->idx] = main_info.accelerometer.acc_mms2[0];
			p->yvalues[p->idx] = main_info.accelerometer.acc_mms2[1];
			if (++p->idx >= def_max_values_correlation)
			{
				p->idx = 0;
				unsigned int i;
				int32_t xm = 0;
				int32_t ym = 0;
				for (i = 0; i < def_max_values_correlation; i++)
				{
					xm += p->xvalues[i];
					ym += p->yvalues[i];
				}
				xm /= def_max_values_correlation;
				ym /= def_max_values_correlation;
				int64_t sxy = 0;
				int64_t sx2 = 0;
				int64_t sy2 = 0;
				for (i = 0; i < def_max_values_correlation; i++)
				{
					p->xvalues[i] -= xm;
					p->yvalues[i] -= ym;
					sxy += (int64_t)p->xvalues[i] * (int64_t)p->yvalues[i];
					sx2 += (int64_t)p->xvalues[i] * (int64_t)p->xvalues[i];
					sy2 += (int64_t)p->yvalues[i] * (int64_t)p->yvalues[i];
				}
				p->correlation = ((sxy * 1000) / sx2) * ((sxy * 1000) / sy2);
			}
		}
	}
#endif

	type_struct_control *p_control = &main_info.control;
	type_handle_rom_entry *p_rom_table = &p_control->rom_entry;
	{
		main_info.control.dipsw1 = read_dipswitch1();
		main_info.control.table_entry_index_value = (main_info.control.dipsw1 & mask_dipsw1_value) >> num_shiftright_dipsw1_value;
		main_info.control.table_entry_index_crc = (main_info.control.dipsw1 & mask_dipsw1_crc) >> num_shiftright_dipsw1_crc;
		if ((main_info.control.table_entry_index_value == 0) || !is_OK_crc_3bit_of_6bit_number(main_info.control.table_entry_index_value, main_info.control.table_entry_index_crc))
		{
			accum_current_alarm(enum_alarm_invalid_dipsw);
			p_rom_table->is_index_valid = 0;
		}
		else
		{
			p_rom_table->is_index_valid = 1;
		}
	}
	if (p_rom_table->is_index_valid)
	{
		if (!p_rom_table->is_valid)
		{
			p_rom_table->is_valid = is_OK_get_rom_table_entry(p_rom_table->idx, &p_rom_table->value);
		}
		if (p_rom_table->idx != main_info.control.table_entry_index_value)
		{
			p_rom_table->idx = main_info.control.table_entry_index_value;
			p_rom_table->is_valid = is_OK_get_rom_table_entry(p_rom_table->idx, &p_rom_table->value);
		}
	}

	if (!p_rom_table->is_valid || !p_rom_table->is_index_valid)
	{
		accum_current_alarm(enum_alarm_invalid_dipsw);
		p_control->status = enum_status_control_idle;
	}

	switch(p_control->status)
	{
		case enum_status_control_idle:
		default:
		{
			p_control->alarm_active = 0;
			p_control->relais = 0;
			if (p_rom_table->is_valid && ui_acc_stats_valid())
			{
				p_control->status = enum_status_control_check_speed_and_amplitude;
				p_control->pause_validate_alarm_elapsed_ms = 0;
				p_control->base_validate_alarm_ms = get_tick_count();
			}
			break;
		}
		case enum_status_control_check_speed_and_amplitude:
		{
			p_control->alarm_active = 0;
			p_control->relais = 0;
			uint32_t now_alarm = 0;
			uint32_t w_squared = 0;
			uint32_t rpm = (main_info.encoder.freq_mHz * 60) / 1000 ;
			{
				// 804/256 approximates pi
				uint64_t w = (2 * 804 * main_info.encoder.freq_mHz);
				w_squared = ((w * w) / (1000 * 1000)) >> 16;
			}
			uint64_t u64 = 0;
			// u64 = abs(main_info.accelerometer.acc_mms2[0]);
			u64 = abs(main_info.accelerometer.max_acc_mms2[0] - main_info.accelerometer.min_acc_mms2[0]);
			main_info.differentialXmms2 = u64;
			p_control->amplitude_um_X = (u64 * 1000) / (w_squared);
			if (p_control->amplitude_um_X > p_rom_table->value.threshold_amplitude_X_um)
			{
				if (rpm >= p_rom_table->value.threshold_rpm_X)
				{
					now_alarm |= 1;
				}
			}
			u64 = abs(main_info.accelerometer.max_acc_mms2[1] - main_info.accelerometer.min_acc_mms2[1]);
			main_info.differentialYmms2 = u64;
			p_control->amplitude_um_Y = (u64 * 1000) / (w_squared);
			if (p_control->amplitude_um_Y > p_rom_table->value.threshold_amplitude_Y_um)
			{
				if (rpm >= p_rom_table->value.threshold_rpm_Y)
				{
					now_alarm |= 2;
				}
			}
			//uint64_t now = get_tick_count();

			uint32_t idx_hist_amplitude = p_control->idx_hist_amplitude;
			p_control->hist_amplitude_um_X[idx_hist_amplitude] = p_control->amplitude_um_X;
			p_control->hist_amplitude_um_Y[idx_hist_amplitude] = p_control->amplitude_um_Y;

			qsort(&p_control->hist_amplitude_um_X[0], def_length_hist_amp, sizeof(p_control->hist_amplitude_um_X[0]), uint32_tcmp);
			qsort(&p_control->hist_amplitude_um_Y[0], def_length_hist_amp, sizeof(p_control->hist_amplitude_um_Y[0]), uint32_tcmp);
			main_info.cur_max_ampX = p_control->hist_amplitude_um_X[def_length_hist_amp - 1];
			main_info.cur_max_ampY = p_control->hist_amplitude_um_Y[def_length_hist_amp - 1];

			if (now_alarm)
			{
				int32_t delta_ms = now_ms - p_control->base_validate_alarm_ms;
				p_control->base_validate_alarm_ms = now_ms;
				p_control->pause_validate_alarm_elapsed_ms += delta_ms;
				// no filter on the alarm !
				if (p_control->pause_validate_alarm_elapsed_ms >= 0)
				{
					switch (now_alarm & 3)
					{
						case 3:
						{
							accum_current_alarm(enum_alarm_balance_XY);
							break;
						}
						case 2:
						{
							accum_current_alarm(enum_alarm_balance_Y);
							break;
						}
						case 1:
						{
							accum_current_alarm(enum_alarm_balance_X);
							break;
						}
						default:
						{
							break;
						}
					}

					if (now_alarm & 1)
					{
						p_control->amplitude_last_alarm_um_X = p_control->amplitude_um_X;
						p_control->alarm_num_X++;
					}
					if (now_alarm & 2)
					{
						p_control->amplitude_last_alarm_um_Y = p_control->amplitude_um_Y;
						p_control->alarm_num_Y++;
					}
					p_control->alarm_mask = now_alarm;
					p_control->status = enum_status_control_signal_alarm;
				}
			}
			else
			{
				p_control->pause_validate_alarm_elapsed_ms = 0;
				p_control->base_validate_alarm_ms = get_tick_count();
			}
			break;
		}
		case enum_status_control_signal_alarm:
		{
			p_control->alarm_active = 1;
			p_control->relais = 1;
			//set_led(enum_LED_red, 1);
			p_control->pause_alarm_elapsed_ms = 0;
			p_control->base_alarm_ms = get_tick_count();
			p_control->status = enum_status_control_wait_signal_alarm;
			break;
		}
		case enum_status_control_wait_signal_alarm:
		{
			p_control->alarm_active = 1;
			p_control->relais = 1;
			int32_t delta_ms = now_ms - p_control->base_alarm_ms;
			p_control->base_alarm_ms = now_ms;
			p_control->pause_alarm_elapsed_ms += delta_ms;
			if (p_control->pause_alarm_elapsed_ms >= 10)
			{
				p_control->pause_alarm_elapsed_ms = 0;
				p_control->base_alarm_ms = get_tick_count();
				p_control->status = enum_status_control_wait_after_signal_alarm;
			}
			break;
		}
		case enum_status_control_wait_after_signal_alarm:
		{
			p_control->alarm_active = 1;
			p_control->relais = 0;
			int32_t delta_ms = now_ms - p_control->base_alarm_ms;
			p_control->base_alarm_ms = now_ms;
			p_control->pause_alarm_elapsed_ms += delta_ms;
			if (p_control->pause_alarm_elapsed_ms >= 10)
			{
				p_control->status = enum_status_control_check_speed_and_amplitude;
			}
			break;
		}
	}
}


// chiave 4 bytes
// velocità minima rpm 4 bytes
// soglia X 4 bytes (accX[m/s2] * 1000 * 1000)/((2*pi*freq)^2)
// soglia Y 4 bytes (accY[m/s2] * 1000 * 1000)/((2*pi*freq)^2)
// tot 16 bytes
// 4096/16 = 256 entries
int main(void)
{
	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_SetupMuxing();

// modules initializations
	{
		init_uart();
		tx_uart((uint8_t*)"hello\r\n", 7);
	}
	led_init();
	init_i2c();
	encoder_module_init();
	accelerometer_module_init();
	system_tick_module_init();
	relais_init();
	module_init_alarms();


	memset(&main_info, 0, sizeof(main_info));


	encoder_module_register_callbacks();

	//test_flash();
	//isOK_test_crc3();

	led_test();

	{
		typedef enum
		{
			enum_run_status_idle = 0,
			enum_run_status_init,
			enum_run_status_run,

			enum_run_status_numof
		}enum_run_status;
		enum_run_status s = enum_run_status_idle;

		while(1)
		{
			switch(s)
			{
				case enum_run_status_idle:
				default:
				{
					s = enum_run_status_init;
					break;
				}
				case enum_run_status_init:
				{
					start_handle_alarms();
					accelerometer_module_init();
					s = enum_run_status_run;
					// test_accelerometer();
#if 0
					void test_eeprom(void);
					test_eeprom();

					void test_table(void);
					test_table();
#endif
					uint32_t wd_st = Chip_WWDT_GetStatus(LPC_WWDT);
					if (wd_st & WWDT_WDMOD_WDTOF)
					{
						init_new_alarm(enum_alarm_SW);
						main_info.from_watchdog_reset = 1;
					}
#if def_watchdog_enabled
					Chip_Clock_SetWDTClockSource(SYSCTL_WDTCLKSRC_IRC, 64);
					Chip_WWDT_Init(LPC_WWDT);
					Chip_WWDT_SetTimeOut(LPC_WWDT, 30000 * 2);
					Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDEN | WWDT_WDMOD_WDRESET);
					Chip_WWDT_Feed(LPC_WWDT);
#endif
					break;
				}
				case enum_run_status_run:
				{
					Chip_WWDT_Feed(LPC_WWDT);
					reset_current_alarm();
					{
						if (accelerometer_module_handle_run())
						{
							accum_current_alarm(enum_alarm_HW);
						}
						refresh_accelerometer_info(&main_info.accelerometer);
						refresh_encoder_info(&main_info.encoder);
						do_control();
					}
					set_current_alarm();
					Chip_WWDT_Feed(LPC_WWDT);
					vhandle_alarms();
					print_info();
					break;
				}
			}
		}
	}



	/* Should not run to here */
	return 0;
}
