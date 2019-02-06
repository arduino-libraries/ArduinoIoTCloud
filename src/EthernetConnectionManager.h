#include "ConnectionManager.h"

#include <Ethernet.h>
#define BOARD_HAS_ETHERNET

class EthConnectionManager : public ConnectionManager {
public:
  EthConnectionManager(uint8_t *mac, int ss_pin);

  virtual unsigned long getTime();
  virtual void init();
  virtual void check();
  virtual Client &getClient() { return ethClient; };
  

private:
  
  void changeConnectionState(NetworkConnectionState _newState);

  const int CHECK_INTERVAL_IDLE = 100;
  const int CHECK_INTERVAL_INIT = 100;
  const int CHECK_INTERVAL_CONNECTING = 500;
  const int CHECK_INTERVAL_GETTIME = 100;
  const int CHECK_INTERVAL_CONNECTED = 10000;
  const int CHECK_INTERVAL_RETRYING = 5000;
  const int CHECK_INTERVAL_DISCONNECTED = 1000;
  const int CHECK_INTERVAL_ERROR = 500;

  unsigned long lastConnectionTickTime, lastNetworkStep;
  uint8_t* mac;
  int ss_pin;
  EthernetClient ethClient;
  int connectionTickTimeInterval;
};

#if !defined(BOARD_HAS_WIFI) && !defined(BOARD_HAS_GSM)
static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;
#endif

EthConnectionManager::EthConnectionManager(uint8_t *mac, int ss_pin = 10) :
  mac(mac),
  ss_pin(ss_pin),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

#include <EthernetUdp.h>

void sendNTPpacket(const char * address, uint8_t* packetBuffer, EthernetUDP* udp) {
  const int NTP_PACKET_SIZE = 48;
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp->beginPacket(address, 123);
  udp->write(packetBuffer, NTP_PACKET_SIZE);
  udp->endPacket();
}

unsigned long EthConnectionManager::getTime() {
  unsigned int localPort = 8888;
  const char timeServer[] = "time.nist.gov";
  const int NTP_PACKET_SIZE = 48;
  uint8_t packetBuffer[NTP_PACKET_SIZE];
  EthernetUDP Udp;

  Udp.begin(localPort);
  sendNTPpacket(timeServer, packetBuffer, &Udp);
  while (!Udp.parsePacket()) {}
  Udp.read(packetBuffer, NTP_PACKET_SIZE);

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  const unsigned long seventyYears = 2208988800UL;
  unsigned long epoch = secsSince1900 - seventyYears;

  return epoch;
}

void EthConnectionManager::init() {
}

void EthConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
  netConnectionState = _newState;
  int newInterval = CHECK_INTERVAL_IDLE;
  switch (_newState) {
    case CONNECTION_STATE_INIT:
      newInterval = CHECK_INTERVAL_INIT;
      break;
    case CONNECTION_STATE_CONNECTING:
      newInterval = CHECK_INTERVAL_CONNECTING;
      break;
    case CONNECTION_STATE_GETTIME:
      newInterval = CHECK_INTERVAL_GETTIME;
      break;
    case CONNECTION_STATE_CONNECTED:
      newInterval = CHECK_INTERVAL_CONNECTED;
      break;
    case CONNECTION_STATE_DISCONNECTED:
      newInterval = CHECK_INTERVAL_DISCONNECTED;
      
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
}

void EthConnectionManager::check() {
  char msgBuffer[120];
  unsigned long now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case CONNECTION_STATE_IDLE:
        changeConnectionState(CONNECTION_STATE_INIT);
        break;
      case CONNECTION_STATE_INIT:
        Ethernet.init(ss_pin);
        networkStatus = Ethernet.hardwareStatus();
        *msgBuffer = 0;
        sprintf(msgBuffer, "Eth hardware status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == EthernetNoHardware) {
          debugMessage("No Ethernet chip connected", 0);
          // don't continue:
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        networkStatus = Ethernet.linkStatus();
        *msgBuffer = 0;
        sprintf(msgBuffer, "Eth link status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == LinkOFF) {
          debugMessage("Failed to configure Ethernet via dhcp", 0);
          // don't continue:
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Ethernet shield recognized: ID", Ethernet.hardwareStatus());
        debugMessage(msgBuffer, 0);
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
      case CONNECTION_STATE_CONNECTING:
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connecting via dhcp");
        debugMessage(msgBuffer, 2);

        networkStatus = Ethernet.begin(mac, ss_pin);
        *msgBuffer = 0;
        sprintf(msgBuffer, "Ethernet.status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == 0) {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connection failed");
          debugMessage(msgBuffer, 0);

          *msgBuffer = 0;
          sprintf(msgBuffer, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          debugMessage(msgBuffer, 2);
          //changeConnectionState(CONNECTION_STATE_CONNECTING);
          return;
        } else {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connected!");
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_GETTIME);
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        debugMessage("Acquiring Time from Network", 3);
        unsigned long networkTime;
        networkTime = getTime();
        *msgBuffer = 0;
        sprintf(msgBuffer, "Network Time: %u", networkTime);
        debugMessage(msgBuffer, 3);
        if(networkTime > lastValidTimestamp){
          lastValidTimestamp = networkTime;
          changeConnectionState(CONNECTION_STATE_CONNECTED);
        }
        break;
      case CONNECTION_STATE_CONNECTED:
        // keep testing connection
        Ethernet.maintain();
        networkStatus = Ethernet.linkStatus();
        *msgBuffer = 0;
        sprintf(msgBuffer, "Eth link status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus != LinkON) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connected");
        debugMessage(msgBuffer, 2);
        break;
      case CONNECTION_STATE_DISCONNECTED:
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connection lost.");
        debugMessage(msgBuffer, 0);
        debugMessage("Attempting reconnection", 1);
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        //wifiClient.stop();
        break;
    }
    lastConnectionTickTime = now;
  }
}
