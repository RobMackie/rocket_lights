#include <Servo.h>

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WebServer.h>

// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

const char *ssid = "ImpactRocket"; // The name of the Wi-Fi network that will be created
const char *password = "pyrotech";   // The password required to connect to it, leave blank for an open network
IPAddress ip(10, 0, 0, 1);  // Set the desired IP address for the access point

#define SECTION_1_COLOR_R_ON 0
#define SECTION_1_COLOR_G_ON 255
#define SECTION_1_COLOR_B_ON 255

#define SECTION_2_COLOR_R_ON 255
#define SECTION_2_COLOR_G_ON 0
#define SECTION_2_COLOR_B_ON 255

#define SECTION_3_COLOR_R_ON 255
#define SECTION_3_COLOR_G_ON 255
#define SECTION_3_COLOR_B_ON 0

#define SECTION_4_COLOR_R_ON 255
#define SECTION_4_COLOR_G_ON 255
#define SECTION_4_COLOR_B_ON 255

// Here's where you set up the sections for which LEDs and what colors for each section
#define SECTION_1_FIRST 1
#define SECTION_1_END 10

#define SECTION_2_FIRST 11
#define SECTION_2_END 20

#define SECTION_3_FIRST 21
#define SECTION_3_END 30

#define SECTION_4_FIRST 31
#define SECTION_4_END 60


// Which pin on the Arduino/esp8266 is connected to the NeoPixels?
#define PIN       D6 // esp8266 D6 - On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS SECTION_4_END // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer server(80);  // Create an instance of the web server on port 80

const int BUILT_IN_LED2 = 2;
// prototypes for functions below
void handleRoot();
void handleLED();
void handleNotFound();
void switch_lights();
void switch_lights_off();
void switch_lights_section1();
void switch_lights_section2();
void switch_lights_section3();
void switch_lights_section4();
void switch_lights_on();
void setSpecificPixelRange(int first, int last, int red, int green, int blue);

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  pinMode(BUILT_IN_LED2, OUTPUT);
  digitalWrite(BUILT_IN_LED2, HIGH);

  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  WiFi.setSleep(false);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)


  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0)); // Set access point IP, gateway IP, and subnet mask
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer

  server.on("/", HTTP_GET, handleRoot);  // Handle the root path ("/") with the handleRoot function
  server.on("/LED", HTTP_GET, handleLED);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.on("/switch_lights", HTTP_GET, switch_lights);
  server.on("/switch_lights/off", HTTP_GET, switch_lights_off);
  server.on("/switch_lights/section1", HTTP_GET, switch_lights_section1);
  server.on("/switch_lights/section2", HTTP_GET, switch_lights_section2);
  server.on("/switch_lights/section3", HTTP_GET, switch_lights_section3);
  server.on("/switch_lights/section4", HTTP_GET, switch_lights_section4);
  server.on("/switch_lights/on", HTTP_GET, switch_lights_on);
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  
  pixels.clear();
  setSpecificPixelRange(SECTION_1_FIRST, SECTION_4_END, 0, 0, 0);
  
  server.begin();  // Start the server
  server.keepAlive(false);
}

void loop() { 
  server.handleClient();  // Handle incoming client requests
}

const char* g_selectionPageText = "<html> \
    <head> \
      <title> experiment </title> \
        <style> \
          div { \
           font-size: 0; \
          } \
          input { \
            font-size: 75px; \
            display: inline-block; \
            width: calc(80% - 20px); \
            padding: 40 px; \
            margin: 40px; \
          } \
        </style> \
      </head> \
    <body> \
      <div> \
        <br /> \
        <br /> \
        <br /> \
        <br /> \
        <center> \
            <input type=\"button\" onclick=\"location.href='http://10.0.0.1/switch_lights/off';\" value=\"Light OFF\" /> \
            <br /> \
            <br /> \
            <input type=\"button\" onclick=\"location.href='http://10.0.0.1/switch_lights/section1';\" value=\"Light up section 1\" /> \
            <br /> \
            <br /> \
            <input type=\"button\" onclick=\"location.href='http://10.0.0.1/switch_lights/section2';\" value=\"Light up section 2\" /> \
            <br /> \
            <br /> \
            <input type=\"button\" onclick=\"location.href='http://10.0.0.1/switch_lights/section3';\" value=\"Light up section 3\" /> \
            <br /> \
            <br /> \
            <input type=\"button\" onclick=\"location.href='http://10.0.0.1/switch_lights/section4';\" value=\"Light up section 4\" /> \
            <br /> \
            <br /> \
            <input type=\"button\" onclick=\"location.href='http://10.0.0.1/switch_lights/on';\" value=\"Light ON\" /> \
            <br /> \
            <br /> \
        </center> \
      </div> \
    </body> \
</html>";

const char* getSelectionPageText() {
  return g_selectionPageText;
}


void setSpecificPixelRange(int first, int last, int red, int green, int blue) {
  for (int jj = first; jj < last+1; jj++) {
    pixels.setPixelColor(jj, pixels.Color(red, green, blue));
  }
  pixels.show();
}

void handleRoot() {        
  const char* text = getSelectionPageText();                 // When URI / is requested, send a web page with a buttons to toggle the LED strip sections
  server.send(200, "text/html", text);
}

void handleLED() {                          // If a POST request is made to URI /LED
  digitalWrite(BUILT_IN_LED2,!digitalRead(BUILT_IN_LED2));      // Change the state of the LED
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void switch_lights_off() {
  // act here (activate section 1)
  pixels.clear();
  setSpecificPixelRange(SECTION_1_FIRST, SECTION_4_END, 0, 0, 0);
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}

void switch_lights_section1() {
  // act here (activate section 1)
  pixels.clear();
  setSpecificPixelRange(SECTION_1_FIRST, SECTION_1_END, SECTION_1_COLOR_R_ON, SECTION_1_COLOR_G_ON, SECTION_1_COLOR_B_ON);
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}
void switch_lights_section2(){
  // act here (activate section 2)
  pixels.clear();
  setSpecificPixelRange(SECTION_2_FIRST, SECTION_2_END, SECTION_2_COLOR_R_ON, SECTION_2_COLOR_G_ON, SECTION_2_COLOR_B_ON);
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}

void switch_lights_section3() {
  // act here (activate section 3)
  pixels.clear();
  setSpecificPixelRange(SECTION_3_FIRST, SECTION_3_END, SECTION_3_COLOR_R_ON, SECTION_3_COLOR_G_ON, SECTION_3_COLOR_B_ON);
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}

void switch_lights_section4(){
  // act here (activate section 4)
  pixels.clear();
  setSpecificPixelRange(SECTION_4_FIRST, SECTION_4_END, SECTION_4_COLOR_R_ON, SECTION_4_COLOR_G_ON, SECTION_4_COLOR_B_ON);
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}

void switch_lights_on(){
  // act here (activate section 4)
  pixels.clear();
  setSpecificPixelRange(SECTION_1_FIRST, SECTION_4_END, SECTION_4_COLOR_R_ON, SECTION_4_COLOR_G_ON, SECTION_4_COLOR_B_ON);
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}

void switch_lights(){
  // act here - do nothing
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}