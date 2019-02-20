
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

#define AWS_TIMEOUT 		100
#define AWS_CONNECT_TIMEOUT 10000

#define AWS_PUB_DELAY    					          (5000)
#define PUBLISHER_CERTIFICATES_MAX_SIZE     (0x7fffffff)

#define SUBSCRIBE_TOPIC                     "TEST1"
#define PUMP_KICK_TOPIC					            "PUMP_KICK_TOPIC"

#define APP_PUBLISH_RETRY_COUNT             (5)
#define AWSIOT_KEEPALIVE_TIMEOUT            (6000)
#define AWS_IOT_SECURE_PORT                 (8883)
#define AWSIOT_TIMEOUT                      (10)
#define AWSIOT_ENDPOINT_ADDRESS             "amk6m51qrxr2u-ats.iot.us-east-1.amazonaws.com"

WiFiInterface *wifi;
AWSIoTEndpoint* ep = NULL;
AWSIoTClient client;

char myName[10];

static void messageArrived( aws_iot_message_t& md)
{
  aws_message_t &message = md.message;
	cJSON *root;
	cJSON *left;
	cJSON *right;

  dbg_printf("WiFiThread:Message length = %d\n",message.payloadlen);


  char buff[48];
  char *payload = (char *)message.payload;
  if(message.payloadlen > sizeof(buff)-1)
    return;
  for(int i=0;i<message.payloadlen;i++)
    buff[i] = payload[i];
  buff[message.payloadlen] = 0;

  dbg_printf("WiFiThread:%s=%s\n",SUBSCRIBE_TOPIC,buff);

  return;
  root = cJSON_Parse((char*) message.payload);
  left = cJSON_GetObjectItem(root,"left");
  right = cJSON_GetObjectItem(root,"right");
  uint8_t leftValue=0;
  uint8_t rightValue= 0;

  if(left && cJSON_IsNumber(left))
  {
    leftValue = left->valueint;
  }

  if(right && cJSON_IsNumber(right))
  {
    rightValue = right->valueint;
  }
  cJSON_Delete(root);

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
  for(int i=0;i<strlen(tempName);i++)
  {
      uniquenum += tempName[i];
  }
  sprintf(myName,"R%04X",uniquenum);

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

  /* set MQTT connection parameters */
  conn_params.username = NULL;
  conn_params.password = NULL;
  conn_params.keep_alive = AWSIOT_KEEPALIVE_TIMEOUT;
  conn_params.peer_cn = NULL;
  conn_params.client_id = (uint8_t*)myName;


  msg = displayPool.alloc();
  msg->command = WIFI_SCREEN;
  msg->type = AWS_CONNECT;
  displayQueue.put(msg);
  dbg_printf("WiFiThread:AWS_CONNECT\n");

  SocketAddress mySocket;
  dbg_printf("WiFiThread:Attempting to connect to endpoint:%s\n",AWSIOT_ENDPOINT_ADDRESS);
  wifi->gethostbyname(AWSIOT_ENDPOINT_ADDRESS, &mySocket, NSAPI_IPv4);
  dbg_printf("WiFiThread:Host URL : %s\n", AWSIOT_ENDPOINT_ADDRESS);
  dbg_printf("WiFiThread:IP address : %s\n", mySocket.get_ip_address());
  dbg_printf("WiFiThread:Port number : %d\n", mySocket.get_port());
  dbg_printf("WiFiThread:IP version : %d\n\n", mySocket.get_ip_version());

  AWSIoTClient client(wifi, myName , SSL_CLIENTKEY_PEM, strlen(SSL_CLIENTKEY_PEM), SSL_CLIENTCERT_PEM, strlen(SSL_CLIENTCERT_PEM));

  ep = client.create_endpoint(AWS_TRANSPORT_MQTT_NATIVE, AWSIOT_ENDPOINT_ADDRESS, AWS_IOT_SECURE_PORT, SSL_CA_PEM, strlen(SSL_CA_PEM));

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

  result = client.subscribe (ep, SUBSCRIBE_TOPIC, AWS_QOS_ATMOST_ONCE, messageArrived);
  if(result == AWS_SUCCESS)
  {
    dbg_printf ("WiFiThread:Subscribed to topic successfully \n");
  }
  else
  {
      dbg_printf ("WiFiThread:Subscription to MQTT topic failed \n");
  }

  msg = displayPool.alloc();
  msg->command = GAME_SCREEN;
  msg->type = INIT_WIFI;
  displayQueue.put(msg);

  gameMode = MODE_GAME;

  while(1)
  {
    int32_t *swipeMsg;
    osEvent evt = swipeQueue.get(10);
    if (evt.status == osEventMessage) {
        dbg_printf("WiFiThread:Event Status = %d\n",evt.status);

        char buff[20];
        swipeMsg = (int32_t *)evt.value.p;
        dbg_printf("WiFiThread:Received swipe=%d\n",*swipeMsg);
        if(*swipeMsg < 0)
          sprintf(buff,"{\"left\":%d}",*swipeMsg * -1);
        else
          sprintf(buff,"{\"right\":%d}",*swipeMsg);

        swipePool.free(swipeMsg);

          int pub_retries = 0;
          do
          {
              dbg_printf("WiFiThread:Trying to publish = %s\n",buff);
              result = client.publish(ep, PUMP_KICK_TOPIC, buff, strlen(buff), publish_params);
              dbg_printf("WiFiThread: result= %d retry=%d\n",result,pub_retries);
              pub_retries++ ;
          } while ( ( result != AWS_SUCCESS )  && ( pub_retries < APP_PUBLISH_RETRY_COUNT ) );
    }
    client.yield(AWSIOT_TIMEOUT);

  }
}
