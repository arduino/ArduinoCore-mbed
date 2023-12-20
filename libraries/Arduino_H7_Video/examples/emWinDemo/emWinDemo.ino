/*
  emWinDemo

  created 04 Dec 2023
  by Leonardo Cavagnis
*/

#include "DIALOG.h" /* emWin library includes Arduino_H7_Video and Arduino_GigaDisplayTouch library */

/* 
* Main window handler: It creates 4 window childs. 
* Source: https://wiki.segger.com/WM_child_windows_(Sample) 
*/
static void _cbWin(WM_MESSAGE * pMsg) {
  switch (pMsg->MsgId) {
    case WM_CREATE:
      /* [0, 0] - Image */
      WM_CreateWindowAsChild(20, 20, 370, 210, pMsg->hWin, WM_CF_SHOW, _cbChildWinImg, 0);

      /* [1, 0] - Slider */
      WM_CreateWindowAsChild(20, 210+20*2, 370, 210, pMsg->hWin, WM_CF_SHOW, _cbChildWinSlider, 0);

      /* [0, 1] - Checkbox, button and labels */
      WM_CreateWindowAsChild(370+20*2, 20, 370, 210, pMsg->hWin, WM_CF_SHOW, _cbChildWinChkBtn, 0);

      /* [1, 1] - Progress bar */
      WM_CreateWindowAsChild(370+20*2, 210+20*2, 370, 210, pMsg->hWin, WM_CF_SHOW, _cbChildWinPgrBar, 0);
      break;
    case WM_PAINT:
      GUI_SetBkColor(0x03989e); /* Background color set to: R(0x03),G(0x98),B(0x9E) */
      GUI_Clear();
      break;
    default:
      WM_DefaultProc(pMsg);
      break;
  }
}

/* 
* Image window handler
* To convert image use "Bitmap Converter for emWin" 
* https://www.segger.com/products/user-interface/emwin/tools/tools-overview/ 
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmarduinologo; /* Image bitmap structure (see img_arduinologo_emwin.c in attach) */

static void _cbChildWinImg(WM_MESSAGE * pMsg) {
  switch (pMsg->MsgId) {
    case WM_CREATE:
      break;
    case WM_PAINT:
      GUI_SetBkColor(GUI_WHITE);
      GUI_Clear();
      /* Draw image */
      GUI_DrawBitmap(&bmarduinologo, 85, 35);
      break;
    default:
      WM_DefaultProc(pMsg);
      break;
  }
}

/* 
* Slider window handler
* Source: https://wiki.segger.com/SLIDER_-_Usage_(Sample)
*/
static void _cbChildWinSlider(WM_MESSAGE * pMsg) {
  static WM_HWIN hSlider;
  int            NCode, Id;
  int            Value;
  char           acBuffer[32];

  switch(pMsg->MsgId) {
    case WM_CREATE:
      /* Create horizonzal slider */
      hSlider = SLIDER_CreateEx(110, 90, 150, 30, pMsg->hWin, WM_CF_SHOW, SLIDER_CF_HORIZONTAL, GUI_ID_SLIDER0);
      /* Set range of slider */
      SLIDER_SetRange(hSlider, 0, 100);
      /* Set number of tick marks */
      SLIDER_SetNumTicks(hSlider, 10);
      /* Set value of slider */
      SLIDER_SetValue(hSlider, 20);
      /* Set width of thumb */
      SLIDER_SetWidth(hSlider, 20);
      break;
    case WM_PAINT:
      GUI_SetBkColor(GUI_WHITE);
      GUI_Clear();
      GUI_SetFont(&GUI_Font13B_1);
      GUI_SetColor(GUI_BLACK);

      /* Display slider value */
      Value = SLIDER_GetValue(hSlider);
      sprintf(acBuffer, "Value: %d", Value);
      GUI_DispStringAt(acBuffer, 110, 120);
      break;
    case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);
      NCode = pMsg->Data.v;

      switch(Id) {
        case GUI_ID_SLIDER0:
          switch(NCode) {
            case WM_NOTIFICATION_VALUE_CHANGED:
              /* Redraw the window when a value has changed so the displayed value will be updated */
              WM_InvalidateWindow(pMsg->hWin);
              break;
          }
          break;
      }
      break;
    default:
      WM_DefaultProc(pMsg);
  }
}

/* 
* Checkbox&Button window handler
* Source: 
* https://wiki.segger.com/CHECKBOX_-_Usage_(Sample)
* https://wiki.segger.com/BUTTON_-_Usage_(Sample)
*/
#define ID_BUTTON  1

static void _cbChildWinChkBtn(WM_MESSAGE * pMsg) {
  static WM_HWIN  hBox;
  BUTTON_Handle   hButton;
  int             NCode, Id;
  char            acBuffer[32];
  int             State;
  static int      Clicked, Released;

  switch(pMsg->MsgId) {
    case WM_CREATE:
      /* Create CHECKBOX widget */
      hBox = CHECKBOX_CreateEx(10, 30, 80, 20, pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_CHECK0);
      /* Edit widget properties */
      CHECKBOX_SetText(hBox, "Check");
      CHECKBOX_SetTextColor(hBox, GUI_BLACK);
      CHECKBOX_SetFont(hBox, &GUI_Font16_1);
      /* Set number of possible states to 3 (if needed). The minimum number of states is 2 and the maximum is 3 */
      CHECKBOX_SetNumStates(hBox, 3);
      /* Manually set the state */
      CHECKBOX_SetState(hBox, 1);

      /* Create a button */
      hButton = BUTTON_CreateEx(10, 100, 80, 20, pMsg->hWin, WM_CF_SHOW, 0, ID_BUTTON);
      BUTTON_SetText(hButton, "Click me");
      break;
    case WM_PAINT:
      GUI_SetBkColor(GUI_WHITE);
      GUI_Clear();
      GUI_SetFont(&GUI_Font16_1);
      GUI_SetColor(GUI_BLACK);

      /* Display current CHECKBOX state */
      State = CHECKBOX_GetState(hBox);
      sprintf(acBuffer, "State of checkbox: %d", State);
      GUI_DispStringAt(acBuffer, 10, 60);

      /* Check button state and print info on labels */
      if(Clicked) {
        sprintf(acBuffer, "Button was clicked at: %d.", Clicked);
        GUI_DispStringAt(acBuffer, 10, 130);
      }
      if(Released) {
        sprintf(acBuffer, "Button was released at: %d.", Released);
        GUI_DispStringAt(acBuffer, 10, 150);
      }
      break;
    case WM_NOTIFY_PARENT:
    /* Get Id of sender window and notification code */
      Id    = WM_GetId(pMsg->hWinSrc);
      NCode = pMsg->Data.v;
      
      switch (Id) {
        case GUI_ID_CHECK0:
          switch(NCode) {
            case WM_NOTIFICATION_VALUE_CHANGED:
              /* When the value of the checkbox changed, redraw parent window to update the display of the state */
              WM_InvalidateWindow(pMsg->hWin);
              break;
          }  
          break;
        case ID_BUTTON:
          switch(NCode) {
            case WM_NOTIFICATION_CLICKED:
              Clicked = GUI_GetTime();
              WM_InvalidateWindow(pMsg->hWin);
              break;
            case WM_NOTIFICATION_RELEASED:
              Released = GUI_GetTime();
              WM_InvalidateWindow(pMsg->hWin);
              break;
          }
          break;
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/* 
* Progress bar window handler
* Source: https://wiki.segger.com/PROGBAR_-_Custom_(Sample) 
*/
PROGBAR_Handle hProg;

static void _cbChildWinPgrBar(WM_MESSAGE * pMsg) {
  GUI_RECT Rect;
  float    ValueF;
  int      Value;
  char     acBuffer[16];

  switch (pMsg->MsgId) {
    case WM_CREATE:
      hProg = PROGBAR_CreateEx(85, 90, 200, 30, pMsg->hWin, WM_CF_SHOW, PROGBAR_CF_HORIZONTAL, GUI_ID_PROGBAR0);
      WM_SetCallback(hProg, _cbProgbar);
      break;
    case WM_PAINT:
      GUI_SetBkColor(GUI_WHITE);
      GUI_Clear();
      break;
    default:
      WM_DefaultProc(pMsg);
      break;
  }
}

/* 
* Progress bar widget handler
* Source: https://wiki.segger.com/PROGBAR_-_Custom_(Sample) 
*/
static void _cbProgbar(WM_MESSAGE * pMsg) {
  GUI_RECT Rect;
  float    ValueF;
  int      Value;
  char     acBuffer[16];

  switch (pMsg->MsgId) {
    case WM_PAINT:
      GUI_SetBkColor(GUI_WHITE);
      GUI_Clear();
      /* Draw progress bar */
      WM_GetClientRect(&Rect);
      GUI_SetColor(GUI_BLACK);
      GUI_AA_DrawRoundedRectEx(&Rect, 3);
      Value = PROGBAR_GetValue(pMsg->hWin);
      ValueF = Value / 100.0F;
      sprintf(acBuffer, "Progress: %d%%", Value);
      Rect.x0 += 2;
      Rect.y0 += 2;
      Rect.x1 -= 2;
      Rect.y1 -= 2;
      Rect.x1 = Rect.x1 * (ValueF);
      GUI_SetColor(GUI_GRAY_9A);
      GUI_AA_FillRoundedRectEx(&Rect, 1);
      WM_GetClientRect(&Rect);
      Rect.x0 += 2;
      Rect.y0 += 2;
      Rect.x1 -= 2;
      Rect.y1 -= 2;
      GUI_SetColor(GUI_BLACK);
      GUI_SetTextMode(GUI_TM_TRANS);
      GUI_SetFont(&GUI_Font16B_1);
      GUI_DispStringInRect(acBuffer, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER);
      break;
    default:
      PROGBAR_Callback(pMsg);
      break;
   }
}

int progbarCnt = 0;
unsigned long previousMillis = 0;

void setup() {
  /* Init SEGGER emWin library. It also init display and touch controller */
  GUI_Init();

  LCD_ROTATE_SetSel(1); /* Set landscape mode */
  WM_MULTIBUF_Enable(1); /* Enable multi buffering mode for Windows manager */

  /* Create the main window. It will include all the sub-windows */
  WM_CreateWindowAsChild(0, 0, LCD_GetXSize(), LCD_GetYSize(), WM_HBKWIN, WM_CF_SHOW, _cbWin, 0);
}

void loop() {
  /* Update progress bar value */
  if (millis() - previousMillis >= 100) {
    previousMillis = millis();
    progbarCnt++;
    if (progbarCnt > 100) {
      progbarCnt = 0;
    }
    PROGBAR_SetValue(hProg, progbarCnt);
    WM_InvalidateWindow(hProg); /* Make sure the entire PROGBAR gets redrawn */
  }

  /* Keep emWin alive, handle touch and other stuff */
  GUI_Exec();  
}