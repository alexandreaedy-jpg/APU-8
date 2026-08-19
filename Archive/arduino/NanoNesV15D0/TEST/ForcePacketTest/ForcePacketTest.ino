/*
  ForcePacketTest - Test forçant l'envoi de paquets sans LATCH
  ----------------------------------------------------------
  Ce sketch envoie continuellement des paquets sur D2 et D3
  sans attendre le signal LATCH de la console.
*/

static const uint8_t NES_DATA_PIN = 2;
static const uint8_t NES_D3_PIN = 3;
static const uint8_t PACKET_SIZE = 12;

volatile uint8_t activePacket[PACKET_SIZE];
volatile uint8_t activePacketD3[PACKET_SIZE];

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  pinMode(NES_DATA_PIN, OUTPUT);
  pinMode(NES_D3_PIN, OUTPUT);
  
  digitalWrite(NES_DATA_PIN, LOW);
  digitalWrite(NES_D3_PIN, LOW);
  
  Serial.println("=== Force Packet Test ===");
  Serial.println("Envoi continu de paquets sur D2 et D3...");
  
  writeP2PacketFromState();
  writeD3PacketFromState();
}

void loop() {
  // Envoyer le paquet bit par bit
  for (uint8_t byteIndex = 0; byteIndex < PACKET_SIZE; ++byteIndex) {
    for (uint8_t bitMask = 1; bitMask != 0; bitMask <<= 1) {
      bool bitD2 = (activePacket[byteIndex] & bitMask) != 0;
      bool bitD3 = (activePacketD3[byteIndex] & bitMask) != 0;
      
      digitalWrite(NES_DATA_PIN, bitD2 ? HIGH : LOW);
      digitalWrite(NES_D3_PIN, bitD3 ? HIGH : LOW);
      
      delayMicroseconds(10);
    }
  }
  
  // Pause entre les paquets
  delay(10);
}

void writeP2PacketFromState() {
  activePacket[0] = 0xA8;
  activePacket[1] = 0x5A;
  activePacket[2] = 1;  // P1 trigger
  activePacket[3] = 0x80 | 60;  // P1 gate + note 60
  activePacket[4] = 1;  // P2 trigger
  activePacket[5] = 0x80 | 64;  // P2 gate + note 64
  activePacket[6] = 0x00;  // Attack P1=0, P2=0
  activePacket[7] = 0x00;  // Decay P1=0, P2=0
  activePacket[8] = 0x00;  // Release P1=0, P2=0
  activePacket[9] = 0x00;  // LFO P1
  activePacket[10] = 0x02;  // P2 LFO + P1 duty
  activePacket[11] = 0x02;  // P2 LFO + P2 duty
  
  Serial.println("Paquet P2 configuré:");
  Serial.print("  Magic: 0x");
  Serial.println(activePacket[0], HEX);
  Serial.print("  Magic: 0x");
  Serial.println(activePacket[1], HEX);
  Serial.print("  P1 trigger: ");
  Serial.println(activePacket[2]);
  Serial.print("  P1 gate+note: 0x");
  Serial.println(activePacket[3], HEX);
  Serial.print("  P2 trigger: ");
  Serial.println(activePacket[4]);
  Serial.print("  P2 gate+note: 0x");
  Serial.println(activePacket[5], HEX);
}

void writeD3PacketFromState() {
  activePacketD3[0] = 0xD3;
  activePacketD3[1] = 0x3D;
  activePacketD3[2] = 0x00;  // Attack P1=0, P2=0
  activePacketD3[3] = 0x00;  // Decay P1=0, P2=0
  activePacketD3[4] = 0x00;  // Release P1=0, P2=0
  for (uint8_t i = 5; i < PACKET_SIZE; ++i) {
    activePacketD3[i] = 0;
  }
  
  Serial.println("Paquet D3 configuré:");
  Serial.print("  Magic: 0x");
  Serial.println(activePacketD3[0], HEX);
  Serial.print("  Magic: 0x");
  Serial.println(activePacketD3[1], HEX);
}
