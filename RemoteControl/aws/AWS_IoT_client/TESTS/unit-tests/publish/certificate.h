/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

/* Please change device certificate and device private key based on your account */
#define  SSL_CLIENTCERT_PEM   "-----BEGIN CERTIFICATE-----\n" \
        "MIIDWjCCAkKgAwIBAgIVAMK0tnwHtfJTOYHZMEjyS4rDiJ0oMA0GCSqGSIb3DQEB\n" \
        "CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
        "IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xNTA5MTUwNzIx\n" \
        "MjdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
        "dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCelAzfZ0cd1HF/aNAX\n" \
        "Do+k6qL33zYnWI9JA00cML4enKOIyexPrszldm8GaR28nkIifz4PfcC5IaOnse/k\n" \
        "v+KV6C0t7AULA2VubnusSQ5mM312MwFh6gMAeCoT7NHRp0zcc/sc4aXx5YUJ4Sxq\n" \
        "8vQnvCl+bgtViiKsvMR4HGzN6x8CsMzec0J8dfdp9KkE6KifHc1UIqiAFyTlvB1k\n" \
        "ZxgfR5AWSxK1hHbEd399JtlrIzIwY2uZbkH4e4NGYBqagSRtz6RBYQQ+ueYL7oAO\n" \
        "x9MBCLK9dtxfJhgNuRkD20w6ivOBzVcFKjLudVmiBd6VBjXnvloCW3G/Mrm6vNQ5\n" \
        "HPAVAgMBAAGjYDBeMB8GA1UdIwQYMBaAFEZxyFIPg3mtu69Uj6bbYGwoMTXZMB0G\n" \
        "A1UdDgQWBBRr/s+1qPbnYZtFQ/YDccyf1aSn5DAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
        "DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAkkx470byaJhFVuA17MSpvT5k\n" \
        "dZwb+YdgnIJQlX9C4wL1sDEIsE5lXvVYCoov96JazkKVnhFlpDDqzavpawQYGkjV\n" \
        "lieu2CbVtLKYARJWBAHyyXPBqMBMV9NcPOm7TT16pFOBjpaJQx0TTZ0yUbSXlh6X\n" \
        "pOkPoqtNoqcHse+w60LBrmy0fV7xkVktuPK0emfa3gTyKVx+qmwqPTghidFkCiiy\n" \
        "vjtv6DnWMmRHE2sR/6CuWS9Qnb/xzzfYzjgY+YnaESVs82OejaugOzm/C3ITaviD\n" \
        "R7HEGuy8j840r5LLTlJPt84vNm2FKVnms7AX7vK8tSiiUoNSFgx7kQpROavcaQ==\n" \
        "-----END CERTIFICATE-----\n" \
		"\0"\
		"\0"



#define  SSL_CLIENTKEY_PEM  "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEowIBAAKCAQEAnpQM32dHHdRxf2jQFw6PpOqi9982J1iPSQNNHDC+HpyjiMns\n" \
        "T67M5XZvBmkdvJ5CIn8+D33AuSGjp7Hv5L/ilegtLewFCwNlbm57rEkOZjN9djMB\n" \
        "YeoDAHgqE+zR0adM3HP7HOGl8eWFCeEsavL0J7wpfm4LVYoirLzEeBxszesfArDM\n" \
        "3nNCfHX3afSpBOionx3NVCKogBck5bwdZGcYH0eQFksStYR2xHd/fSbZayMyMGNr\n" \
        "mW5B+HuDRmAamoEkbc+kQWEEPrnmC+6ADsfTAQiyvXbcXyYYDbkZA9tMOorzgc1X\n" \
        "BSoy7nVZogXelQY1575aAltxvzK5urzUORzwFQIDAQABAoIBAH0eeU2nkkAiB+8s\n" \
        "Rst6bLIFg9IpZvQCdwN3vFaaytciZhGeqHpyhC4klRMsyw6wm6PWW6QjZ3Vq6lJ9\n" \
        "HbeSAvieeU87Yvs+kcBhIelctyTCSaKCOfwEhJvRM/oGp8JQy19Bj4dJLwK+Qor6\n" \
        "BG7aBPR5DRA5SW4TkKpUQnw2iU6qjZhWO8NWzhSYflBe+HO6KhEnWAx1sX7wP2lp\n" \
        "tVRDqxyLcSxGEZSmjgGtGpxhokJG1B6M8c4HxUkOVwRx/oqNe34MTgyQjehCMRvl\n" \
        "MQ84/3GbG74pqDg2Q7W6Cjv/IBQbvTWJw9zZbKS8CC0j6I86i74aqW0LbhcsMPPT\n" \
        "IqNRPR0CgYEA5Gc2Es0lQGGUlyIxE7gXxQXZBuUPlFqp1c2GM1edOADzdfiUP/Pw\n" \
        "7+EIe6Crg9ZYxI5blMHTE1/Xi6ZvW2UG9RHsNa/Vx4IG/preswfuENGqEfqL+c8w\n" \
        "VMxgQUbA7tDI7M4rHqcEEtHYoPvfXPacaug1uHh9d2xHMcEGjroqLfMCgYEAsb0S\n" \
        "QUK+0I47Eycbc1gZx8slT9lI8YRtxw2/vszQiEulLrAfWEuE7cJzdFsAtIhfrXJW\n" \
        "5D05HEOtUaaedH+20muUFI3WdQS9/K9dpvQEfs6N6obWedqcRxg6IhqtCpXa/cx/\n" \
        "Iw+/Ngm6ACrL2qIYmvAOOamlmkQOcEGTvg9fg9cCgYBi07nU9sjoi5BkuJmto5k9\n" \
        "WeNnFVPhaD1WUoB/1KglZFvzrqRO67lTwfYOsB2mhOeR3NsJibhQCMdWGKTUUelp\n" \
        "vuCK9lM06TEnl1FG++Zphp7k+pj4dUq/VlNLy1aNvu9Bb4++ypt7nSrztSgXrGlP\n" \
        "5Lvh3tjDBKnlgFTbCvhXOwKBgGn0KBm3TEDPSPAV9AtCjbvIoimDgjRXmYl77L41\n" \
        "ImrdjU7TtxvPiSKjNGg8h6MXWNHww1O0G4N55Kw0A2m3aKjvcIsAMsP3W7pyYAXS\n" \
        "mYtuXrNcRibF5zuKeb0y4czZoH6DlZa1IGt6SOMon/Vpg/l4+UHum8XmpCTPMt+u\n" \
        "EmZpAoGBAOMNN2sDrFPWddvKvW+Yg05t4+MUw1clui+WKqmaHaymPgA/ZTFYZ8ig\n" \
        "+p3lMfIMne4GddKWF8KZAB5n8kstSnPZCjhyE8NFd4adsbnWkLCebA27084co3Pl\n" \
        "2urEffkOp9OwmfV/wyHW6Sim3EX1IQJMoeqPTGWwOmKIkyNFWAO1\n" \
        "-----END RSA PRIVATE KEY-----\n" \
		"\0"\
		"\0"


#define SSL_CA_PEM  "-----BEGIN CERTIFICATE-----\n" \
        "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
        "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
        "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
        "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
        "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
        "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
        "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
        "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
        "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
        "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
        "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
        "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
        "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
        "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
        "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
        "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
        "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
        "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
        "-----END CERTIFICATE-----\n" \
		"\0"\
		"\0"



/* INVALID CERTIFICATES */
/* Please change device certificate and device private key based on your account */
#define  SSL_CLIENTCERT_PEM_INVALID   "-----BEGIN CERTIFICATE-----\n" \
        "MIIDWjCCAkKgAwIBAgIVAMK0tnwHtfJTOYHZMEjyS4rDiJ0oMA0GCSqGSIb3DQEB\n" \
        "CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
        "IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xNTA5MTUwNzIx\n" \
        "MjdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
        "pOkPoqtNoqcHse+w60LBrmy0fV7xkVktuPK0emfa3gTyKVx+qmwqPTghidFkCiiy\n" \
        "vjtv6DnWMmRHE2sR/6CuWS9Qnb/xzzfYzjgY+YnaESVs82OejaugOzm/C3ITaviD\n" \
        "R7HEGuy8j840r5LLTlJPt84vNm2FKVnms7AX7vK8tSiiUoNSFgx7kQpROavcaQ==\n" \
        "-----END CERTIFICATE-----\n" \
		"\0"\
		"\0"



#define  SSL_CLIENTKEY_PEM_INVALID  "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEowIBAAKCAQEAnpQM32dHHdRxf2jQFw6PpOqi9982J1iPSQNNHDC+HpyjiMns\n" \
        "T67M5XZvBmkdvJ5CIn8+D33AuSGjp7Hv5L/ilegtLewFCwNlbm57rEkOZjN9djMB\n" \
        "YeoDAHgqE+zR0adM3HP7HOGl8eWFCeEsavL0J7wpfm4LVYoirLzEeBxszesfArDM\n" \
        "3nNCfHX3afSpBOionx3NVCKogBck5bwdZGcYH0eQFksStYR2xHd/fSbZayMyMGNr\n" \
        "mW5B+HuDRmAamoEkbc+kQWEEPrnmC+6ADsfTAQiyvXbcXyYYDbkZA9tMOorzgc1X\n" \
        "BSoy7nVZogXelQY1575aAltxvzK5urzUORzwFQIDAQABAoIBAH0eeU2nkkAiB+8s\n" \
        "Iw+/Ngm6ACrL2qIYmvAOOamlmkQOcEGTvg9fg9cCgYBi07nU9sjoi5BkuJmto5k9\n" \
        "WeNnFVPhaD1WUoB/1KglZFvzrqRO67lTwfYOsB2mhOeR3NsJibhQCMdWGKTUUelp\n" \
        "vuCK9lM06TEnl1FG++Zphp7k+pj4dUq/VlNLy1aNvu9Bb4++ypt7nSrztSgXrGlP\n" \
        "5Lvh3tjDBKnlgFTbCvhXOwKBgGn0KBm3TEDPSPAV9AtCjbvIoimDgjRXmYl77L41\n" \
        "ImrdjU7TtxvPiSKjNGg8h6MXWNHww1O0G4N55Kw0A2m3aKjvcIsAMsP3W7pyYAXS\n" \
        "mYtuXrNcRibF5zuKeb0y4czZoH6DlZa1IGt6SOMon/Vpg/l4+UHum8XmpCTPMt+u\n" \
        "EmZpAoGBAOMNN2sDrFPWddvKvW+Yg05t4+MUw1clui+WKqmaHaymPgA/ZTFYZ8ig\n" \
        "+p3lMfIMne4GddKWF8KZAB5n8kstSnPZCjhyE8NFd4adsbnWkLCebA27084co3Pl\n" \
        "2urEffkOp9OwmfV/wyHW6Sim3EX1IQJMoeqPTGWwOmKIkyNFWAO1\n" \
        "-----END RSA PRIVATE KEY-----\n" \
		"\0"\
		"\0"


#define SSL_CA_PEM_INVALID  "-----BEGIN CERTIFICATE-----\n" \
        "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
        "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
        "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
        "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
        "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
        "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
        "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
        "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
        "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
        "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
        "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
        "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
        "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
        "-----END CERTIFICATE-----\n" \
		"\0"\
		"\0"
