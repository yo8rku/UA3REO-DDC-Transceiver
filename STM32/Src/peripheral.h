#ifndef Peripheral_h
#define Peripheral_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define MCP3008_THRESHOLD 100

typedef struct
{
	//MCP3008 - 1
	bool key_ab;
	bool key_ab_prev;
	uint32_t key_ab_starttime;
	bool key_ab_afterhold;
	bool key_tune;
	bool key_tune_prev;
	bool key_preatt;
	bool key_preatt_prev;
	uint32_t key_preatt_starttime;
	bool key_preatt_afterhold;
	bool key_fast;
	bool key_fast_prev;
	bool key_modep;
	bool key_modep_prev;
	bool key_moden;
	bool key_moden_prev;
	bool key_bandp;
	bool key_bandp_prev;
	bool key_bandn;
	bool key_bandn_prev;
	//MCP3008 - 2
	bool key_agc;
	bool key_agc_prev;
	uint32_t key_agc_starttime;
	bool key_agc_afterhold;
	bool key_dnr;
	bool key_dnr_prev;
	uint32_t key_dnr_starttime;
	bool key_dnr_afterhold;
	bool key_a_set_b;
	bool key_a_set_b_prev;
	uint32_t key_a_set_b_starttime;
	bool key_a_set_b_afterhold;
	bool key_notch;
	bool key_notch_prev;
	uint32_t key_notch_starttime;
	bool key_notch_afterhold;
	bool key_clar;
	bool key_clar_prev;
	uint32_t key_clar_starttime;
	bool key_clar_afterhold;
	bool key_menu;
	bool key_menu_prev;
	uint32_t key_menu_starttime;
	bool key_menu_afterhold;
	uint16_t key_afgain; //10-bit
	uint16_t key_shift;	 //10-bit
						 //
} PERIPH_FrontPanel_Type;

extern void PERIPH_ENCODER_checkRotate(void);
extern void PERIPH_ENCODER2_checkRotate(void);
extern void PERIPH_ENCODER2_checkSwitch(void);
extern void PERIPH_RF_UNIT_UpdateState(bool clean);
extern void PERIPH_ProcessFrontPanel(void);
extern void PERIPH_ProcessSWRMeter(void);
extern void PERIPH_InitFrontPanel(void);
extern bool PERIPH_SPI_Transmit(uint8_t *out_data, uint8_t *in_data, uint8_t count, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN, bool hold_cs);

volatile extern PERIPH_FrontPanel_Type PERIPH_FrontPanel;
volatile extern bool PERIPH_SPI_process;

extern RTC_HandleTypeDef hrtc;

#endif
