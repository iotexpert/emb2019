
#include "mbed.h"
#include "WifiThread.h"
#include "aws_common.h"
#include "aws_client.h"
#include "cjson.h"
#include "awskeys.h"
#include "wsleddma.h"

//#define dbg_printf(...)
#define dbg_printf printf

#define SUBSCRIBE_TOPIC                     "LEDtopic"

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
  uint8_t percent = 0;
  char buff[128];
  aws_message_t &message = md.message;
  cJSON *root;
  cJSON *left;
  cJSON *right;

  char *payload = (char *)message.payload;
  for(unsigned int i=0; i<message.payloadlen; i++)
      buff[i] = payload[i];

  buff[message.payloadlen] = 0;

  dbg_printf("WiFiThread:%s=%s\n",SUBSCRIBE_TOPIC,buff);

  root = cJSON_Parse(buff);
  if(root == 0)
    return;
  left = cJSON_GetObjectItem(root,"left");
  if(left != 0)
    {
      percent = (uint8_t)(left->valuedouble);
      WS_fill_side(LEFT_STRING, WS_percent_toLEDs(percent), 0x00, 0xFF, 0x00);
      dbg_printf( "WiFiThread: Left LEDs set to %d\n", percent );
    }
  right = cJSON_GetObjectItem(root,"right");
  if(right != 0)
    {
      percent = (uint8_t)(right->valuedouble);
      WS_fill_side(RIGHT_STRING, WS_percent_toLEDs(percent), 0x00, 0x00, 0xFF);
      dbg_printf( "WiFiThread: Right LEDs set to %d\n", percent );
    }

  cJSON_Delete(root);       /* Free up memory */
}


void wifiThread()
{
  dbg_printf("WiFiThread:Started WiFI Thread\n");

  wifi = WiFiInterface::get_default_instance();
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
      uniquenum += tempName[i];

  sprintf(myName,"R%04X",(unsigned int)uniquenum);

  dbg_printf("WiFiThread: myName %s\n",myName);
  dbg_printf("WiFiThread:IP: %s\n", wifi->get_ip_address());
  dbg_printf("WiFiThread:Netmask: %s\n", wifi->get_netmask());
  dbg_printf("WiFiThread:Gateway: %s\n", wifi->get_gateway());
  dbg_printf("WiFiThread:RSSI: %d\n", wifi->get_rssi());

  aws_connect_params conn_params = { 0,0,NULL,NULL,NULL,NULL,NULL };

  client.set_command_timeout( 5000 );
  conn_params.keep_alive = AWS_IOT_KEEPALIVE_TIMEOUT;
  conn_params.client_id = (uint8_t*)myName;

  dbg_printf("WiFiThread:AWS_CONNECT\n");
  SocketAddress mySocket;

  wifi->gethostbyname(AWS_IOT_ENDPOINT_ADDRESS, &mySocket, NSAPI_IPv4);

  dbg_printf("WiFiThread:Host URL:%s IP=%s\n", AWS_IOT_ENDPOINT_ADDRESS, mySocket.get_ip_address());

  AWSIoTClient client(wifi, myName, SSL_CLIENTKEY_PEM, strlen(SSL_CLIENTKEY_PEM), SSL_CLIENTCERT_PEM, strlen(SSL_CLIENTCERT_PEM));

  ep = client.create_endpoint(AWS_TRANSPORT_MQTT_NATIVE, AWS_IOT_ENDPOINT_ADDRESS, AWS_IOT_SECURE_PORT, SSL_CA_PEM, strlen(SSL_CA_PEM));
  client.connect (ep, conn_params);

  dbg_printf("WiFiThread:SUBSCRIBE to %s\n",SUBSCRIBE_TOPIC);

	client.subscribe (ep, SUBSCRIBE_TOPIC, AWS_QOS_ATMOST_ONCE, messageArrived);

	while(1)
	  {
	    client.yield(AWS_IOT_TIMEOUT);
	  }
}
