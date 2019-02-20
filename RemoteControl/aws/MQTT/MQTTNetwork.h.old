/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */
/** file
 *
 * MQTT wrapper for MQTT client library
 */
#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "mbed.h"

#define MQTT_NETWORK_DEBUG( x )  //printf x
#define MQTT_NETWORK_ERROR( x )  printf x
#define MQTT_NETWORK_INFO( x )   printf x


typedef enum {
	SECURED_MQTT,
	NON_SECURED_MQTT
} mqtt_security_flag;


class MQTTNetwork {
public:
	MQTTNetwork(NetworkInterface* aNetwork, mqtt_security_flag is_security =
			NON_SECURED_MQTT) :
			network(aNetwork) {
		is_security_enabled = is_security;

		is_security_enabled = is_security;

		if (is_security_enabled == SECURED_MQTT) {
			TLSSocket *socket;
			socket = new TLSSocket;
			socket_context = socket;

		} else {
			TCPSocket *socket;
			socket = new TCPSocket;
			socket_context = socket;
		}
	}

	~MQTTNetwork() {

		if (is_security_enabled == SECURED_MQTT) {
			TLSSocket *socket;

			socket = (TLSSocket *) socket_context;
			delete socket;

		} else {
			TCPSocket *socket;

			socket = (TCPSocket *) socket_context;
			delete socket;

		}

	}

	int read(unsigned char* buffer, int len, int timeout) {
		int bytes_read = 0;
		int total_bytes = len;
		int ret = 0;
		if (is_security_enabled == SECURED_MQTT) {
			TLSSocket *socket;

			socket = (TLSSocket *) socket_context;
			do {
				ret = socket->recv(buffer + bytes_read,
						total_bytes - bytes_read);
				if (ret <= 0)
					return -1;
				bytes_read += ret;
			} while (bytes_read < total_bytes);
			return bytes_read;

		} else {
			TCPSocket *socket;

			socket = (TCPSocket *) socket_context;
			do {
				ret = socket->recv(buffer + bytes_read,
						total_bytes - bytes_read);
				if (ret < 0)
					return -1;
				bytes_read += ret;
			} while (bytes_read < total_bytes);
			return bytes_read;
		}

	}


	int write(unsigned char* buffer, int len, int timeout) {

		if (is_security_enabled == SECURED_MQTT) {
			TLSSocket *socket;

			socket = (TLSSocket *) socket_context;
			return socket->send(buffer, len);

		} else {
			TCPSocket *socket;

			socket = (TCPSocket *) socket_context;
			return socket->send(buffer, len);
		}

	}

	int set_root_ca_certificate(const char* root_ca_certifcate) {
		TLSSocket *socket = NULL;
		socket = (TLSSocket *) socket_context;

		if (root_ca_certifcate == NULL)
		{
			MQTT_NETWORK_INFO(("[MQTT INFO] : ROOT CA CERTIFICATE IS IGNORED\r\n"));
			return 0;
		}
		return socket->set_root_ca_cert(root_ca_certifcate);
	}

	int set_client_cert_key(const char* client_cert, const char* client_key) {
		TLSSocket *socket = NULL;
		socket = (TLSSocket *) socket_context;
		if (client_cert == NULL || client_key == NULL) {
			MQTT_NETWORK_ERROR(("[MQTT ERROR] : PASS VALID client certificate and client private key\r\n"));
			return -1;
		}
		return socket->set_client_cert_key(client_cert, client_key);
	}
	int connect(const char* hostname, int port, const char* peer_cn) {

		if (is_security_enabled == SECURED_MQTT) {
			TLSSocket *socket;
			int r;

			socket = (TLSSocket *) socket_context;

			r = socket->open(network);
			if (r != 0) {
				MQTT_NETWORK_ERROR(
						("[MQTT ERROR] : TLS SOCKET OPEN FAILED\r\n"));
			}

		    /* FIXME: By default we should pick hostname to verify the certificate, if user pass peer_cn as NULL*/
			MQTT_NETWORK_DEBUG(("[MQTT INFO] : hostname set : %s \n", peer_cn ));
			socket->set_hostname(peer_cn);

			address = SocketAddress(network, hostname, port );

			return socket->connect(address);

		} else {
			TCPSocket *socket;
			int r;
			socket = (TCPSocket *) socket_context;

			r = socket->open(network);
			if (r != 0) {
				MQTT_NETWORK_ERROR(
						("[MQTT ERROR] :  TCP SOCKET OPEN FAILED\r\n"));
			}

			return socket->connect(hostname, port);
		}

	}

	int disconnect() {

		if (is_security_enabled == SECURED_MQTT) {
			TLSSocket *socket;

			socket = (TLSSocket *) socket_context;
			return socket->close();

		} else {
			TCPSocket *socket;

			socket = (TCPSocket *) socket_context;
			return socket->close();

		}

	}

private:
	NetworkInterface* network;
	void* socket_context;
	mqtt_security_flag is_security_enabled;
	SocketAddress address;
};

#endif // _MQTTNETWORK_H_
