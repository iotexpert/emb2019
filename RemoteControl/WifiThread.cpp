
#include "mbed.h"
#include "global.h"
#include "DisplayThread.h"
#include "CapSenseThread.h"
#include "WifiThread.h"
#include "aws_common.h"
#include "aws_client.h"
#include "cJSON.h"
#include "awskeys.h"

//#define dbg_printf(...)
#define dbg_printf printf

#define SLIDER_POS_TOPIC                    "LEDtopic"
#define SUBSCRIBE_TOPIC                     "$aws/things/Electronica2018/shadow/update"
#define PUMP_KICK_TOPIC                                             "PumpAWS"

#define APP_PUBLISH_RETRY_COUNT             (5)
#define AWS_IOT_KEEPALIVE_TIMEOUT           (600)
#define AWS_IOT_SECURE_PORT                 (8883)
#define AWS_CONNECT_TIMEOUT                 (10000)
#define AWS_IOT_TIMEOUT                     (10)
#define AWS_IOT_ENDPOINT_ADDRESS            "amk6m51qrxr2u-ats.iot.us-east-1.amazonaws.com"

WiFiInterface *wifi;
AWSIoTEndpoint* ep = NULL;
AWSIoTClient client;
char myName[10];

static void messageArrived( aws_iot_message_t& md)
{
	uint8_t waterLeft  = 0;
	uint8_t waterRight = 0;
	char buff[128];
	aws_message_t &message = md.message;
	cJSON *root;
	cJSON *state;
	cJSON *reported;

	char *payload = (char *)message.payload;

	for(unsigned int i=0; i<message.payloadlen; i++)
	{
		buff[i] = payload[i];
	}
	buff[message.payloadlen] = 0;

	dbg_printf("WiFiThread:%s=%s\n",SUBSCRIBE_TOPIC,buff);

	root = cJSON_Parse(buff);
	if(root == 0)
		return;
	state = cJSON_GetObjectItem(root,"state");
	if(state == 0)
		return;
	reported = cJSON_GetObjectItem(state,"reported");
	if(reported == 0)
		return;

	waterLeft = (uint8_t) cJSON_GetObjectItem(reported,"WaterLevelLeftAWS")->valuedouble;
	waterRight = (uint8_t) cJSON_GetObjectItem(reported,"WaterLevelRightAWS")->valuedouble;
	cJSON_Delete(root);                   /* Free up memory */
	dbg_printf( "WiFiThread: Water Left: %d\t Water Right: %d\n", waterLeft, waterRight );

	DisplayMessage_t *msg;
	msg = displayPool.alloc();
	msg->command = GAME_SCREEN;
	msg->type = WATER_VALUE;
	msg->val1 = waterLeft;
	msg->val2 = waterRight;
	displayQueue.put(msg);


}


void wifiThread()
{

	dbg_printf("WiFiThread:Started WiFI Thread\n");
	DisplayMessage_t *msg;

	wifi = WiFiInterface::get_default_instance();
	if (!wifi) {
		dbg_printf("WiFiThread:ERROR: No WiFiInterface found.\n");
		while(1);
	}


	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = WIFI_CONNECT;
	displayQueue.put(msg);

	int ret;

	do {
		dbg_printf("WiFiThread:Connecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
		ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
		if(ret !=0)
		{
			dbg_printf("WiFiThread:WiFi could not connect\n");
			wait(0.5);
		}
	} while(ret != 0);

	dbg_printf("WiFiThread:MAC: %s\n", wifi->get_mac_address());

	const char *tempName = wifi->get_mac_address();

	uint32_t uniquenum=0;
	for(unsigned int i=0; i<strlen(tempName); i++)
	{
		uniquenum += tempName[i];
	}
	sprintf(myName,"R%04X",(unsigned int)uniquenum);

	dbg_printf("WiFiThread: myName %s\n",myName);

	dbg_printf("WiFiThread:IP: %s\n", wifi->get_ip_address());
	dbg_printf("WiFiThread:Netmask: %s\n", wifi->get_netmask());
	dbg_printf("WiFiThread:Gateway: %s\n", wifi->get_gateway());
	dbg_printf("WiFiThread:RSSI: %d\n", wifi->get_rssi());

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = AWS_RESOURCES;
	displayQueue.put(msg);
	dbg_printf("WiFiThread:AWS_RESOURCES\n");

	aws_connect_params conn_params = { 0,0,NULL,NULL,NULL,NULL,NULL };
	aws_publish_params publish_params = { AWS_QOS_ATMOST_ONCE };
	AWS_error result = AWS_SUCCESS;

	client.set_command_timeout( 5000 );
	conn_params.username = NULL;
	conn_params.password = NULL;
	conn_params.keep_alive = AWS_IOT_KEEPALIVE_TIMEOUT;
	conn_params.peer_cn = NULL;
	conn_params.client_id = (uint8_t*)myName;

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = AWS_CONNECT;
	displayQueue.put(msg);

	dbg_printf("WiFiThread:AWS_CONNECT\n");
	SocketAddress mySocket;
	dbg_printf("WiFiThread:Attempting to connect to endpoint:%s\n",AWS_IOT_ENDPOINT_ADDRESS);
	wifi->gethostbyname(AWS_IOT_ENDPOINT_ADDRESS, &mySocket, NSAPI_IPv4);
	dbg_printf("WiFiThread:Host URL : %s\n", AWS_IOT_ENDPOINT_ADDRESS);
	dbg_printf("WiFiThread:IP address : %s\n", mySocket.get_ip_address());
	dbg_printf("WiFiThread:Port number : %d\n", mySocket.get_port());
	dbg_printf("WiFiThread:IP version : %d\n\n", mySocket.get_ip_version());

	AWSIoTClient client(wifi, myName, SSL_CLIENTKEY_PEM, strlen(SSL_CLIENTKEY_PEM), SSL_CLIENTCERT_PEM, strlen(SSL_CLIENTCERT_PEM));

	ep = client.create_endpoint(AWS_TRANSPORT_MQTT_NATIVE, AWS_IOT_ENDPOINT_ADDRESS, AWS_IOT_SECURE_PORT, SSL_CA_PEM, strlen(SSL_CA_PEM));
	result = client.connect (ep, conn_params);

	if(result == AWS_SUCCESS)
	{
		dbg_printf("WiFiThread:Connected to AWS endpoint\n");
	}

	msg = displayPool.alloc();
	msg->command = WIFI_SCREEN;
	msg->type = SUBSCRIBE_SHADOW;
	displayQueue.put(msg);
	dbg_printf("WiFiThread:SUBSCRIBE to %s\n",SUBSCRIBE_TOPIC);

	if(positionMode == false)
	{
		result = client.subscribe (ep, SUBSCRIBE_TOPIC, AWS_QOS_ATMOST_ONCE, messageArrived);
		if(result == AWS_SUCCESS)
		{
			dbg_printf ("WiFiThread:Subscribed to topic successfully \n");
		}
		else
		{
			dbg_printf ("WiFiThread:Subscription to MQTT topic failed \n");
		}
	}


	msg = displayPool.alloc();
	msg->command = GAME_SCREEN;
	msg->type = INIT_WIFI;
	displayQueue.put(msg);

	gameMode = MODE_GAME;

	while(1)
	{
		char buff[20];

		int32_t *swipeMsg;
		osEvent evt = swipeQueue.get(AWS_IOT_TIMEOUT);
		if (evt.status == osEventMessage) {

			if(positionMode)
			{
				swipeMsg = (int32_t *)evt.value.p;
				dbg_printf("Position = %d\n",*swipeMsg);
				sprintf(buff,"{\"left\":%d}",(int)(*swipeMsg));
				int pub_retries = 0;
				do
				{
					dbg_printf("WiFiThread:Trying to publish = %s\n",buff);
					result = client.publish(ep, SLIDER_POS_TOPIC, buff, strlen(buff), publish_params);
					dbg_printf("WiFiThread: result= %d retry=%d\n",result,pub_retries);
					pub_retries++;
				} while ( ( result != AWS_SUCCESS )  && ( pub_retries < 1 ) );
				swipePool.free(swipeMsg);

			}
			else
			{
				swipeMsg = (int32_t *)evt.value.p;
				dbg_printf("WiFiThread:Received swipe=%d\n",(int)*swipeMsg);
				if(*swipeMsg < 0)
					sprintf(buff,"{\"left\":%d}",(int)(*swipeMsg * -1));
				else
					sprintf(buff,"{\"right\":%d}",(int)(*swipeMsg));

				swipePool.free(swipeMsg);

				int pub_retries = 0;
				do
				{
					dbg_printf("WiFiThread:Trying to publish = %s\n",buff);
					result = client.publish(ep, PUMP_KICK_TOPIC, buff, strlen(buff), publish_params);
					dbg_printf("WiFiThread: result= %d retry=%d\n",result,pub_retries);
					pub_retries++;
				} while ( ( result != AWS_SUCCESS )  && ( pub_retries < APP_PUBLISH_RETRY_COUNT ) );
			}
		}

		// backup plan

		client.yield(AWS_IOT_TIMEOUT);
	}
}
