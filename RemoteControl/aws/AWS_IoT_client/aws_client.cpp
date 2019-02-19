/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

/** @file
 *
 * Implementation for AWS IoT client library
 *
 */
#include "aws_client.h"
#include "https_request.h"

static aws_greengrass_discovery_callback_data_t discovery_data;
extern linked_list_t* group_list;

AWSIoTClient::AWSIoTClient() {};

AWSIoTClient::AWSIoTClient(NetworkInterface* network, const char* thing_name, const char* private_key, uint16_t key_length, const char* certificate, uint16_t certificate_length)
{
    /* Assign thing name and credentials to AWS client members */
    AWSIoTClient::thing_name = thing_name;
    AWSIoTClient::private_key = private_key;
    AWSIoTClient::key_length = key_length;
    AWSIoTClient::certificate = certificate;
    AWSIoTClient::certificate_length = certificate_length;

    AWSIoTClient::command_timeout = 5000;
    AWSIoTClient::network = network;
    AWSIoTClient::flag = SECURED_MQTT;
}

void AWSIoTClient::set_command_timeout( int command_timeout )
{
    AWSIoTClient::command_timeout = command_timeout;
}

AWSIoTEndpoint* AWSIoTClient::create_endpoint(aws_iot_transport_type_t transport, const char* uri, int port, const char* root_ca, uint16_t root_ca_length)
{
    AWSIoTEndpoint* ep = NULL;

	switch (transport)
    {
        case AWS_TRANSPORT_MQTT_NATIVE:
		{
            break;
        }
        case AWS_TRANSPORT_MQTT_WEBSOCKET:
        case AWS_TRANSPORT_RESTFUL_HTTPS:
        case AWS_TRANSPORT_INVALID:
        default:
        {
			return NULL;
        }
    }

    ep = new AWSIoTEndpoint();
    ep->root_ca = root_ca;
    ep->root_ca_length = root_ca_length;
    ep->transport = transport;
    ep->uri = (char*) uri;
    ep->port = port;

    return ep;
}

AWS_error AWSIoTClient::connect(AWSIoTEndpoint* ep, aws_connect_params conn_params)
{
	int rc = 0;

	mqttnetwork = new MQTTNetwork(AWSIoTClient::network, flag);
	if (mqttnetwork == NULL) {
		return AWS_ERROR;
	}

	rc = mqttnetwork->set_root_ca_certificate(ep->root_ca);
	if (rc != 0) {
		AWS_LIBRARY_ERROR (("Error in setting root ca certificate \n"));
		delete mqttnetwork;
		mqttnetwork = NULL;
		return AWS_ERROR;
	}
	rc = mqttnetwork->set_client_cert_key(AWSIoTClient::certificate,
			AWSIoTClient::private_key);
	{
		if (rc != 0) {
			AWS_LIBRARY_ERROR (("Error in setting cilent certificate and private key\n"));
			delete mqttnetwork;
			mqttnetwork = NULL;
			return AWS_ERROR;
		}
	}

	rc = mqttnetwork->connect(ep->uri, ep->port, (char*) conn_params.peer_cn);
	if (rc != 0) {
		AWS_LIBRARY_ERROR (("TLS connection to MQTT broker failed \n"));
		delete mqttnetwork;
		mqttnetwork = NULL;
		return AWS_ERROR;
	} else {

		AWS_LIBRARY_DEBUG(("TLS connection to AWS endpoint established \n"));

		mqtt_obj = new MQTT::Client<MQTTNetwork, Countdown, 200>(*mqttnetwork,
				AWSIoTClient::command_timeout);

		static MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
		data.MQTTVersion = 4;
		data.clientID.cstring = (char*) AWSIoTClient::thing_name;
		data.username.cstring = (char*) conn_params.username;
		data.password.cstring = (char*) conn_params.password;

		AWS_LIBRARY_DEBUG(("Send MQTT connect frame \n"));
		if ((rc = mqtt_obj->connect(data)) != 0) {
			AWS_LIBRARY_ERROR(("MQTT connect failed : %d\r\n", rc));
			return AWS_ERROR;
		} else {
			AWS_LIBRARY_DEBUG(("MQTT connect is successful %d\r\n", rc));
		}

		return AWS_SUCCESS;
	}
}

AWS_error AWSIoTClient::disconnect(AWSIoTEndpoint* ep)
{
	int rc = 0;
	AWS_LIBRARY_DEBUG(("Send MQTT dis-connect frame \n"));
	if ((rc = mqtt_obj->disconnect()) != 0) {
		AWS_LIBRARY_ERROR(("MQTT dis-connect failed : %d\r\n", rc));
		return AWS_ERROR;
	} else {
		AWS_LIBRARY_DEBUG(("MQTT dis-connect is successful %d\r\n", rc));
	}

	mqttnetwork->disconnect();

	delete mqttnetwork;
	mqttnetwork = NULL;

	return AWS_SUCCESS;
}

AWS_error AWSIoTClient::publish(AWSIoTEndpoint* ep, const char* topic, const char* data, uint32_t length, aws_publish_params pub_params )
{
    int rc = 0;

    MQTT::Message message;

    message.qos = (MQTT::QoS) pub_params.QoS;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)data;
    message.payloadlen = length;

    rc = mqtt_obj->publish(topic, message);
    if ( rc != 0 )
    {
        AWS_LIBRARY_ERROR(("Publish to AWS endpoint failed  : %d \n", rc ));
        return AWS_ERROR;
    }

    AWS_LIBRARY_DEBUG(("Published to AWS endpoint successfully \n"));

    return AWS_SUCCESS;
}

AWS_error AWSIoTClient::subscribe(AWSIoTEndpoint* ep, const char* topic, aws_iot_qos_level_t qos, subscriber_callback cb)
{
    int rc = 0;

    if ((rc = mqtt_obj->subscribe(topic, (MQTT::QoS)qos, cb)) != 0)
    {
        AWS_LIBRARY_ERROR(("MQTT subscribe failed %d\r\n", rc));
        return AWS_ERROR;
    }
    else
    {
        AWS_LIBRARY_DEBUG(("MQTT subscribtion successful %d\r\n", rc));
    }

    return AWS_SUCCESS;
}

/** A call to this API must be made within the keepAlive interval to keep the MQTT connection alive
 *  yield can be called if no other MQTT operation is needed.  This will also allow messages to be
 *  received.
 *  @param timeout_ms the time to wait, in milliseconds
 *  @return success code - on failure, this means the client has disconnected
 */
AWS_error AWSIoTClient::yield(unsigned long timeout_ms)
{
	int rc = -1;
    rc = mqtt_obj->yield( timeout_ms );
    if(rc != 0)
    	return AWS_ERROR;

    return AWS_SUCCESS;
}

void dump_response(HttpResponse* res)
{
    AWS_LIBRARY_DEBUG(("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str()));

    AWS_LIBRARY_DEBUG(("Headers:\n"));
    for (size_t ix = 0; ix < res->get_headers_length(); ix++)
    {
        AWS_LIBRARY_DEBUG(("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str()));
    }

    AWS_LIBRARY_DEBUG(("\nBody (%lu bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str()));

}

AWS_error AWSIoTClient::discover(aws_iot_transport_type_t transport, const char* uri, const char* root_ca, uint16_t root_ca_length, aws_greengrass_callback gg_cb)
{
    nsapi_error_t result;
    TLSSocket* socket = new TLSSocket();
    SocketAddress address;
    char* discovery_uri = NULL;

#if 0
    if( !gg_cb || !ep )
    {
        return AWS_ERROR;
    }
#endif

    result = socket->set_client_cert_key(AWSIoTClient::certificate, AWSIoTClient::private_key);
    if (result != 0) {
        AWS_LIBRARY_ERROR((" Error in initializing client certificate and key : %d \n", result));
        return AWS_ERROR;
    }

    /* Initialize RootCA certificate */
    result = socket->set_root_ca_cert((const char*) root_ca);
    if (result != 0)
    {
        printf (" Error in initializing rootCA certificate \n");
        return AWS_ERROR;
    }

    /* Resolve hostname address */
    result = network->gethostbyname(uri, &address);

    AWS_LIBRARY_INFO((" IP address of server : %s \n", address.get_ip_address()));

    /* set GreenGrass port */
    address.set_port( AWS_GG_HTTPS_SERVER_PORT);

    result = socket->open(AWSIoTClient::network);

    /* set peer common name */
    socket->set_hostname(uri);

    socket->set_timeout(5000);

    result = socket->connect(address);
    if (result != 0)
    {
        AWS_LIBRARY_ERROR((" TLS connection to server failed : %d \n", result));
        return AWS_ERROR;
    }

    AWS_LIBRARY_DEBUG((" TLS connection to server established. Connected to server \n"));

    /* Register callback for AWS discovery payload */
    wiced_JSON_parser_register_callback( json_callback_for_discovery_payload );

    /* form discovery URI to send to AWS cloud */
    uint8_t discovery_uri_length = (strlen(thing_name) + strlen(GG_REQUEST_PROTOCOL) + strlen(uri) + strlen ( GREENGRASS_DISCOVERY_HTTP_REQUEST_URI_PREFIX));

    discovery_uri = (char *) malloc( discovery_uri_length);
    memset( discovery_uri, 0, discovery_uri_length);

    /* example : https://a38td4ke8seeky-ats.iot.us-east-1.amazonaws.com/greengrass/discover/thing/wiced_ggd_1 */
    strncat( discovery_uri, GG_REQUEST_PROTOCOL, strlen(GG_REQUEST_PROTOCOL));
    strncat( discovery_uri, uri, strlen(uri));
    strncat( discovery_uri, GREENGRASS_DISCOVERY_HTTP_REQUEST_URI_PREFIX, sizeof( GREENGRASS_DISCOVERY_HTTP_REQUEST_URI_PREFIX) - 1);
    strncat( discovery_uri, thing_name, strlen(thing_name));

    AWS_LIBRARY_DEBUG(("[AWS-Greengrass] Discovery URI is: %s (len:%d)\n", discovery_uri, (int) strlen(discovery_uri)));

    HttpsRequest* get_req = new HttpsRequest(socket, HTTP_GET, discovery_uri);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
        AWS_LIBRARY_ERROR(("HttpRequest failed (error code %d)\n", get_req->get_error()));
        return AWS_ERROR;
    }

    AWS_LIBRARY_DEBUG(("\n----- HTTPS GET response -----\n"));
    dump_response(get_res);

    result = wiced_JSON_parser( (const char*)get_res->get_body_as_string().c_str(), (uint32_t) get_res->get_body_length() );
    if( result != WICED_SUCCESS )
    {
        AWS_LIBRARY_ERROR(("[AWS-Greengrass] JSON parser error\n"));
        return AWS_ERROR;
    }

    discovery_data.groups = group_list;

    if( gg_cb )
    {
        gg_cb( &discovery_data );
    }

    delete get_req;
    delete socket;
    free(discovery_uri);
    return AWS_SUCCESS;
}
