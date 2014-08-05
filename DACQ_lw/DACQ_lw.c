//==============================================================================
//
// Title:		DACQ_lw
// Purpose:		A short description of the application.
//
// Created on:	25.07.2014 at 14:35:27 by R2D2.
// Copyright:	srsbzns. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <tcpsupp.h>
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include "DACQ_lw.h"
#include "toolbox.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables

static int panelHandle = 0;
static unsigned int  conversation = 0; 

//==============================================================================
// Static functions
static int CVICALLBACK ClientTCPCB (unsigned handle, int event, int error,
                             void *callbackData);
//==============================================================================
// Global variables

int connected = 0;
int state = 0;
int count = 0;
int ping = 0;

int bytecnt = 0;  

char ch_receiveBuf[32768] = {0};
ssize_t dataSize	= sizeof (ch_receiveBuf) - 1;

unsigned char CMD;
unsigned char LEN;
unsigned char DAT[255];
int DAT_count = 0;

int DATA_X_DUMMY[2048];
int CH0_DATA[2048];
int CH1_DATA[2048];  
int CH2_DATA[2048];
int POS_ERROR_DATA[2048];  
int data_count = 0;

//==============================================================================
// Global functions

int UI_INIT(void);

int CMD_execute(unsigned char CMD,unsigned char LEN,unsigned char * DAT);
unsigned short Fletcher16( unsigned char * data, int count);











/// HIFN The main entry-point function.
int main (int argc, char *argv[])
{
	int error = 0;

	/* initialize and load resources */
	nullChk (InitCVIRTE (0, argv, 0));
	errChk (panelHandle = LoadPanel (0, "DACQ_lw.uir", PANEL));
	
	/* display the panel and run the user interface */
	errChk (DisplayPanel (panelHandle));
	
	errChk (UI_INIT());

	errChk (RunUserInterface ());

Error:
	/* clean up */
	if (panelHandle > 0)
		DiscardPanel (panelHandle);
	return 0;
}

int UI_INIT(void)
{
	unsigned short temp;  
		
	GetCtrlVal (panelHandle, PANEL_FREQ_DIV, &temp);
	SetCtrlVal (panelHandle, PANEL_US_CONV, 20*temp/1000);
	
	SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_LABEL_TEXT, "Stopped");
	SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_OFF_COLOR,  0xCC0000);
	SetCtrlVal (panelHandle, PANEL_STATUS_LED, 0);
	
	SetCtrlAttribute (panelHandle, PANEL_START_BUTTON, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_STOP_BUTTON, ATTR_DIMMED, 1);
	
	return 0;
}

//==============================================================================
// UI callback function prototypes

/// HIFN Exit when the user dismisses the panel.
int CVICALLBACK panelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	if (event == EVENT_CLOSE)
	{
		if (connected) DisconnectFromTCPServer(conversation); 
		QuitUserInterface (0);
		SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, 0);
	}
	return 0;
}

int CVICALLBACK connect (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	char ip   [64] = {0}; 
	char port [32] = {0};
	int portval;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			
			if (connected == 1)
			{ 
				SetCtrlVal (panel, PANEL_CONN_LED, 0);
				DisconnectFromTCPServer(conversation);
				connected = 0;
				SetCtrlAttribute (panelHandle, PANEL_START_BUTTON, ATTR_DIMMED, 1);
				SetCtrlAttribute (panelHandle, PANEL_STOP_BUTTON, ATTR_DIMMED, 1);
			}
			else
			{
				GetCtrlVal (panel, PANEL_IP_STRING,   ip);
				GetCtrlVal (panel, PANEL_PORT_STRING, port);
				portval = atoi(port);
				
				if (ConnectToTCPServer (&conversation, portval, ip, ClientTCPCB, NULL, 5000) <0 )
				{
					MessagePopup("TCP Client", "Connection to server failed !");	
				}
				else
				{
					connected = 1;
					SetCtrlVal (panel, PANEL_CONN_LED, 1);
					SetCtrlAttribute (panelHandle, PANEL_START_BUTTON, ATTR_DIMMED, 0);
					SetCtrlAttribute (panelHandle, PANEL_STOP_BUTTON, ATTR_DIMMED, 0);
				}
			}

			break;
	}
	return 0;
}

static int CVICALLBACK ClientTCPCB (unsigned handle, int event, int error, void *callbackData)
{
	int length;
    
    switch (event)
        {
        case TCP_DATAREADY:
			
			length = ClientTCPRead (conversation, ch_receiveBuf, dataSize, 1000);
			
            if (length < 1)
            {
            	
            }
            else
        	{
				
				bytecnt = 0;

				while (length > 0)
				{
					
					switch (state)
					{
						case 0:			//	Wait state
						{
							if (length > 0) state = 1;
							break;
						}
						case 1:			//	Get command
						{
							CMD = ch_receiveBuf[bytecnt];
							bytecnt++;
							length--;
							state = 2;
							break;
						}
						case 2:			//	Get data length
						{
							LEN = ch_receiveBuf[bytecnt];
							bytecnt++;
							length--;
							state = 3;
							break;
						}
						case 3:			//	Fill data to length
						{
							DAT[DAT_count] = ch_receiveBuf[bytecnt];
							DAT_count++;
							bytecnt++;
							length--;
							if (DAT_count == LEN)	//	Done - (check control sum) and (execute command)
								{
									DAT_count = 0;
									state = 0;
									
									CMD_execute(CMD, LEN, DAT);

									break;
								}
							else{break;}
						}
					}
				}
            }	 
			

            break;
			
        case TCP_DISCONNECT:
			
            MessagePopup ("TCP Client", "Server has closed connection!");
			connected = 0;
			SetCtrlVal (panelHandle, PANEL_CONN_LED, 0);
			
			break;
    	}
    return 0;
}


int CMD_execute(unsigned char CMD,unsigned char LEN,unsigned char * DAT)
{
	int ret = -1;
	int tmp, j;
	int buf;

	switch (CMD)
	{
			case 0xD0:	// acquired data transfer begin
			{
				data_count = 0;
				break; 
			}
			case 0xD1:	// get data on channel 0
			{
				buf = 0;
				tmp =  DAT[0];
				buf += (tmp&0xFF) << 24;
				tmp =  DAT[1];
				buf += (tmp&0xFF) << 16;
				tmp =  DAT[2];
				buf += (tmp&0xFF) << 8;
				tmp =  DAT[3];
				buf += (tmp&0xFF);
				
				CH0_DATA[data_count] = buf;
				data_count++;
				
				ret = 0;
				
				break;
			}
			case 0xD2:	// get data on channel 1
			{
				buf = 0;
				tmp =  DAT[0];
				buf += (tmp&0xFF) << 24;
				tmp =  DAT[1];
				buf += (tmp&0xFF) << 16;
				tmp =  DAT[2];
				buf += (tmp&0xFF) << 8;
				tmp =  DAT[3];
				buf += (tmp&0xFF);
				
				CH1_DATA[data_count] = buf;
				data_count++;
				
				ret = 0;
				
				break;
			}
			case 0xD3:	// get data on channel 2
			{
				buf = 0;
				tmp =  DAT[0];
				buf += (tmp&0xFF) << 24;
				tmp =  DAT[1];
				buf += (tmp&0xFF) << 16;
				tmp =  DAT[2];
				buf += (tmp&0xFF) << 8;
				tmp =  DAT[3];
				buf += (tmp&0xFF);
				
				CH2_DATA[data_count] = buf;
				data_count++;
				
				ret = 0;
				
				break;
			}
			case 0xDF: // data transfer ended
			{
				for (j = 0; j<1024; j++)
				{
					DATA_X_DUMMY[j] = j;
					POS_ERROR_DATA[j] = CH0_DATA[j] - CH1_DATA[j];   
					
				}
						
				DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW); 
		
				PlotXY (panelHandle, PANEL_GRAPH, DATA_X_DUMMY, CH0_DATA, 512, VAL_INTEGER, VAL_INTEGER, VAL_FAT_LINE,
				VAL_SIMPLE_DOT, VAL_SOLID, 1, VAL_RED);
				
				PlotXY (panelHandle, PANEL_GRAPH, DATA_X_DUMMY, CH1_DATA, 512, VAL_INTEGER, VAL_INTEGER, VAL_FAT_LINE,
				VAL_SIMPLE_DOT, VAL_SOLID, 1, VAL_GREEN);
				
				PlotXY (panelHandle, PANEL_GRAPH, DATA_X_DUMMY, CH2_DATA, 512, VAL_INTEGER, VAL_INTEGER, VAL_FAT_LINE,
				VAL_SIMPLE_DOT, VAL_SOLID, 1, VAL_BLUE);
				
				PlotXY (panelHandle, PANEL_GRAPH, DATA_X_DUMMY, POS_ERROR_DATA, 512, VAL_INTEGER, VAL_INTEGER, VAL_FAT_LINE,
				VAL_SIMPLE_DOT, VAL_SOLID, 1, VAL_YELLOW);
				
				SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, 0);
				
				SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_LABEL_TEXT, "Stopped");
				SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_OFF_COLOR,  0xCC0000);
				SetCtrlVal (panelHandle, PANEL_STATUS_LED, 0);
				
				SetCtrlAttribute (panelHandle, PANEL_START_BUTTON, ATTR_DIMMED, 0);
				break; 
			}
			default:{break;}
			
	}
	
	return ret;
}


int CVICALLBACK start_acquisition (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
				
	unsigned char buffer[32] = {0};
	int bytesToWrite;
	int bytesWritten;
	int messageSize;
	unsigned short checksum;
	
	unsigned char source, logic;
	int value;
	unsigned short freq_d;
				
	switch (event)
	{
		case EVENT_COMMIT:
			
				SetCtrlVal (panelHandle, PANEL_NUMERIC_COUNT, count++);           
			
				GetCtrlVal (panelHandle, PANEL_SOURCE_RING, &source);	   	// Which channel is the trigger source?
				GetCtrlVal (panelHandle, PANEL_LOGIC_RING,  &logic );  	   	// What is the logic fuinction?
				GetCtrlVal (panelHandle, PANEL_TRIG_VALUE,  &value );  	   	// What is the trigger level?
				GetCtrlVal (panelHandle, PANEL_FREQ_DIV,    &freq_d);  	   	// What is the sample rate?               

				buffer[0] = 0xDC;								// Command: send data acquisition parameters (0xDC)
				buffer[1] = 0x8;								// Length

				buffer[2] 	=  source & 0x000000FF;				// Channel number
				
				buffer[3] 	=  logic  & 0x000000FF;				// Logic function
				
				buffer[4] 	= (value&0xFF000000)>>24;			// Trigger value
				buffer[5] 	= (value&0x00FF0000)>>16;	
				buffer[6] 	= (value&0x0000FF00)>>8;	
				buffer[7] 	=  value&0x000000FF;
				
				buffer[8] 	= (freq_d&0x0000FF00)>>8;	  		// Frequency divider     
				buffer[9] 	=  freq_d&0x000000FF;

				checksum = Fletcher16(buffer, 10);
	
				buffer[10] 	=  (checksum&0xFF00)>>8; 
				buffer[11] 	=   checksum&0x00FF; 

				messageSize = 12;
				bytesToWrite = messageSize;

				while (bytesToWrite > 0)
				{
					bytesWritten = ClientTCPWrite (conversation, &buffer[messageSize - bytesToWrite], bytesToWrite, 0);
					bytesToWrite -= bytesWritten;
				}
				
				SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, 1);
				
				SetCtrlAttribute (panelHandle, PANEL_START_BUTTON, ATTR_DIMMED, 1); 

				 
			break;
	}
	return 0;
}


int CVICALLBACK stop_acquisition (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	
	unsigned char buffer[32] = {0};
	int bytesToWrite;
	int bytesWritten;
	int messageSize;
	unsigned short checksum; 
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			
			buffer[0] = 0xDD;								
			buffer[1] = 0x1;							
			buffer[2] = 0x0;
			
			checksum = Fletcher16(buffer, 3);

			buffer[3] 	=  (checksum&0xFF00)>>8; 
			buffer[4] 	=   checksum&0x00FF; 

			messageSize = 5;
			bytesToWrite = messageSize;

			while (bytesToWrite > 0)
			{
				bytesWritten = ClientTCPWrite (conversation, &buffer[messageSize - bytesToWrite], bytesToWrite, 0);
				bytesToWrite -= bytesWritten;
			}
			
			SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, 0);
						
			SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_LABEL_TEXT, "Stopped");
			SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_OFF_COLOR,  0xCC0000);
			SetCtrlVal (panelHandle, PANEL_STATUS_LED, 0);
			
			SetCtrlAttribute (panelHandle, PANEL_START_BUTTON, ATTR_DIMMED, 0);
			
			DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);

			break;
	}
	return 0;
}


unsigned short Fletcher16( unsigned char * data, int count)
{
	unsigned short sum1 = 0xff;
	unsigned short sum2 = 0xff;


	while (count)
	{
	    int tlen = count > 20 ? 20 : count ;
	    count -= tlen;
	    do {
	            sum2 += sum1 += *data++;
	    } while (--tlen);
	    sum1 = (sum1 & 0xff) + (sum1 >> 8);
	    sum2 = (sum2 & 0xff) + (sum2 >> 8);
	}
	
	/* Second reduction step to reduce sums to 8 bits */
	
	sum1 = (sum1 & 0xff) + (sum1 >> 8);
	sum2 = (sum2 & 0xff) + (sum2 >> 8);
	return sum2 << 8 | sum1;
}


int CVICALLBACK freq_div_CB (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	unsigned short temp;  
		
	switch (event)
	{
		case EVENT_COMMIT:
			
			
				GetCtrlVal (panelHandle, PANEL_FREQ_DIV, &temp);
				SetCtrlVal (panelHandle, PANEL_US_CONV, 20*temp/1000);

			break;
	}
	return 0;
}

int CVICALLBACK timer_CB (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	
	unsigned char buffer[32] = {0};
	int bytesToWrite;
	int bytesWritten;
	int messageSize;
	unsigned short checksum; 
	
	switch (event)
	{
		case EVENT_TIMER_TICK:
			
				buffer[0] = 0xDE;								
				buffer[1] = 0x1;							
				buffer[2] = 0x0;
			
				checksum = Fletcher16(buffer, 3);

				buffer[3] 	=  (checksum&0xFF00)>>8; 
				buffer[4] 	=   checksum&0x00FF; 

				messageSize = 5;
				bytesToWrite = messageSize;

				while (bytesToWrite > 0)
				{
					bytesWritten = ClientTCPWrite (conversation, &buffer[messageSize - bytesToWrite], bytesToWrite, 0);
					bytesToWrite -= bytesWritten;
				}
			
				ping = !ping;
				
				SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_LABEL_TEXT, "Trigger armed");
				SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_OFF_COLOR,  0x143200); 
				SetCtrlAttribute (panelHandle, PANEL_STATUS_LED, ATTR_ON_COLOR, 0x8CFF00); 
				
				if (ping)
				{
					SetCtrlVal (panelHandle, PANEL_STATUS_LED, 1);  	
				}
				else
				{
					SetCtrlVal (panelHandle, PANEL_STATUS_LED, 0);  	
				}
			
				


			break;
	}
	return 0;
}
