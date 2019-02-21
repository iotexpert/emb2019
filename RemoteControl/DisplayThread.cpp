#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "Adafruit_SSD1306.h"
#include "cypressLogo.h"
#include "mouserLogo.h"

#define dbg_printf(...)
//#define dbg_printf printf

class I2CPreInit : public I2C
{
public:
I2CPreInit(PinName sda, PinName scl) : I2C(sda, scl)
{
	frequency(400000);
	start();
};
};

Queue<DisplayMessage_t, 10> displayQueue;
MemoryPool<DisplayMessage_t, 10> displayPool;
int bleTimerId;
int wifiTimerId;

I2CPreInit gI2C(P6_1,P6_0);
Adafruit_SSD1306_I2c display(gI2C,P0_2,0x78,64,128);
DigitalOut bleLED(WIFI_LED);
DigitalOut wifiLED(BT_LED);

void bleLedTimer()
{
	bleLED = !bleLED;
}

void wifiLedTimer()
{
	wifiLED = !wifiLED;
}


void drawSplash( void )
{
	display.drawX11BitMap(cypressLogo,cypressLogoWidth,sizeof(cypressLogo),0,(64-cypressLogoHeight)/2);
	display.display();
	wait(2.0);       // Cypress Logo 2 Seconds
	display.drawX11BitMap(mouserLogo,mouserLogoWidth,sizeof(mouserLogo),0,(64-mouserLogoHeight)/2);
	display.display();
	wait(2.0);       // Mouser Logo 2 Seconds


	display.clearDisplay();
	display.printAt(5,0,Adafruit_GFX::ALIGN_CENTER,2,(char *)"Cypress");
	display.printAt(5,1,Adafruit_GFX::ALIGN_CENTER,2,(char *)"Choose:");
	display.printAt(5,2,Adafruit_GFX::ALIGN_CENTER,2,(char *)"Bluetooth");
	display.printAt(5,3,Adafruit_GFX::ALIGN_CENTER,2,(char *)"and WiFi");
	display.display();

}



void drawBle( DisplayType_t type )
{
	EventQueue *queue = mbed_event_queue();
	switch (type)
	{
	case BLE_START:
		queue->cancel(wifiTimerId);
		dbg_printf("DisplayThread: WiFi Timer ID Cancel\n");
		wifiLED = LED_OFF;             // turn it off

		display.clearDisplay();
		display.printAt(10,0,Adafruit_GFX::ALIGN_CENTER,1,"Start BLE");
		break;

	case BLE_ADVERTISE:
		display.printAt(10,2,Adafruit_GFX::ALIGN_CENTER,1,"Start Advertise");
		break;
	case BLE_CONNECT:
		display.printAt(10,3,Adafruit_GFX::ALIGN_CENTER,1,"Connect Done");
		break;
	case BLE_SERVICE_DISC:
		display.printAt(10,4,Adafruit_GFX::ALIGN_CENTER,1,"Service Disc Done");
	break;
	case REGISTER_NOTIFY:
		display.printAt(10,5,Adafruit_GFX::ALIGN_CENTER,1,"Notify on");
		break;

	default:
		dbg_printf("DisplayThread: Function drawBle Missing case %u",(unsigned int)type);
		break;
	}
	display.display();

}

void drawWiFi(  DisplayType_t type )
{
	EventQueue *queue = mbed_event_queue();

	switch (type)
	{
	case WIFI_CONNECT:
		queue->cancel(bleTimerId);
		bleLED = LED_OFF;             // off

		display.clearDisplay();
		display.printAt(10,1,Adafruit_GFX::ALIGN_CENTER,1,"Connecting WiFi");
		break;
	case AWS_RESOURCES:
		display.printAt(10,2,Adafruit_GFX::ALIGN_CENTER,1,"Loading Resources");
		break;
	case AWS_CONNECT:
		display.printAt(10,3,Adafruit_GFX::ALIGN_CENTER,1,"Connecting AWS");
		break;
	case SUBSCRIBE_SHADOW:
		display.printAt(10,4,Adafruit_GFX::ALIGN_CENTER,1,"Subscribing");
		break;

	default:
		dbg_printf("DisplayThread: Function drawWiFi Missing case %u",type);
		break;
	}
	display.display();
}


void drawGame( DisplayMessage_t *message )
{
	EventQueue *queue = mbed_event_queue();

	char msg[22];
	switch(message->type)
	{
	case INIT_BLE:
	case INIT_WIFI:
		display.clearDisplay();
		display.printAt(10,1,Adafruit_GFX::ALIGN_CENTER,1,"Game On!");
		display.printAt(10,2,Adafruit_GFX::ALIGN_CENTER,1,"Water Level");

		if(message->type == INIT_BLE)
		{
			queue->cancel(bleTimerId);
			bleLED = LED_ON;
			wifiLED = LED_OFF;
		}
		else
		{
			queue->cancel(wifiTimerId);
			bleLED = LED_OFF;
			wifiLED = LED_ON;
		}
		break;

	case WATER_VALUE:
		snprintf(msg, sizeof(msg), "L:%3d R:%3d",(int)message->val1, (int)message->val2);
		display.printAt(10,5,Adafruit_GFX::ALIGN_CENTER,1,msg);
		break;

	default:
		dbg_printf("DisplayThread: Function drawGame Missing case %u",message->type);
		break;
	}
	display.display();
}

void drawSwipe(DisplayMessage_t *message)
{
	dbg_printf("DisplayThread: draw swipe %d\n",(int)message->val1);
	char msg[22];
	snprintf(msg, sizeof(msg), "Swipe: %4d", (int)message->val1);
	display.printAt(10,6,Adafruit_GFX::ALIGN_CENTER,1,msg);
	display.display();
}


void testDrawBLE()
{
	dbg_printf("TestDraw BLE\n");
	DisplayMessage_t *msg;

	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_START;
	displayQueue.put(msg);
	dbg_printf(DisplayThread: "BLE_START\n");
	wait(5.0);

	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_ADVERTISE;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: BLE_ADVERTISE\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_CONNECT;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: BLE_CONNECT\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = BLE_SERVICE_DISC;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: BLE_SERVICE_DISC\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = BLE_SCREEN;
	msg->type = REGISTER_NOTIFY;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: REGISTER_NOTIFY\n");
	wait(1.0);
}

void testDrawWiFi()
{
	dbg_printf("TestDraw WiFi\n");
	DisplayMessage_t *msg;

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = WIFI_CONNECT;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: WIFI_CONNECT\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = AWS_RESOURCES;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: AWS_RESOURCES\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = AWS_CONNECT;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: AWS_CONNECT\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = SUBSCRIBE_SHADOW;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: SUBSCRIBE_SHADOW\n");
	wait(1.0);

}

void testDrawGame()
{
	dbg_printf("TestDraw drawGame\n");
	DisplayMessage_t *msg;

	msg = displayPool.alloc();
	msg->command = GAME_SCREEN;
	msg->type = INIT_BLE;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: GAME_SCREEN INIT_BLE\n");
	wait(1.0);

	msg = displayPool.alloc();
	msg->command = GAME_SCREEN;
	msg->type = INIT_WIFI;
	displayQueue.put(msg);
	dbg_printf("DisplayThread: GAME_SCREEN INIT_WIFI\n");
	wait(1.0);

	for(int i=0; i<=100; i=i+5)
	{
		msg = displayPool.alloc();
		msg->command = GAME_SCREEN;
		msg->type = WATER_VALUE;
		msg->val1 = i;
		msg->val2 = 100-i;

		displayQueue.put(msg);
		dbg_printf("DisplayThread: GAME_SCREEN WATER_VALUE %d %d\n",i,100-i);
		wait(0.2);
	}
}

void testSwipe()
{

	dbg_printf("testswipe\n");
	DisplayMessage_t *msg;

	for(int i=-100; i<=100; i=i+5)
	{
		msg = displayPool.alloc();
		msg->command = SWIPE_VALUE;
		//msg->type = WATER_VALUE;
		msg->val1 = i;

		displayQueue.put(msg);
		dbg_printf("DisplayThread: Swipe value %d %d\n",i);
		wait(0.2);
	}


}


void runTest()
{
	testDrawBLE();
	testDrawWiFi();
	testDrawGame();
	testSwipe();

}

void displayThread()
{

	Thread queueThreadHandle;

	EventQueue *queue = mbed_event_queue();

	bleTimerId = queue->call_every(100, bleLedTimer);
	wifiTimerId = queue->call_every(99, wifiLedTimer);

	dbg_printf("DisplayThread: WiFi = %d BLE=%d\n",wifiTimerId,bleTimerId);
	drawSplash();
	gameMode = MODE_CONNECT;

	//queue->call_in(5000,runTest); // test the Screens

	while(1)
	{
		DisplayMessage_t *msg;

		osEvent evt = displayQueue.get();
		dbg_printf("DisplayThread: Event Status = %d\n",evt.status);
		if (evt.status == osEventMessage) {
			msg = (DisplayMessage_t *)evt.value.p;

			switch (msg->command)
			{
			case BLE_SCREEN:
				drawBle(msg->type);
				displayPool.free(msg);

				break;
			case WIFI_SCREEN:
				drawWiFi(msg->type);
				displayPool.free(msg);

				break;
			case GAME_SCREEN:
				drawGame(msg);
				displayPool.free(msg);

				break;
			case SWIPE_VALUE:
				drawSwipe(msg);
				displayPool.free(msg);
				break;
			}
		}
	}
}
