#ifndef LCD_h
#define LCD_h

#include "stm32f4xx_hal.h"
#include "trx_manager.h"
#include "LCD/lcd_driver.h"

#define MENU_FREQ_HZ 1
#define MENU_FREQ_KHZ 2
#define MENU_FREQ_MHZ 3
#define MENU_FREQ_COUNT 3

#define MENU_MAIN_EXIT 1
#define MENU_MAIN_VOLUME 2
#define MENU_MAIN_AGCSPEED 3
#define MENU_MAIN_RF_POWER 4
#define MENU_MAIN_FM_SQL 5
#define MENU_MAIN_RF_GAIN 6

#define COLOR_BUTTON_ACTIVE COLOR_DGREEN
#define COLOR_BUTTON_INACTIVE rgb888torgb565(198, 202, 206)
#define COLOR_BUTTON_MENU rgb888torgb565(224, 145, 8)
#define COLOR_BUTTON_TEXT rgb888torgb565(0, 27, 51)

#define METER_WIDTH 172

typedef struct {
	bool Background;
	bool MainMenu;
	bool TopButtons;
	bool FreqInfo;
	bool StatusInfoGUI;
	bool StatusInfoBar;
	bool SystemMenu;
} DEF_LCD_UpdateQuery;

struct button_handler {
	uint16_t x1;
	uint16_t x2;
	uint16_t y1;
	uint16_t y2;
	void(*handler) ();
};

extern void LCD_Init(void);
extern void LCD_doEvents(void);
extern void LCD_showError(char text[]);
extern void LCD_redraw(void);
extern void LCD_checkTouchPad(void);

void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char* text, uint16_t back_color, uint16_t text_color, uint16_t active_color, bool active, void(*onclick) ());
void printMenuButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char* text1, char* text2, bool active, bool switchable, void(*onclick) ());

extern uint32_t LCD_last_showed_freq;
extern uint8_t LCD_menu_main_index;
extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
extern bool LCD_mainMenuOpened;
extern bool LCD_modeMenuOpened;
extern bool LCD_timeMenuOpened;
extern bool LCD_systemMenuOpened;
extern char LCD_freq_string_hz[6];
extern char LCD_freq_string_khz[6];
extern char LCD_freq_string_mhz[6];
extern bool LCD_bandMenuOpened;
extern int LCD_last_s_meter;
extern bool LCD_busy;
extern uint8_t TimeMenuSelection;
extern IWDG_HandleTypeDef hiwdg;

//HANDLERS
void LCD_Handler_TUNE(void);
void LCD_Handler_MODE(void);
void LCD_Handler_BAND(void);
void LCD_Handler_WIDTH(void);
void LCD_Handler_WIDTH_BACK(void);
void LCD_Handler_PREAMP(void);
void LCD_Handler_VFO(void);
void LCD_Handler_AGC(void);
void LCD_Handler_MUTE(void);
void LCD_Handler_FAST(void);
void LCD_Handler_MENU_MAP(void);
void LCD_Handler_MENU_SYSTEM_MENU(void);
void LCD_Handler_MENU_LINEMIC(void);
void LCD_Handler_SETTIME(void);
void LCD_Handler_TIMEMENU_NEXT(void);
void LCD_Handler_TIMEMENU_BACK(void);
void LCD_Handler_MENU(void);
void LCD_Handler_BAND_160(void);
void LCD_Handler_BAND_80(void);
void LCD_Handler_BAND_40(void);
void LCD_Handler_BAND_30(void);
void LCD_Handler_BAND_20(void);
void LCD_Handler_BAND_17(void);
void LCD_Handler_BAND_15(void);
void LCD_Handler_BAND_12(void);
void LCD_Handler_BAND_10(void);
void LCD_Handler_BAND_FM1(void);
void LCD_Handler_BAND_FM2(void);
void LCD_Handler_BAND_VHF(void);
void LCD_Handler_BAND_UHF(void);
void LCD_Handler_BAND_BACK(void);
void LCD_Handler_MODE_LSB(void);
void LCD_Handler_MODE_USB(void);
void LCD_Handler_MODE_IQ(void);
void LCD_Handler_MODE_CW_L(void);
void LCD_Handler_MODE_CW_U(void);
void LCD_Handler_MODE_BACK(void);
void LCD_Handler_MODE_DIGL(void);
void LCD_Handler_MODE_DIGU(void);
void LCD_Handler_MODE_NFM(void);
void LCD_Handler_MODE_WFM(void);
void LCD_Handler_MODE_AM(void);
void LCD_Handler_MODE_LOOP(void);
void LCD_Handler_MENU_BACK(void);
void LCD_Handler_MENU_VOLUME(void);
void LCD_Handler_MENU_RF_GAIN(void);
void LCD_Handler_MENU_FM_SQL(void);
void LCD_Handler_MENU_RF_POWER(void);
void LCD_Handler_MENU_AGC_S(void);
void LCD_Handler_WIDTH_0(void);
void LCD_Handler_WIDTH_03(void);
void LCD_Handler_WIDTH_05(void);
void LCD_Handler_WIDTH_14(void);
void LCD_Handler_WIDTH_16(void);
void LCD_Handler_WIDTH_18(void);
void LCD_Handler_WIDTH_21(void);
void LCD_Handler_WIDTH_23(void);
void LCD_Handler_WIDTH_25(void);
void LCD_Handler_WIDTH_27(void);
void LCD_Handler_WIDTH_29(void);
void LCD_Handler_WIDTH_30(void);
void LCD_Handler_WIDTH_32(void);
void LCD_Handler_WIDTH_34(void);
void LCD_Handler_WIDTH_50(void);
void LCD_Handler_WIDTH_60(void);
void LCD_Handler_WIDTH_70(void);
void LCD_Handler_WIDTH_80(void);
void LCD_Handler_WIDTH_85(void);
void LCD_Handler_WIDTH_90(void);
void LCD_Handler_WIDTH_95(void);
void LCD_Handler_WIDTH_100(void);
void LCD_Handler_WIDTH_150(void);
//

#endif
