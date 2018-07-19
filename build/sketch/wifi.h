#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"


// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   2  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  4
#define ADAFRUIT_CC3000_CS    8
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "DPE2018"           // cannot be longer than 32 characters!
#define WLAN_PASS       "cornelldpe2018"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           2    // What TCP port to listen on for connections. This is shown by the taped label.

bool GOT_NUMBER = false;

Adafruit_CC3000_Server chatServer(LISTEN_PORT);

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

void wifi_init(){
	Serial.println(F("Hello, CC3000!\n")); 
	if (!cc3000.begin())
		{
		Serial.println(F("Couldn't begin()! Check your wiring?"));
		while(1);
	  }
	if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
		Serial.println(F("Failed!"));
		while(1);
	  }
	while (!cc3000.checkDHCP())
	{
		delay(100); // ToDo: Insert a DHCP timeout!
	}
	while (! displayConnectionDetails()) {
		delay(1000);
	}
	chatServer.begin();
}

int get_number(){
	// Try to get a client which is connected.
	int a = 99;
	Adafruit_CC3000_ClientRef client = chatServer.available();
	GOT_NUMBER = false;
	if (client) {
     // Check if there is data available to read.
		if (client.available() > 0) {
			// Read a byte and write it to all clients.
			uint8_t ch = client.read();
			int value = ch - '0';
			//Serial.print(value);
			chatServer.write(ch);
			a = value;
			GOT_NUMBER = true;
		}
	}
	return a;
}



