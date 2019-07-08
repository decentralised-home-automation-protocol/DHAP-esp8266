#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "FileManager.h"

IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);

class NetworkManager
{
private:
    WiFiUDP Udp;
    FileManager fileManager;

    unsigned int localUdpPort = 4210; // local port to listen on
    char *DEFAULT_SSID = "ESPsoftAP_01";
    char *DEFAULT_PASSWORD = "passforap";

public:
    char incomingPacket[255]; // buffer for incoming packets
    bool hasJoinedNetwork = false;

    bool commandRecieved()
    {
        int packetSize = Udp.parsePacket();
        if (packetSize)
        {
            Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
            int len = Udp.read(incomingPacket, 255);
            if (len > 0)
            {
                incomingPacket[len] = 0;
            }
            Serial.printf("UDP packet contents: %s\n", incomingPacket);
            return true;
        }
        return false;
    }

    void getRecentPacket(char *recentPacket)
    {
        strcpy(recentPacket, incomingPacket);
    }

    void setupAccessPoint()
    {
        Serial.print("Setting soft-AP configuration ... ");
        Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

        Serial.print("Setting soft-AP ... ");
        Serial.println(WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD) ? "Ready" : "Failed!");

        Serial.print("Soft-AP IP address = ");
        Serial.println(WiFi.softAPIP());

        Udp.begin(localUdpPort);
        Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.softAPIP().toString().c_str(), localUdpPort);
    }

    void sendXMLfile()
    {
        String response = fileManager.readFile();
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response.c_str());
        Udp.endPacket();
    }

    void joinNetwork(char *SSID, char *password)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(SSID, password);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }

        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        Udp.begin(localUdpPort);

        hasJoinedNetwork = true;
    }
};