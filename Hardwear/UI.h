/* UI.h */
#ifndef __UI_H
#define __UI_H

#include "stm32f10x.h"

void UI_Boot(void);
void UI_ShowIdle(uint8_t mode);
void UI_ShowMeasuring(const char *label, uint8_t progress);
void UI_ShowAlert(uint8_t sec_left);
void UI_ShowHelpSent(void);
void UI_ShowCancelled(void);

#endif
