/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

#include <certificate.h>
#include "greentea-client/test_env.h"
#include "unity.h"
#include "utest.h"
#include "rtos.h"
#include "mbed.h"
#include "aws_client.h"
using namespace utest::v1;

NetworkInterface* network;
AWSIoTEndpoint* ep = NULL;
AWSIoTClient client;

#define APP_INFO( x )  						printf x

#define AWSIOT_KEEPALIVE_TIMEOUT 			(60)
#define AWSIOT_CLIENT_ID 					"AwsIoT publisher APP"
#define AWSIOT_PUBLISHER_TOPIC 				"WICED_BULB"
#define AWSIOT_MESSAGE 						"HELLO"
#define AWSIOT_TIMEOUT       				(1000)
#define AWS_VALID_HOSTNAME                  "a38td4ke8seeky-ats.iot.us-east-1.amazonaws.com"
#define AWS_INVALID_HOSTNAME                "asdfasdfsad-asdfasts.iot.us-asdf-1.amazonaws.com"
#define AWS_IOT_SECURE_PORT  				(8883)
#define AWS_IOT_INVALID_SECURE_PORT  		(1234)

typedef enum test_failure {
	STATUS_INIT,
	NETWORK_CONNECTION_FAILED,
	NETWORK_CONNECTION_SUCCESS,
	AWSIOT_CONNECTION_FAILED,
	AWSIOT_CONNECTION_SUCCESS,
	AWSIOT_SUBSCRIBTION_FAILED,
	AWSIOT_SUBSCRIBTION_SUCCESS,
	AWSIOT_PUBLISH_FAILED,
	AWSIOT_PUBLISH_SUCCESS,
	AWSIOT_UNSUBSCRIBE_FAILED,
	AWSIOT_UNSUBSCRIBE_SUCCESS,
	AWSIOT_DISCONNECT_FAILED,
	AWSIOT_DISCONNECT_SUCCESS,
} test_failure_t;

test_failure_t test_status = STATUS_INIT;
AWS_error test_case(const char* root_ca, uint16_t root_ca_length,
		const char* private_key, uint16_t key_length, const char* certificate,
		uint16_t certificate_length, char* hostname, int port) {
	aws_connect_params conn_params = { 0, 0, NULL, NULL, NULL, NULL, NULL };
	AWS_error result = AWS_SUCCESS;

	test_status = STATUS_INIT;

	APP_INFO(("Connecting to the network using Wifi...\r\n"));
	network = NetworkInterface::get_default_instance();

	nsapi_error_t net_status = -1;
	for (int tries = 0; tries < 3; tries++) {
		net_status = network->connect();
		if (net_status == NSAPI_ERROR_OK) {
			test_status = NETWORK_CONNECTION_SUCCESS;
			break;
		} else {
			test_status = NETWORK_CONNECTION_FAILED;
			APP_INFO(("Unable to connect to network. Retrying...\r\n"));
		}
	}

	if (net_status != NSAPI_ERROR_OK) {
		test_status = NETWORK_CONNECTION_FAILED;
		APP_INFO(
				("ERROR: Connecting to the network failed (%d)!\r\n", net_status));
		return result;
	}
	test_status = NETWORK_CONNECTION_SUCCESS;

	APP_INFO(
			("Connected to the network successfully. IP address: %s\n", network->get_ip_address()));

	/* Initialize AWS Client library */
	AWSIoTClient client(network, "wiced_ggd_1", private_key, key_length,
			certificate, certificate_length);

	ep = client.create_endpoint(AWS_TRANSPORT_MQTT_NATIVE, hostname, port,
			root_ca, root_ca_length);

	client.set_command_timeout(5000);

	/* set MQTT connection parameters */
	conn_params.username = NULL;
	conn_params.password = NULL;
	conn_params.keep_alive = AWSIOT_KEEPALIVE_TIMEOUT;
	conn_params.peer_cn = NULL;
	conn_params.client_id = (uint8_t*) AWSIOT_CLIENT_ID;

	/* connect to an AWS endpoint */
	result = client.connect(ep, conn_params);
	if (result != AWS_SUCCESS) {
		APP_INFO(("connection to AWS endpoint failed \r\n"));
		test_status = AWSIOT_CONNECTION_FAILED;
		delete ep;
		ep = NULL;
		network->disconnect();

		return result;
	}
	test_status = AWSIOT_CONNECTION_SUCCESS;
	APP_INFO(("Connected to AWS endpoint \r\n"));


	result = client.disconnect(ep);
	if (result != AWS_SUCCESS) {
		test_status = AWSIOT_DISCONNECT_FAILED;

		delete ep;
		ep = NULL;
		APP_INFO(("disconnection to AWS endpoint failed \r\n"));
		network->disconnect();

		return result;
	}
	test_status = AWSIOT_DISCONNECT_SUCCESS;

	APP_INFO(("disconnection to AWS endpoint \r\n"));
	network->disconnect();

	return result;
}

void test_valid_root_ca(void) {
	test_case(SSL_CA_PEM, strlen(SSL_CA_PEM), SSL_CLIENTKEY_PEM,
			strlen(SSL_CLIENTKEY_PEM), SSL_CLIENTCERT_PEM,
			strlen(SSL_CLIENTCERT_PEM), AWS_VALID_HOSTNAME,
			AWS_IOT_SECURE_PORT);
	if (test_status >= AWSIOT_CONNECTION_SUCCESS) {
		APP_INFO(("\r\n Pass valid root CA certificate, client private key & establish connection to AWS Iot endpoint successfully "));
		APP_INFO(("***Test success*** \r\n"));
		TEST_ASSERT(true);
	} else {
		APP_INFO(
				("\r\nPass valid root CA certificate, client private key & establish connection to AWS Iot endpoint successfully"));
		APP_INFO(("***Test Failed*** \r\n"));
		TEST_ASSERT(false);
	}
}
void test_invalid_root_ca(void) {
	test_case(SSL_CA_PEM_INVALID, strlen(SSL_CA_PEM_INVALID), SSL_CLIENTKEY_PEM_INVALID,
			strlen(SSL_CLIENTKEY_PEM_INVALID), SSL_CLIENTCERT_PEM_INVALID,
			strlen(SSL_CLIENTCERT_PEM_INVALID),AWS_VALID_HOSTNAME, AWS_IOT_SECURE_PORT);

	if (test_status >= AWSIOT_CONNECTION_SUCCESS) {
		APP_INFO(("\r\n Pass invalid root CA certificate, client certificate and client private key & connection to AWS Iot endpoint should fail.."));
		APP_INFO(("***Test Failed*** \r\n"));
		TEST_ASSERT(false);
	} else {
		APP_INFO(("\r\nPass invalid root CA certificate, client certificate and client private key & connection to AWS Iot endpoint should fail.."));
		APP_INFO(("***Test Passed*** \r\n"));
		TEST_ASSERT(true);
	}
}

void test_valid_client_cert(void) {
	test_case(NULL, strlen(NULL), SSL_CLIENTKEY_PEM,
			strlen(SSL_CLIENTKEY_PEM), SSL_CLIENTCERT_PEM,
			strlen(SSL_CLIENTCERT_PEM), AWS_VALID_HOSTNAME, AWS_IOT_SECURE_PORT);
	if (test_status >= AWSIOT_CONNECTION_SUCCESS) {

		APP_INFO(
				("\r\n Pass valid client certificate and client private key & establish connection to AWS Iot endpoint successfully should fail.."));
		APP_INFO(("***Test Failed *** \r\n"));
		TEST_ASSERT(false);

	} else {
		APP_INFO(
				("\r\nPass valid client certificate and client private key & establish connection to AWS Iot endpoint successfully should fail.."));

		APP_INFO(("***Test Passed*** \r\n"));
		TEST_ASSERT(true);
	}
}

void test_invalid_client_cert(void) {
	test_case(NULL, strlen(NULL), SSL_CLIENTKEY_PEM_INVALID,
			strlen(SSL_CLIENTKEY_PEM_INVALID), SSL_CLIENTCERT_PEM_INVALID,
			strlen(SSL_CLIENTCERT_PEM_INVALID),AWS_VALID_HOSTNAME, AWS_IOT_SECURE_PORT);
	if (test_status >= AWSIOT_CONNECTION_SUCCESS) {
		APP_INFO(("\r\n Pass invalid client certificate and client private key & connection to AWS Iot endpoint should fail"));
		APP_INFO(("***Test Failed*** r\n"));
		TEST_ASSERT(false);
	} else {
		APP_INFO(
				("\r\nPass invalid client certificate and client private key & connection to AWS Iot endpoint should fail."));
		APP_INFO(("***Test Passed*** r\n"));
		TEST_ASSERT(true);
	}
}

utest::v1::status_t test_setup(const size_t number_of_cases) {
    // Setup Greentea using a reasonable timeout in seconds
    GREENTEA_SETUP(300, "default_auto");
    return verbose_test_setup_handler(number_of_cases);
}

// Test cases
Case cases[] =
		{
				Case(
						"Pass valid root CA certificate, client private key & establish connection to AWS Iot endpoint successfully..",
						test_valid_root_ca),
				Case(
						"Pass invalid root CA certificate, client certificate and client private key & connection to AWS Iot endpoint should fail..",
						test_invalid_root_ca),
				Case(
						"Pass valid client certificate and client private key & establish connection to AWS Iot endpoint successfully should fail..",
						test_valid_client_cert),
				Case(
						"Pass invalid client certificate and client private key & connection to AWS Iot endpoint should fail",
						test_invalid_client_cert)
		};

Specification specification(test_setup, cases,greentea_continue_handlers);

// Entry point into the tests
int main() {

    return !Harness::run(specification);
}
