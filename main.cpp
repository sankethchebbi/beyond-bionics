#include <SoftwareSerial.h>
#include <Servo.h>
SoftwareSerial BT(10, 11);
Servo myservo;

#define LED 13
#define BAUDRATE 57600
#define DEBUGOUTPUT 0

byte generatedChecksum = 0;
byte checksum = 0;
int payloadLength = 0;
byte payloadData[64] = {0};
byte poorQuality = 0;
byte attention = 0;

long lastReceivedPacket = 0;
boolean bigPacket = false;
boolean servoActivated = false;

void setup() {
  pinMode(LED, OUTPUT);
  BT.begin(BAUDRATE);
  Serial.begin(BAUDRATE);
  myservo.detach();  // Detach the servo initially : newly added 
  delay(2000);  // Add a delay to allow the system to stabilize : newly added

}

byte ReadOneByte() {
  int ByteRead;
  while (!BT.available());
  ByteRead = BT.read();
#if DEBUGOUTPUT
  Serial.print((char)ByteRead);
#endif
  return ByteRead;
}

void loop() {
  if (ReadOneByte() == 170) {
    if (ReadOneByte() == 170) {
      payloadLength = ReadOneByte();
      if (payloadLength > 169)
        return;

      generatedChecksum = 0;
      for (int i = 0; i < payloadLength; i++) {
        payloadData[i] = ReadOneByte();
        generatedChecksum += payloadData[i];
      }

      checksum = ReadOneByte();
      generatedChecksum = 255 - generatedChecksum;

      if (checksum == generatedChecksum) {
        poorQuality = 200;
        attention = 0;
        for (int i = 0; i < payloadLength; i++) {
          switch (payloadData[i]) {
            case 2:
              i++;
              poorQuality = payloadData[i];
              bigPacket = true;
              break;
            case 0x04:
              i++;
              attention = payloadData[i];
              break;
            case 5:
              i++;
              break;
            case 0x16:
              i++;
              break;
            case 0x80:
              i = i + 3;
              break;
            case 0x83:
              i = i + 25;
              break;
            default:
              break;
          }
        }

#if !DEBUGOUTPUT
        if (bigPacket) {
          if (poorQuality == 0)
            digitalWrite(LED, HIGH);
          else
            digitalWrite(LED, LOW);
          Serial.print("Attention: ");
          Serial.print(attention);
          if (attention > 60 && !servoActivated) {
            myservo.attach(9);
            servoActivated = true;
            Serial.println("\nActivating servo motor!");
            myservo.write(30);
            delay(5000);
            myservo.write(120);
          } else {
            if (servoActivated) {
              myservo.detach();
            }
          }
          Serial.print("\n");
        }
#endif
        bigPacket = false;
      }
    }
  }
}