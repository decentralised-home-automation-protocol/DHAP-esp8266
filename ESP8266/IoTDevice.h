#include "NetworkManager.h"

class IoTDevice
{
private:
    NetworkManager networkManager;
    FileManager fileManager;

    char *ssid;
    char *password;

public:
    void setup(bool setupAP)
    {
        fileManager.mountFileSystem();

        if (setupAP)
        {
            networkManager.setupAccessPoint();
        }
        else
        {
            String credentials = fileManager.getNetworkCredentials();

            if (credentials.length() == 0)
            {
                Serial.println("No credentials found!");
                networkManager.setupAccessPoint();
            }
            else
            {
                char *creds = new char[credentials.length()];
                strcpy(creds, credentials.c_str());
                tolkenizeCredentials(creds);
                if (!networkManager.joinNetwork(ssid, password))
                {
                    Serial.println("Failed to join network!");
                    networkManager.setupAccessPoint();
                }
            }
        }
    }

    bool commandRecieved(char *iotCommand)
    {
        bool newIncomingCommandReceived = networkManager.commandRecieved();
        if (newIncomingCommandReceived)
        {
            return handleIncomingPacket(iotCommand);
        }
        return false;
    }

    bool handleIncomingPacket(char *iotCommand)
    {
        if (networkManager.hasJoinedNetwork)
        {
            if (isUIRequest())
            {
                networkManager.sendXMLfile();
            }
            else if (isDiscoveryRequest())
            {
                networkManager.sendDiscoveryPacket();
            }
            else
            {
                networkManager.getRecentPacket(iotCommand);
                return true;
            }
        }
        else
        {
            attemptToJoinNetwork();
        }
        return false;
    }

    void attemptToJoinNetwork()
    {
        tolkenizeCredentials(networkManager.incomingPacket);
        if (networkManager.joinNetwork(ssid, password))
        {
            fileManager.saveNetworkCredentials(ssid, password);
        }
        else
        {
            Serial.println("Failed to join network!");
        }
    }

    bool isUIRequest()
    {
        return networkManager.incomingPacket[0] == 'U' && networkManager.incomingPacket[1] == 'I';
    }

    bool isDiscoveryRequest()
    {
        return networkManager.incomingPacket[0] == 'D' && networkManager.incomingPacket[1] == 'I';
    }

    void tolkenizeCredentials(char *credentials)
    {
        ssid = strtok(credentials, ":");
        password = strtok(NULL, ":");
    }
};
