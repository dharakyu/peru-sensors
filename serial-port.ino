const size_t READ_BUF_SIZE = 256;

// Global variables
int counter = 0;
unsigned long lastSend = 0;

char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;

void setup() {
    Serial.begin(9600);

	// Serial1 RX is connected to Arduino TX (1)
	// Serial2 TX is connected to Arduino RX (0)
	// Photon GND is connected to Arduino GND
	Serial1.begin(9600);

	Serial.println("starting");
}

void loop() {
    // Serial.println("hi");
    // Read data from serial
	while(Serial1.available()) {
		if (readBufOffset < READ_BUF_SIZE) {
			char c = Serial1.read();
			if (c != '\n') {
				// Add character to buffer
				readBuf[readBufOffset++] = c;
			}
			else {
				// End of line character found, process line
				readBuf[readBufOffset] = 0;
				Particle.publish("test2", readBuf);
				delay(1000);
				Serial.println(readBuf);
				readBufOffset = 0;
			}
		}
		else {
			//Serial.println("readBuf overflow, emptying buffer");
			readBufOffset = 0;
		}
	}
}