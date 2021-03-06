#include "wifi.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include "functions.h"
#include "settings.h"
#include "trx_manager.h"
#include "lcd.h"
#include <stdlib.h>

static WiFiProcessingCommand WIFI_ProcessingCommand = WIFI_COMM_NONE;
static void (*WIFI_ProcessingCommandCallback)(void);

static SRAM1 char WIFI_AnswerBuffer[WIFI_ANSWER_BUFFER_SIZE] = {0};
static char WIFI_readedLine[WIFI_ANSWER_BUFFER_SIZE] = {0};
static char tmp[WIFI_ANSWER_BUFFER_SIZE] = {0};
static uint16_t WIFI_Answer_ReadIndex = 0;
static uint32_t commandStartTime = 0;
static uint8_t WIFI_FoundedAP_Index = 0;
static bool WIFI_stop_auto_ap_list = false;

static void WIFI_SendCommand(char *command);
static bool WIFI_WaitForOk(void);
static bool WIFI_ListAP_Sync(void);
static bool WIFI_TryGetLine(void);

bool WIFI_connected = false;
volatile uint8_t WIFI_InitStateIndex = 0;
volatile WiFiState WIFI_State = WIFI_UNDEFINED;
static char WIFI_FoundedAP_InWork[WIFI_FOUNDED_AP_MAXCOUNT][32] = {0};
volatile char WIFI_FoundedAP[WIFI_FOUNDED_AP_MAXCOUNT][32] = {0};
bool WIFI_IP_Gotted = false;
char WIFI_IP[15] = {0};

void WIFI_Init(void)
{
	WIFI_SendCommand("AT+UART_CUR=115200,8,1,0,1\r\n"); //uart config
	WIFI_WaitForOk();
	WIFI_SendCommand("ATE0\r\n"); //echo off
	WIFI_WaitForOk();
	WIFI_SendCommand("AT+GMR\r\n"); //system info ESP8266
	WIFI_WaitForOk();
	/*
	AT version:1.6.2.0(Apr 13 2018 11:10:59)
	SDK version:2.2.1(6ab97e9)
	compile time:Jun  7 2018 19:34:27
	Bin version(Wroom 02):1.6.2
	*/
	WIFI_SendCommand("AT\r\n");
	WIFI_WaitForOk();
	if (strstr(WIFI_readedLine, "OK") != NULL)
	{
		sendToDebug_str("[WIFI] WIFI Module Inited\r\n");
		WIFI_State = WIFI_INITED;

		//проверяем, есть ли активные подключения, если да - новое не создаём
		WIFI_SendCommand("AT+CIPSTATUS\r\n");
		while (WIFI_TryGetLine())
		{
			if (strstr(WIFI_readedLine, "STATUS:2") != NULL)
			{
				WIFI_SendCommand("AT+CWAUTOCONN=1\r\n"); //AUTOCONNECT
				WIFI_WaitForOk();
				WIFI_State = WIFI_READY;
				WIFI_connected = true;
				LCD_UpdateQuery.StatusInfoBar = true;
				sendToDebug_str("[WIFI] Connected\r\n");
			}
		}
	}
	if (WIFI_State == WIFI_UNDEFINED)
	{
		WIFI_State = WIFI_NOTFOUND;
		sendToDebug_str("[WIFI] WIFI Module Not Found\r\n");
	}
}

void WIFI_Process(void)
{
	char com_t[128] = {0};
	char tz[2] = {0};
	char com[128] = {0};

	if (WIFI_State == WIFI_NOTFOUND)
		return;
	if (WIFI_State == WIFI_UNDEFINED)
	{
		WIFI_Init();
		return;
	}

	/////////

	switch (WIFI_State)
	{
	case WIFI_INITED:
		sendToDebug_str3("[WIFI] Start connecting to AP: ", TRX.WIFI_AP, "\r\n");
		WIFI_SendCommand("AT+CWAUTOCONN=0\r\n"); //AUTOCONNECT OFF
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+RFPOWER=82\r\n"); //rf power
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWMODE_CUR=1\r\n"); //station mode
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWDHCP_CUR=1,1\r\n"); //DHCP
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWHOSTNAME=\"UA3REO\"\r\n"); //Hostname
		WIFI_WaitForOk();
		WIFI_SendCommand("AT+CWCOUNTRY_CUR=1,\"RU\",1,13\r\n"); //Country
		WIFI_WaitForOk();
		strcat(com_t, "AT+CIPSNTPCFG=1,");
		sprintf(tz, "%d", TRX.WIFI_TIMEZONE);
		strcat(com_t, tz);
		strcat(com_t, ",\"pool.ntp.org\"\r\n");
		WIFI_SendCommand(com_t); //configure SNMP
		WIFI_WaitForOk();
		WIFI_stop_auto_ap_list = false;
		WIFI_IP_Gotted = false;
		WIFI_State = WIFI_CONFIGURED;
		break;
	case WIFI_CONFIGURED:
		if (WIFI_stop_auto_ap_list)
			break;
		WIFI_ListAP_Sync();
		bool AP_exist = false;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
		{
			if (strcmp((char *)WIFI_FoundedAP[i], TRX.WIFI_AP) == 0)
				AP_exist = true;
		}
		if (AP_exist)
		{
			strcat(com, "AT+CWJAP_CUR=\"");
			strcat(com, TRX.WIFI_AP);
			strcat(com, "\",\"");
			strcat(com, TRX.WIFI_PASSWORD);
			strcat(com, "\"\r\n");
			WIFI_SendCommand(com); //connect to AP
			WIFI_WaitForOk();
			WIFI_State = WIFI_CONNECTING;
		}
		break;

	case WIFI_CONNECTING:
		WIFI_TryGetLine();
		if (strstr(WIFI_readedLine, "GOT IP") != NULL)
		{
			sendToDebug_str("[WIFI] Connected\r\n");
			WIFI_SendCommand("AT+CWAUTOCONN=1\r\n"); //AUTOCONNECT
			WIFI_WaitForOk();
			WIFI_State = WIFI_READY;
			WIFI_connected = true;
			LCD_UpdateQuery.StatusInfoBar = true;
		}
		if (strstr(WIFI_readedLine, "WIFI DISCONNECT") != NULL)
		{
			sendToDebug_str("[WIFI] Disconnected\r\n");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			LCD_UpdateQuery.StatusInfoBar = true;
		}
		if (strstr(WIFI_readedLine, "FAIL") != NULL)
		{
			sendToDebug_str("[WIFI] Connect failed\r\n");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			LCD_UpdateQuery.StatusInfoBar = true;
		}
		if (strstr(WIFI_readedLine, "ERROR") != NULL)
		{
			sendToDebug_str("[WIFI] Connect error\r\n");
			WIFI_State = WIFI_CONFIGURED;
			WIFI_connected = false;
			LCD_UpdateQuery.StatusInfoBar = true;
		}
		break;

	case WIFI_READY:
		WIFI_ProcessingCommandCallback = 0;
		break;

	case WIFI_TIMEOUT:
		WIFI_TryGetLine();
		if (WIFI_connected)
			WIFI_State = WIFI_READY;
		else
			WIFI_State = WIFI_CONFIGURED;
		WIFI_InitStateIndex = 0;
		break;

	case WIFI_PROCESS_COMMAND:
		WIFI_TryGetLine();
		if ((HAL_GetTick() - commandStartTime) > WIFI_COMMAND_TIMEOUT)
		{
			WIFI_State = WIFI_TIMEOUT;
			WIFI_ProcessingCommand = WIFI_COMM_NONE;
		}
		else if (strstr(WIFI_readedLine, "OK") != NULL)
		{
			if (WIFI_ProcessingCommand == WIFI_COMM_LISTAP) //ListAP Command Ended
				for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
					strcpy((char *)&WIFI_FoundedAP[i], (char *)&WIFI_FoundedAP_InWork[i]);
			if (WIFI_ProcessingCommandCallback != NULL)
			{
				WIFI_ProcessingCommandCallback();
			}
			WIFI_ProcessingCommand = WIFI_COMM_NONE;
			WIFI_stop_auto_ap_list = false;
		}
		else if (strlen(WIFI_readedLine) > 5)
		{
			if (WIFI_ProcessingCommand == WIFI_COMM_LISTAP) //ListAP Command process
			{
				char *start = strchr(WIFI_readedLine, '"');
				if(start != NULL)     
				{           
					start = start + 1;
					char *end = strchr(start, '"');
					if(end != NULL)
					{
						*end = 0x00;
						strcat((char *)&WIFI_FoundedAP_InWork[WIFI_FoundedAP_Index], start);
						if (WIFI_FoundedAP_Index < WIFI_FOUNDED_AP_MAXCOUNT)
								WIFI_FoundedAP_Index++;
					}
				}
			}
			else if (WIFI_ProcessingCommand == WIFI_COMM_GETSNMP) //Get and sync SNMP time
			{
				char *hrs_str = strchr(WIFI_readedLine, ' ');
				if(hrs_str != NULL)
				{
					hrs_str = hrs_str + 1;
					hrs_str = strchr(hrs_str, ' ');
					if(hrs_str != NULL)
					{
						hrs_str = hrs_str + 1;
						hrs_str = strchr(hrs_str, ' ');
						if(hrs_str != NULL)
						{
							hrs_str = hrs_str + 1;
							//hh:mm:ss here
							char *min_str = strchr(hrs_str, ':');
							if(min_str != NULL)
							{
								min_str = min_str + 1;
								char *sec_str = strchr(min_str, ':');
								char *year_str = strchr(min_str, ' ');
								char *end = strchr(hrs_str, ':');
								if(sec_str != NULL && year_str != NULL && end != NULL)
								{
									sec_str = sec_str + 1;
									year_str = year_str + 1;
									*end = 0x00;
									end = strchr(min_str, ':');
									if(end != NULL)
									{
										*end = 0x00;
										end = strchr(sec_str, ' ');
										if(end != NULL)
										{
											*end = 0x00;
											//split strings here
											uint8_t hrs = (uint8_t)atoi(hrs_str);
											uint8_t min = (uint8_t)atoi(min_str);
											uint8_t sec = (uint8_t)atoi(sec_str);
											uint16_t year = (uint16_t)atoi(year_str);
											//save to RTC clock
											if (year > 2018)
											{
												RTC_TimeTypeDef sTime;
												sTime.TimeFormat = RTC_HOURFORMAT12_PM;
												sTime.SubSeconds = 0;
												sTime.SecondFraction = 0;
												sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
												sTime.StoreOperation = RTC_STOREOPERATION_SET;
												sTime.Hours = hrs;
												sTime.Minutes = min;
												sTime.Seconds = sec;
												BKPSRAM_Enable();
												HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
												TRX_SNMP_Synced = true;
												sendToDebug_str("[WIFI] TIME SYNCED\r\n");
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else if (WIFI_ProcessingCommand == WIFI_COMM_GETIP) //GetIP Command process
			{
				char *sep = "_CUR:ip";
				char *istr;
				istr = strstr(WIFI_readedLine, sep);
				if (istr != NULL)
				{
					char *start = strchr(WIFI_readedLine, '"');
					if(start != NULL)
					{
						start = start + 1;
						char *end = strchr(start, '"');
						if(end != NULL)
						{
							*end = 0x00;
							strcat(WIFI_IP, start);
							sendToDebug_str3("[WIFI] GOT IP: ", WIFI_IP, "\r\n");
							WIFI_IP_Gotted = true;
						}
					}
				}
			}
		}
		break;

	case WIFI_UNDEFINED:
	case WIFI_NOTFOUND:
	case WIFI_FAIL:
	case WIFI_SLEEP:
		break;
	}
}

bool WIFI_GetSNMPTime(void *callback)
{
	if (WIFI_State != WIFI_READY)
		return false;
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_GETSNMP;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+CIPSNTPTIME?\r\n"); //get SNMP time
	return true;
}

bool WIFI_GetIP(void *callback)
{
	if (WIFI_State != WIFI_READY)
		return false;
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_GETIP;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_SendCommand("AT+CIPSTA_CUR?\r\n"); //get ip
	return true;
}

void WIFI_ListAP(void *callback)
{
	if (WIFI_State != WIFI_READY && WIFI_State != WIFI_CONFIGURED)
		return;
	if (WIFI_State == WIFI_CONFIGURED && !WIFI_stop_auto_ap_list) //останавливаем авто-подключение при поиске сетей
	{
		WIFI_stop_auto_ap_list = true;
		WIFI_WaitForOk();
	}
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_LISTAP;
	WIFI_ProcessingCommandCallback = callback;
	WIFI_FoundedAP_Index = 0;

	for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
		memset((char *)&WIFI_FoundedAP_InWork[i], 0x00, sizeof WIFI_FoundedAP_InWork[i]);
	WIFI_SendCommand("AT+CWLAP\r\n"); //List AP
}

static bool WIFI_ListAP_Sync(void)
{
	WIFI_SendCommand("AT+CWLAP\r\n"); //List AP
	WIFI_FoundedAP_Index = 0;
	for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
		memset((char *)&WIFI_FoundedAP[i], 0x00, sizeof WIFI_FoundedAP[i]);
	uint32_t startTime = HAL_GetTick();
	char *sep = "OK";
	char *istr;

	while ((HAL_GetTick() - startTime) < WIFI_COMMAND_TIMEOUT)
	{
		if (!WIFI_TryGetLine())
		{
			CPULOAD_GoToSleepMode();
			CPULOAD_WakeUp();
			continue;
		}

		istr = strstr(WIFI_readedLine, sep);
		if (istr != NULL)
			return true;

		if (strlen(WIFI_readedLine) > 5) //-V814
		{
			char *start = strchr(WIFI_readedLine, '"');
			if(start != NULL)
			{
				start = start + 1;
				char *end = strchr(start, '"');
				if(end != NULL)
				{
					*end = 0x00;
					strcat((char *)&WIFI_FoundedAP[WIFI_FoundedAP_Index], start);
					if (WIFI_FoundedAP_Index < WIFI_FOUNDED_AP_MAXCOUNT)
						WIFI_FoundedAP_Index++;
				}
			}
		}
	}
	return false;
}

void WIFI_GoSleep(void)
{
	if (WIFI_State == WIFI_SLEEP)
		return;
	WIFI_State = WIFI_PROCESS_COMMAND;
	WIFI_ProcessingCommand = WIFI_COMM_DEEPSLEEP;
	WIFI_SendCommand("AT+GSLP=1000\r\n"); //go sleep
	WIFI_State = WIFI_SLEEP;
}

static void WIFI_SendCommand(char *command)
{
	HAL_UART_AbortReceive(&huart6);
	HAL_UART_AbortReceive_IT(&huart6);
	memset(WIFI_AnswerBuffer, 0x00, sizeof(WIFI_AnswerBuffer));
	WIFI_Answer_ReadIndex = 0;
	HAL_UART_Receive_DMA(&huart6, (uint8_t *)WIFI_AnswerBuffer, WIFI_ANSWER_BUFFER_SIZE);
	HAL_Delay(100);
	HAL_UART_Transmit_IT(&huart6, (uint8_t *)command, (uint16_t)strlen(command));
	commandStartTime = HAL_GetTick();
	HAL_Delay(WIFI_COMMAND_DELAY);
#if WIFI_DEBUG //DEBUG
	sendToDebug_str2("WIFI_S: ", command);
#endif
}

static bool WIFI_WaitForOk(void)
{
	char *sep = "OK";
	char *istr;
	uint32_t startTime = HAL_GetTick();
	while ((HAL_GetTick() - startTime) < WIFI_COMMAND_TIMEOUT)
	{
		if (WIFI_TryGetLine())
		{
			istr = strstr(WIFI_readedLine, sep);
			if (istr != NULL)
			{
				return true;
			}
		}
		CPULOAD_GoToSleepMode();
		CPULOAD_WakeUp();
	}
	return false;
}
static bool WIFI_TryGetLine(void)
{
	memset(WIFI_readedLine, 0x00, sizeof(WIFI_readedLine));
	memset(tmp, 0x00, sizeof(tmp));

	uint16_t dma_index = WIFI_ANSWER_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(huart6.hdmarx);
	if (WIFI_Answer_ReadIndex == dma_index)
		return false;

	strncpy(tmp, &WIFI_AnswerBuffer[WIFI_Answer_ReadIndex], dma_index - WIFI_Answer_ReadIndex);
	if (tmp[0] =='\0')
		return false;

	char *istr = strchr(tmp, '\n'); //ищем конец строки
	if (istr == NULL)
		return false;
	uint16_t len = (uint16_t)((uint32_t)istr - (uint32_t)tmp + 1);
	strncpy(WIFI_readedLine, tmp, len);

	WIFI_Answer_ReadIndex += len;
	if (WIFI_Answer_ReadIndex > dma_index)
		WIFI_Answer_ReadIndex = dma_index;

#if WIFI_DEBUG //DEBUG
	sendToDebug_str2("WIFI_R: ", WIFI_readedLine);
#endif

	return true;
}
