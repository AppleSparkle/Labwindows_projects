/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2014. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: panelCB */
#define  PANEL_GRAPH                      2       /* control type: graph, callback function: (none) */
#define  PANEL_LOGIC_RING                 3       /* control type: ring, callback function: (none) */
#define  PANEL_SOURCE_RING                4       /* control type: ring, callback function: (none) */
#define  PANEL_TRIG_VALUE                 5       /* control type: numeric, callback function: (none) */
#define  PANEL_STOP_BUTTON                6       /* control type: command, callback function: stop_acquisition */
#define  PANEL_START_BUTTON               7       /* control type: command, callback function: start_acquisition */
#define  PANEL_CONNECT_BUTTON             8       /* control type: command, callback function: connect */
#define  PANEL_CONN_LED                   9       /* control type: LED, callback function: (none) */
#define  PANEL_PORT_STRING                10      /* control type: string, callback function: (none) */
#define  PANEL_IP_STRING                  11      /* control type: string, callback function: (none) */
#define  PANEL_FREQ_DIV                   12      /* control type: numeric, callback function: freq_div_CB */
#define  PANEL_US_CONV                    13      /* control type: numeric, callback function: (none) */
#define  PANEL_STATUS_LED                 14      /* control type: LED, callback function: (none) */
#define  PANEL_NUMERIC_COUNT              15      /* control type: numeric, callback function: (none) */
#define  PANEL_TIMER                      16      /* control type: timer, callback function: timer_CB */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK connect(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK freq_div_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK start_acquisition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK stop_acquisition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK timer_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
