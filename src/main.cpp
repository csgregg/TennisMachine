#include <EmbAJAX.h>
#include <Ticker.h>

#define RELAY_PIN D7
#define OPTO_PIN D8
#define PUSH_BUTTON D3

void LaunchBall() {

  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);                             // To get it past the opto slot

  while(!digitalRead(OPTO_PIN)) {         // Keep motor on until next tab hits the slot
    yield();
  };

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Fire");
}

#define BUFLEN 30

// Set up web server, and register it with EmbAJAX. Note: EmbAJAXOutputDirverWebServerClass is a
// converience #define to allow using the same example code across platforms
EmbAJAXOutputDriverWebServerClass server(80);
EmbAJAXOutputDriver driver(&server);

// Define the main elements of interest as variables, so we can access to them later in our sketch.
const char* modes[] = {"Off", "Auto"};
EmbAJAXRadioGroup<2> mode("mode", modes);
EmbAJAXSlider launchfreq("launchfreq", 10, 100, 20);   // in 100ms, from 10 to 100, initial value 20
EmbAJAXMutableSpan launchfreq_d("launchfreq_d");
char launchfreq_d_buf[BUFLEN];

void fire_buttonPressed(EmbAJAXPushButton*){ LaunchBall(); }
EmbAJAXPushButton fire_button("Launch_Button", "Launch", fire_buttonPressed);

// Define a page (named "page") with our elements of interest, above, interspersed by some uninteresting
// static HTML. Note: MAKE_EmbAJAXPage is just a convenience macro around the EmbAJAXPage<>-class.
MAKE_EmbAJAXPage(page, "Tennis Ball Launcher", "",
  new EmbAJAXStatic("<h3>Launch Settings</h1><p>Set the mode: "),
  &mode,
  new EmbAJAXStatic("</p><p>Launch frequency: <i>1s </i>"),
  &launchfreq,
  new EmbAJAXStatic("<i> 10s</i>"),
  &launchfreq_d,
  new EmbAJAXStatic("</p>"),
  &fire_button
)

void updateUI() {
  // Enabled / disable the slider. Note that you could simply do this inside the loop. However,
  // placing it here makes the client UI more responsive (try it).
  launchfreq.setEnabled(mode.selectedOption() == 1);
  Serial.println(launchfreq.intValue());

  sprintf(launchfreq_d_buf, " %2.1f sec", float(launchfreq.intValue())/10 );
  launchfreq_d.setValue(launchfreq_d_buf);
}


Ticker tenthSecPip;
int tenthSec = 0;  

void tenthSecPipCall(){
  tenthSec++;
}


void setup() {

  Serial.begin(115200);
  Serial.println("Now starting...");

  // Example WIFI setup as an access point. Change this to whatever suits you, best.
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig (IPAddress (192,168,4,1), IPAddress (0,0,0,0), IPAddress (255,255,255,0));
  WiFi.softAP("Launcher", "");

  // Tell the server to serve our EmbAJAX test page on root
  // installPage() abstracts over the (trivial but not uniform) WebServer-specific instructions to do so
  driver.installPage(&page, "/", updateUI);
  server.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(OPTO_PIN, INPUT);
  pinMode(PUSH_BUTTON, INPUT);

  digitalWrite(LED_BUILTIN,HIGH);
  digitalWrite(RELAY_PIN,LOW);

  tenthSecPip.attach_ms(100,tenthSecPipCall);

  launchfreq.setEnabled(false);

  updateUI(); // init displays

}



void loop() {
  // handle network. loopHook() simply calls server.handleClient(), in most but not all server implementations.
  driver.loopHook();
  
  if (tenthSec > launchfreq.intValue()) {
    tenthSec = 0;

    if(mode.selectedOption() == 1){ LaunchBall(); }
  }
}
