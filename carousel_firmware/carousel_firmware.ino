// included libraries
#include <SPI.h>
#include <Ethernet2.h>

// the max length of accepted remote commands
#define MAX_CMD_LENGTH   40

// this firmware's version number
String VERSION = "0.2";

// ethernet shield mac address
byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x16, 0x12};

// network settings for the ethernet shield
IPAddress ip(192, 168, 0, 2);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(255, 255, 255, 0);
EthernetServer server = EthernetServer(50630);
EthernetClient client;

boolean connected = false;
String cmd;
int originSensor = 0;
int imgposSensor = 0;

// Arduino pins used in the device
int STOP = 9; // for stopping the step motor
int IMGPOS = 5; // for sensing a plate at imaging position
int ORIGIN = 6; // for sensing plate #1 at origin position
String TAB = "\t";

void setup() {  
  
  Serial.begin(9600);

  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  pinMode(STOP, OUTPUT);
  pinMode(ORIGIN, INPUT);
  pinMode(IMGPOS, INPUT);
    Serial.print ("done");
  digitalWrite(STOP, HIGH);


}

void loop() {
  client = server.available();
  if (client) {
    if (!connected) {
      client.flush();
      connected = true;
      server.println("OK Server started");
      cmd = "";
    }
    if (client.available() > 0) {
      readTelnetCommand(client.read());
    }
  }
  delay(10);
}

void readTelnetCommand(char c) {

  // reset command string if too long
  if (cmd.length() == MAX_CMD_LENGTH) {
    cmd = "";
  }

  // otherwise append recieved char
  cmd += c;

  // upon recieving a new line char, process command string
  if (c == '\n') {
    if (cmd.endsWith("\n")) cmd = cmd.substring (0, cmd.length() - 1);
    if (cmd.endsWith("\r")) cmd = cmd.substring (0, cmd.length() - 1);
    parseCommand();
  }
}

// this method processes incoming commands
void parseCommand() {

  // 'bye' stops the connection
  if (cmd.startsWith("bye")) {
    client.stop();
    connected = false;
  }

  // 'hello' returns a message with firmware's version number
  else if (cmd.startsWith("hello")) {
    server.println("OK" + TAB + "LUMALUM Carousel driver Version:" + VERSION);
  }

  // 'home' lets the carousel spin until plate#1 activates the origin sensor
  // then stops the carousel
  else if (cmd.startsWith("home")) {
    digitalWrite(STOP, LOW);
    originSensor = digitalRead(ORIGIN);
    while (originSensor == LOW) {
      originSensor = digitalRead(ORIGIN);
      delay(10);
    }
    digitalWrite(STOP, HIGH);
    server.println("OK");
  }

  // 'next' lets the carousel spin until next plate activates the sensor
  // at the imaging position, and then stops the carousel
  else if (cmd.startsWith("next")) {
    digitalWrite(STOP, LOW);
    delay(500);
    imgposSensor = digitalRead(IMGPOS);
    while (imgposSensor == LOW) {
      imgposSensor = digitalRead(IMGPOS);
      //delay(10);
    }
    digitalWrite(STOP, HIGH);
    server.println("OK");
  }

  // 'stop' stops the carousel regardless of it's position
  else if (cmd.startsWith("stop")) {
    digitalWrite(STOP, HIGH);
    server.println("OK");
  }

  // 'run' strats the carousel
  else if (cmd.startsWith("run")) {
    digitalWrite(STOP, LOW);
    server.println("OK");
  }

  // 'help' returns a string with available commands
  else if (cmd.startsWith("help")) {
    server.println("OK" + TAB + "hello, bye, stop, run, home, next, help");
  }

  // any other command returns this error message
  else {
    server.println("Invalid command");
  }

  // the cmd string is reset, so it's
  cmd = "";
}
