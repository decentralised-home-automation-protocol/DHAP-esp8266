#include "NetworkManager.h"
#include "FileManager.h"
#include "StatusManager.h"

class IoTDevice
{
private:
    NetworkManager networkManager;
    FileManager fileManager;
    StatusManager statusManager;

    char *ssid;
    char *password;
    char temp[255];

public:
    void setup(bool forceSetupAP, Status &deviceStatus)
    {
        statusManager.setStatusController(deviceStatus);

        fileManager.mountFileSystem();

        if (forceSetupAP)
        {
            networkManager.setupAccessPoint();
        }
        else
        {
            if (!networkManager.setupWiFi())
            {
                networkManager.setupAccessPoint();
            }
        }
    }

    bool commandRecieved(char *iotCommand)
    {
        String status = statusManager.sendStatusUpdateIfNeeded();
        if (status.length() > 0)
        {
            networkManager.broadcastStatus(status);
        }

        if (networkManager.newCommandRecieved())
        {
            return handleIncomingPacket(iotCommand);
        }
        return false;
    }

    bool handleIncomingPacket(char *iotCommand)
    {
        int packetType = networkManager.getRecentPacketType();
        Serial.printf("PACKET TYPE %d\n", packetType);
        String response;

        switch (packetType)
        {
        case 100: //Joining credentials
            if (!networkManager.hasJoinedNetwork)
            {
                networkManager.sendReplyPacket("110");
                attemptToJoinNetwork();
            }
            break;
        case 200: //UI Request
            Serial.println("UI Request Recieved.");
            response = fileManager.readFile();
            networkManager.sendReplyPacket("210:" + response);
            Serial.println("XML File sent.");
            break;
        case 300: //Discovery Request
            Serial.println("Discovery Request Recieved.");
            response = networkManager.getLocalIP();
            networkManager.sendReplyPacket(response);
            Serial.println("Discovery Packet Sent.");
            break;
        case 400: //IoT Command
            Serial.println("IoT Command Recieved.");

            networkManager.getRecentPacket(temp);

            strcpy(iotCommand, temp + 4);
            return true;
            break;
        case 500: //Status Lease Request
            Serial.println("Status Request Recieved. Adding to List...");
            statusManager.newStatusRegistration(networkManager.incomingPacket);
            break;
        case 520: //Leave Status Lease
            break;
        default:
            Serial.println("Unknown Packet");
        };

        return false;
    }

    void attemptToJoinNetwork()
    {
        tolkenizeCredentials(networkManager.incomingPacket);
        if (!networkManager.joinNetwork(ssid, password))
        {
            Serial.println("Failed to join network!");
            networkManager.sendReplyPacket("130");
        }
    }

    void tolkenizeCredentials(char *credentials)
    {
        char *type = strtok(credentials, ":,");
        ssid = strtok(NULL, ":,");
        password = strtok(NULL, ":,");
    }
};