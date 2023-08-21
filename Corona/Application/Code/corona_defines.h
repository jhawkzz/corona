
#ifndef CORONA_DEFINES_H_
#define CORONA_DEFINES_H_

#pragma warning(disable : 4995) //Disable security warnings for stdio functions
#pragma warning(disable : 4996) //Disable security warnings for stdio functions
#define _CRT_SECURE_NO_DEPRECATE //Disable security warnings for stdio functions
#define _CRT_SECURE_NO_WARNINGS //Disable security warnings for stdio functions

#define HANDLE_DLGMSG( hwnd, message, fn ) \
   case (message): return ( SetDlgMsgResult( (hwnd), (message), HANDLE_##message( (hwnd), (wParam), (lParam), (fn) ) ) )

#define CORONA_VERSION                 "0.10"
#define CORONA_NAME                    "Corona Software Renderer"
#define CORONA_CAPTION                 CORONA_NAME" "CORONA_VERSION
#define CORONA_REGISTRY_KEY            "Software\\Corona"

#define ROOT_DRIVE                     "G:\\"

// limits
#define CORONA_DELTA_UPDATE_FRAMES     (0.016666f)
#define CORONA_DELTA_UPDATE_FRAMES_MS  (CORONA_DELTA_UPDATE_FRAMES * 1000)
#define CORONA_MAX_ENTITIES            (5)
#define CORONA_MAX_SCREEN_WIDTH        (1024)
#define CORONA_MAX_SCREEN_HEIGHT       (768)
#define CORONA_MAX_BIT_DEPTH           (32)

// math
#define PI                             (3.1415926f)

// debugging
#define CORONA_AVERAGE_FPS_TICKS       (5)

// Directional
#define BUTTON_LEFT                    (1 <<  0)
#define BUTTON_RIGHT                   (1 <<  1)
#define BUTTON_UP                      (1 <<  2)
#define BUTTON_DOWN                    (1 <<  3)

// Numeric
#define BUTTON_1                       (1 <<  4)
#define BUTTON_2                       (1 <<  5)
#define BUTTON_3                       (1 <<  6)
#define BUTTON_4                       (1 <<  7)
#define BUTTON_5                       (1 <<  8)
#define BUTTON_6                       (1 <<  9)
#define BUTTON_7                       (1 << 10)
#define BUTTON_8                       (1 << 11)
#define BUTTON_9                       (1 << 12)
#define BUTTON_0                       (1 << 13)

// Alpha-Numeric
#define BUTTON_W                       (1 << 14)
#define BUTTON_A                       (1 << 15)
#define BUTTON_S                       (1 << 16)
#define BUTTON_D                       (1 << 17)
#define BUTTON_Q                       (1 << 18)
#define BUTTON_E                       (1 << 19)

// Functional
#define BUTTON_TAB                     (1 << 24)
#define BUTTON_SPACE                   (1 << 25)
#define BUTTON_ENTER                   (1 << 26)
#define BUTTON_BACKSPACE               (1 << 27)
#define BUTTON_PLUS                    (1 << 28)
#define BUTTON_MINUS                   (1 << 29)


#endif
