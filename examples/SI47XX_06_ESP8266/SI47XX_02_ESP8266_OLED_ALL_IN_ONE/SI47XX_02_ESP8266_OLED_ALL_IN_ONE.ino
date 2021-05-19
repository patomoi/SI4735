/*

  UNDER CONSTRUCTION......
 
  This sketch runs on ESP8266 device.   

  If you are using a SI4735-D60 or SI4732-A10, you can also use this sketch to add the SSB functionalities to the
  original Pavleski's project. If you are using another SI4730-D60, the SSB wil not work. But you will still have
  the SW functionalities.

  It is important to say that this sketch was designed to work with the circuit implemented by Mirko Pavleski (see link above).
  The visual interface, control commands, band plan, and some functionalities are different if compared with the original
  sketch. Be sure you are using the SI4735 Arduino Library written by PU2CLR to run this sketch. The library used by the original
  sketch will not work here. Also, you have to install the LiquidCrystal library.

  It is  a  complete  radio  capable  to  tune  LW,  MW,  SW  on  AM  and  SSB  mode  and  also  receive  the
  regular  comercial  stations.

  Features:   AM; SSB; LW/MW/SW; external mute circuit control; AGC; Attenuation gain control;
              SSB filter; CW; AM filter; 1, 5, 10, 50 and 500kHz step on AM and 10Hhz sep on SSB

  Wire up on Arduino UNO, Pro mini and SI4735-D60
  | Device name               | Device Pin / Description      |  ESP8266      |
  | ----------------          | ----------------------------- | ------------  |
  |    OLED                   |                               |               |
  |                           | SDA/SDIO                      |  GPIO4        | 
  |                           | SCL/SCLK                      |  GPIO5        | 
  |    SS4735 (*1)            |                               |               |
  |                           | RESET (pin 15)                |  GPIO2        |
  |                           | SDIO (pin 18)                 |  GPIO4        |
  |                           | SCLK (pin 17)                 |  GPIO5        |
  |                           | (*1)SEN (pin 16)              |  +Vcc or GND  |
  |    Encoder                |                               |               |
  |                           | A                             |       2       |
  |                           | B                             |       3       |
  |                           | PUSH BUTTON (encoder)         |     A0/14     |

  (*1) If you are using the SI4732-A10, check the corresponding pin numbers.
  (*1) The PU2CLR SI4735 Arduino Library has resources to detect the I2C bus address automatically.
       It seems the original project connect the SEN pin to the +Vcc. By using this sketch, you do
       not need to worry about this setting.
  ATTENTION: Read the file user_manual.txt
  Prototype documentation: https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

  By PU2CLR, Ricardo, Oct  2020.
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <SI4735.h>
#include "DSEG7_Classic_Regular_16.h"
#include "Rotary.h"

#include "patch_init.h" // SSB patch for whole SSBRX initialization string

const uint16_t size_content = sizeof ssb_patch_content; // see patch_init.h

#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

#define RESET_PIN 2                // GPIO2

// Enconder PINs
#define ENCODER_PIN_A 14           // GPIO14 
#define ENCODER_PIN_B 13           // GPIO13 


// Buttons controllers
#define ENCODER_PUSH_BUTTON 12     // GPIO12

#define MIN_ELAPSED_TIME 300
#define MIN_ELAPSED_RSSI_TIME 150
#define ELAPSED_COMMAND 2000  // time to turn off the last command controlled by encoder. Time to goes back to the FVO control
#define ELAPSED_CLICK 1500    // time to check the double click commands
#define DEFAULT_VOLUME 35    // change it for your favorite sound volume

#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define LW 4

#define SSB 1

bool bfoOn = false;
bool ssbLoaded = false;

int8_t agcIdx = 0;
uint8_t disableAgc = 0;
int8_t agcNdx = 0;
int8_t softMuteMaxAttIdx = 4;
uint8_t countClick = 0;

uint8_t seekDirection = 1;

bool cmdBand = false;
bool cmdVolume = false;
bool cmdAgc = false;
bool cmdBandwidth = false;
bool cmdStep = false;
bool cmdMode = false;
bool cmdMenu = false;
bool cmdSoftMuteMaxAtt = false;

int currentBFO = 0;
long elapsedRSSI = millis();
long elapsedButton = millis();
long elapsedCommand = millis();
long elapsedClick = millis();
volatile int encoderCount = 0;
uint16_t currentFrequency;

const uint8_t currentBFOStep = 10;

const char * menu[] = {"Seek", "Step", "Mode", "BW", "AGC/Att", "Volume", "SoftMute", "BFO"};
int8_t menuIdx = 0;
const int lastMenu = 7;
int8_t currentMenuCmd = -1;

typedef struct
{
  uint8_t idx;      // SI473X device bandwitdth index
  const char *desc; // bandwitdth description
} Bandwitdth;

int8_t bwIdxSSB = 4;
Bandwitdth bandwitdthSSB[] = {{4, "0.5"},
  {5, "1.0"},
  {0, "1.2"},
  {1, "2.2"},
  {2, "3.0"},
  {3, "4.0"}
};

int8_t bwIdxAM = 4;
Bandwitdth bandwitdthAM[] = {{4, "1.0"},
  {5, "1.8"},
  {3, "2.0"},
  {6, "2.5"},
  {2, "3.0"},
  {1, "4.0"},
  {0, "6.0"}
};

const char *bandModeDesc[] = {"FM ", "LSB", "USB", "AM "};
uint8_t currentMode = FM;

/**
 *  Band data structure
 */
typedef struct
{
  const char *bandName; // Band description
  uint8_t bandType;     // Band type (FM, MW or SW)
  uint16_t minimumFreq; // Minimum frequency of the band
  uint16_t maximumFreq; // maximum frequency of the band
  uint16_t currentFreq; // Default frequency or current frequency
  uint16_t currentStep; // Defeult step (increment and decrement)
} Band;

/**
 *  Band table. You can customize this table for your own band plan
 */
Band band[] = {
  {"VHF", FM_BAND_TYPE, 6400, 10800,  10390, 10},
  {"MW1", MW_BAND_TYPE,   150,  1720,   810, 10},
  {"MW2", MW_BAND_TYPE,  1700,  3500,  2500, 5},
  {"80M", MW_BAND_TYPE,  3500,  4000,  3700, 1},
  {"SW1", SW_BAND_TYPE,  4000,  5500,  4885, 5},
  {"SW2", SW_BAND_TYPE,  5500,  6500,  6000, 5},
  {"40M", SW_BAND_TYPE,  6500,  7300,  7100, 1},
  {"SW3", SW_BAND_TYPE,  7200,  8000,  7200, 5},
  {"SW4", SW_BAND_TYPE,  9000, 11000,  9500, 5},
  {"SW5", SW_BAND_TYPE, 11100, 13000, 11900, 5},
  {"SW6", SW_BAND_TYPE, 13000, 14000, 13500, 5},
  {"20M", SW_BAND_TYPE, 14000, 15000, 14200, 1},  
  {"SW7", SW_BAND_TYPE, 15000, 17000, 15300, 5},
  {"SW8", SW_BAND_TYPE, 17000, 18000, 17500, 5},
  {"15M", SW_BAND_TYPE, 20000, 21400, 21100, 1},
  {"SW9", SW_BAND_TYPE, 21400, 22800, 21500, 5},
  {"CB ", SW_BAND_TYPE, 26000, 28000, 27500, 1},
  {"10M", SW_BAND_TYPE, 28000, 30000, 28400, 1},
  {"ALL", SW_BAND_TYPE, 150, 30000, 15000, 1}    // All band. LW, MW and SW (from 150kHz to 30MHz)
}; // Super band: 150kHz to 30MHz

const int lastBand = (sizeof band / sizeof(Band)) - 1;
int bandIdx = 0;
int tabStep[] = {1, 5, 10, 50, 100, 500, 1000};
const int lastStep = (sizeof tabStep / sizeof(int)) - 1;
int idxStep = 0;
uint16_t currentStep = 1;

uint8_t rssi = 0;
uint8_t volume = DEFAULT_VOLUME;

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

SI4735 rx;


void setup()
{
  // Encoder pins
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);
  
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);


   display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

 
  // Splash - Change it for your introduction text.
  display.display();
  display.setTextColor(SSD1306_WHITE);
  delay(1000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("PU2CLR");
  display.setCursor(0, 15);
  display.print("ESP8266");
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("SI473X");
  display.setCursor(0, 15);
  display.print("Arduino");  
  display.display();

  delay(5000);
  display.clearDisplay();

  // ICACHE_RAM_ATTR void rotaryEncoder(); see rotaryEncoder implementation below.
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setI2CFastModeCustom(100000);
  
  rx.getDeviceI2CAddress(RESET_PIN); // Looks for the I2C bus address and set it.  Returns 0 if error
  rx.setup(RESET_PIN, MW_BAND_TYPE); 


  
  // Set up the radio for the current band (see index table variable bandIdx )
  delay(300); 
  useBand();
  rx.setVolume(volume);
  showStatus();
}



/**
    Set all command flags to false
    When all flags are disabled (false), the encoder controls the frequency
*/
void disableCommands()
{
  cmdBand = false;
  bfoOn = false;
  cmdVolume = false;
  cmdAgc = false;
  cmdBandwidth = false;
  cmdStep = false;
  cmdMode = false;
  cmdMenu = false;
  cmdSoftMuteMaxAtt = false;
  countClick = 0;
  showCommandStatus((char *) "VFO ");
}

/**
 * Reads encoder via interrupt
 * Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
 * if you do not add ICACHE_RAM_ATTR declaration, the system will reboot during attachInterrupt call. 
 * With ICACHE_RAM_ATTR macro you put the function on the RAM.
 */
ICACHE_RAM_ATTR void  rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
}

/**
 * Shows frequency information on Display
 */
void showFrequency()
{
  char tmp[15];
  char bufferDisplay[15];
  char * unit;
  sprintf(tmp, "%5.5u", currentFrequency);
  bufferDisplay[0] = (tmp[0] == '0') ? ' ' : tmp[0];
  bufferDisplay[1] = tmp[1];
  if (rx.isCurrentTuneFM())
  {
    bufferDisplay[2] = tmp[2];
    bufferDisplay[3] = '.';
    bufferDisplay[4] = tmp[3];
    unit = (char *) "MHz";
  }
  else
  {
    if ( currentFrequency  < 1000 ) {
      bufferDisplay[1] = ' ';
      bufferDisplay[2] = tmp[2] ;
      bufferDisplay[3] = tmp[3];
      bufferDisplay[4] = tmp[4];
    } else {
      bufferDisplay[2] = tmp[2];
      bufferDisplay[3] = tmp[3];
      bufferDisplay[4] = tmp[4];
    }
    unit = (char *) "kHz";
  }
  bufferDisplay[5] = '\0';
  // strcat(bufferDisplay, unit);
  // display.setTextSize(2);
  display.setFont(&DSEG7_Classic_Regular_16);
  display.clearDisplay();
  display.setCursor(20, 24);
  display.print(bufferDisplay);
  display.setCursor(90,15);
  display.setFont(NULL);
  display.setTextSize(1);
  display.print(unit);
  display.display();

  showMode();
}

/**
 * Shows the current mode
 */
void showMode() {
  char * bandMode;
  if (currentFrequency < 520)
    bandMode = (char *) "LW  ";
  else
    bandMode = (char *) bandModeDesc[currentMode];
  display.setTextSize(1);
  // display.clearDisplay();
  display.setCursor(0, 0);
  display.print(bandMode);
  display.setCursor(90,0);
  display.print(band[bandIdx].bandName);
  display.display();
}

/**
 * Shows some basic information on display
 */
void showStatus()
{
  // lcd.clear();
  showFrequency();
  showRSSI();
}

/**
 *  Shows the current Bandwitdth status
 */
void showBandwitdth()
{
  char bufferDisplay[15];
  if (currentMode == LSB || currentMode == USB || currentMode == AM)
  {
    char *bw;
    if (currentMode == AM)
      bw = (char *)bandwitdthAM[bwIdxAM].desc;
    else
      bw = (char *)bandwitdthSSB[bwIdxSSB].desc;
    sprintf(bufferDisplay, "BW: %s kHz", bw);
  }
  else
    bufferDisplay[0] = '\0';

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(bufferDisplay);
  display.display();
}

/**
 *   Shows the current RSSI and SNR status
 */
void showRSSI()
{
  char sMeter[10];
  sprintf(sMeter, "S:%d ", rssi);

  display.fillRect(0, 25, 128, 10, SSD1306_BLACK);  
  display.setTextSize(1);
  display.setCursor(80, 25);
  display.print(sMeter);
  if (currentMode == FM)
  {
    display.setCursor(0, 25);
    display.print((rx.getCurrentPilot()) ? "ST" : "MO");
  }

  display.display();
}

/**
 *    Shows the current AGC and Attenuation status
 */
void showAgcAtt()
{
  char sAgc[15];
  // lcd.clear();
  rx.getAutomaticGainControl();
  if (agcNdx == 0 && agcIdx == 0)
    strcpy(sAgc, "AGC ON");
  else
    sprintf(sAgc, "ATT: %2.2d", agcNdx);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(sAgc);
  display.display();
}

/**
 *   Shows the current step
 */
void showStep()
{
  char stAux[15];
  sprintf(stAux, "STEP: %4u", currentStep);
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(stAux);
  display.display();
}

/**
 *  Shows the current BFO value
 */
void showBFO()
{
  char bfo[15];
  if (currentBFO > 0)
    sprintf(bfo, "BFO: +%4.4d", currentBFO);
  else
    sprintf(bfo, "BFO: %4.4d", currentBFO);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(bfo);
  display.display();  
  elapsedCommand = millis();
}

/*
 *  Shows the volume level on LCD
 */
void showVolume()
{
  char volAux[12];
  sprintf(volAux, "VOLUME: %2u", rx.getVolume());
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(volAux);
  display.display();  
}

/**
 * Show Soft Mute 
 */
void showSoftMute()
{
  char sMute[18];
  sprintf(sMute, "Soft Mute: %2d", softMuteMaxAttIdx);
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(sMute);
  display.display(); 
}

/**
 *   Sets Band up (1) or down (!1)
 */
void setBand(int8_t up_down)
{
  band[bandIdx].currentFreq = currentFrequency;
  band[bandIdx].currentStep = currentStep;
  if (up_down == 1)
    bandIdx = (bandIdx < lastBand) ? (bandIdx + 1) : 0;
  else
    bandIdx = (bandIdx > 0) ? (bandIdx - 1) : lastBand;
  useBand();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Switch the radio to current band
 */
void useBand()
{
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    currentMode = FM;
    rx.setTuneFrequencyAntennaCapacitor(0);
    rx.setFM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep);
    rx.setSeekFmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
    bfoOn = ssbLoaded = false;
  }
  else
  {
    // set the tuning capacitor for SW or MW/LW
    rx.setTuneFrequencyAntennaCapacitor((band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) ? 0 : 1);
    if (ssbLoaded)
    {
      rx.setSSB(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep, currentMode);
      rx.setSSBAutomaticVolumeControl(1);
    }
    else
    {
      currentMode = AM;
      rx.setAM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep);
      bfoOn = false;
    }
    rx.setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx); // Soft Mute for AM or SSB
    rx.setAutomaticGainControl(disableAgc, agcNdx);
    rx.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq); // Consider the range all defined current band
    rx.setSeekAmSpacing((band[bandIdx].currentStep > 10) ? 10 : band[bandIdx].currentStep); // Max 10kHz for spacing

  }
  delay(100);
  currentFrequency = band[bandIdx].currentFreq;
  currentStep = band[bandIdx].currentStep;
  idxStep = getStepIndex(currentStep);
  rssi = 0;
  showStatus();
  showCommandStatus((char *) "Band");
}

/**
 *  Switches the Bandwidth
 */
void doBandwidth(int8_t v)
{
  if (currentMode == LSB || currentMode == USB)
  {
    bwIdxSSB = (v == 1) ? bwIdxSSB + 1 : bwIdxSSB - 1;

    if (bwIdxSSB > 5)
      bwIdxSSB = 0;
    else if (bwIdxSSB < 0)
      bwIdxSSB = 5;

    rx.setSSBAudioBandwidth(bandwitdthSSB[bwIdxSSB].idx);
    // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
    if (bandwitdthSSB[bwIdxSSB].idx == 0 || bandwitdthSSB[bwIdxSSB].idx == 4 || bandwitdthSSB[bwIdxSSB].idx == 5)
      rx.setSBBSidebandCutoffFilter(0);
    else
      rx.setSBBSidebandCutoffFilter(1);
  }
  else if (currentMode == AM)
  {
    bwIdxAM = (v == 1) ? bwIdxAM + 1 : bwIdxAM - 1;
    if (bwIdxAM > 6)
      bwIdxAM = 0;
    else if (bwIdxAM < 0)
      bwIdxAM = 6;

    rx.setBandwidth(bandwitdthAM[bwIdxAM].idx, 1);
  }
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  showBandwitdth();
  // showCommandStatus();
  elapsedCommand = millis();
}

/**
 * Show cmd on display. It means you are setting up something.  
 */
void showCommandStatus(char * currentCmd)
{
  display.setCursor(40, 0);
  display.print(currentCmd);
  display.display();  
}

/**
 * Show menu options
 */
void showMenu() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(menu[menuIdx]);
  display.display();
  showCommandStatus( (char *) "Menu");
}

/**
 *  AGC and attenuattion setup
 */
void doAgc(int8_t v) {
  agcIdx = (v == 1) ? agcIdx + 1 : agcIdx - 1;
  if (agcIdx < 0 )
    agcIdx = 35;
  else if ( agcIdx > 35)
    agcIdx = 0;
  disableAgc = (agcIdx > 0); // if true, disable AGC; esle, AGC is enable
  if (agcIdx > 1)
    agcNdx = agcIdx - 1;
  else
    agcNdx = 0;
  rx.setAutomaticGainControl(disableAgc, agcNdx); // if agcNdx = 0, no attenuation
  showAgcAtt();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Gets the current step index.
 */
int getStepIndex(int st)
{
  for (int i = 0; i < lastStep; i++)
  {
    if (st == tabStep[i])
      return i;
  }
  return 0;
}

/**
 * Switches the current step
 */
void doStep(int8_t v)
{
  idxStep = (v == 1) ? idxStep + 1 : idxStep - 1;
  if (idxStep > lastStep)
    idxStep = 0;
  else if (idxStep < 0)
    idxStep = lastStep;
  currentStep = tabStep[idxStep];
  rx.setFrequencyStep(currentStep);
  band[bandIdx].currentStep = currentStep;
  rx.setSeekAmSpacing((currentStep > 10) ? 10 : currentStep); // Max 10kHz for spacing
  showStep();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Switches to the AM, LSB or USB modes
 */
void doMode(int8_t v)
{
  if (currentMode != FM)
  {
    if (v == 1)  { // clockwise
      if (currentMode == AM)
      {
        // If you were in AM mode, it is necessary to load SSB patch (avery time)
        rx.setI2CFastModeCustom(700000); 
        rx.loadPatch(ssb_patch_content, size_content, bandwitdthSSB[bwIdxSSB].idx);
        rx.setI2CFastModeCustom(100000); 
        ssbLoaded = true;
        currentMode = LSB;
      }
      else if (currentMode == LSB)
        currentMode = USB;
      else if (currentMode == USB)
      {
        currentMode = AM;
        bfoOn = ssbLoaded = false;
      }
    } else { // and counterclockwise
      if (currentMode == AM)
      {
        // If you were in AM mode, it is necessary to load SSB patch (avery time)
        rx.setI2CFastModeCustom(700000);
        rx.loadPatch(ssb_patch_content, size_content, bandwitdthSSB[bwIdxSSB].idx);
        rx.setI2CFastModeCustom(100000);
        ssbLoaded = true;
        currentMode = USB;
      }
      else if (currentMode == USB)
        currentMode = LSB;
      else if (currentMode == LSB)
      {
        currentMode = AM;
        bfoOn = ssbLoaded = false;
      }
    }
    // Nothing to do if you are in FM mode
    band[bandIdx].currentFreq = currentFrequency;
    band[bandIdx].currentStep = currentStep;
    useBand();
  }
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
  // showStatus();
  // disableCommands();
}

/**
 * Sets the audio volume
 */
void doVolume( int8_t v ) {
  if ( v == 1)
    rx.volumeUp();
  else
    rx.volumeDown();

  showVolume();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 *  This function is called by the seek function process.
 */
void showFrequencySeek(uint16_t freq)
{
  currentFrequency = freq;
  showFrequency();
}

/**
 *  Find a station. The direction is based on the last encoder move clockwise or counterclockwise
 */
void doSeek()
{
  if ((currentMode == LSB || currentMode == USB)) return; // It does not work for SSB mode
  
  rx.seekStationProgress(showFrequencySeek, seekDirection);
  currentFrequency = rx.getFrequency();
  
}

/**
 * Sets the Soft Mute Parameter
 */
void doSoftMute(int8_t v)
{
  softMuteMaxAttIdx = (v == 1) ? softMuteMaxAttIdx + 1 : softMuteMaxAttIdx - 1;
  if (softMuteMaxAttIdx > 32)
    softMuteMaxAttIdx = 0;
  else if (softMuteMaxAttIdx < 0)
    softMuteMaxAttIdx = 32;

  rx.setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx);
  showSoftMute();
  elapsedCommand = millis();
}

/**
 *  Menu options selection
 */
void doMenu( int8_t v) {
  int8_t lastOpt;
  menuIdx = (v == 1) ? menuIdx + 1 : menuIdx - 1;
  lastOpt = ((currentMode == LSB || currentMode == USB)) ? lastMenu : lastMenu - 1;
  if (menuIdx > lastOpt)
    menuIdx = 0;
  else if (menuIdx < 0)
    menuIdx = lastOpt;

  showMenu();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}


/**
 * Starts the MENU action process
 */
void doCurrentMenuCmd() {
  disableCommands();
  switch (currentMenuCmd) {
    case 1:                 // STEP
      cmdStep = true;
      showStep();
      break;
    case 2:                 // MODE
      cmdMode = true;
      // lcd.clear();
      showMode();
      break;
    case 3:                 // BW
      cmdBandwidth = true;
      showBandwitdth();
      break;
    case 4:                 // AGC/ATT
      cmdAgc = true;
      showAgcAtt();
      break;
    case 5:                 // VOLUME
      cmdVolume = true;
      showVolume();
      break;
    case 6: 
      cmdSoftMuteMaxAtt = true;
      showSoftMute();  
      break;
    case 7:
      bfoOn = true;
      if ((currentMode == LSB || currentMode == USB)) {
        showBFO();
       }
      // showFrequency();
      break;
    default:
        showStatus();
        doSeek();
      break;
  }
  currentMenuCmd = -1;
  elapsedCommand = millis();
}

/**
 * Main loop
 */
void loop()
{
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (bfoOn & (currentMode == LSB || currentMode == USB))
    {
      currentBFO = (encoderCount == 1) ? (currentBFO + currentBFOStep) : (currentBFO - currentBFOStep);
      rx.setSSBBfo(currentBFO);
      showBFO();
    }
    else if (cmdMenu)
      doMenu(encoderCount);
    else if (cmdMode)
      doMode(encoderCount);
    else if (cmdStep)
      doStep(encoderCount);
    else if (cmdAgc)
      doAgc(encoderCount);
    else if (cmdBandwidth)
      doBandwidth(encoderCount);
    else if (cmdVolume)
      doVolume(encoderCount);
    else if (cmdSoftMuteMaxAtt)
      doSoftMute(encoderCount);
    else if (cmdBand)
      setBand(encoderCount);
    else
    {
      if (encoderCount == 1) {
        rx.frequencyUp();
        seekDirection = 1;
      }
      else{
        rx.frequencyDown();
        seekDirection = 0;
      }
      // Show the current frequency only if it has changed
      currentFrequency = rx.getFrequency();
      showFrequency();
    }
    encoderCount = 0;
    // elapsedCommand = millis();
  }
  else
  {
    if ( digitalRead(ENCODER_PUSH_BUTTON) == LOW ) {
      countClick++;
      if (cmdMenu ) {
        currentMenuCmd = menuIdx;
        doCurrentMenuCmd();
      } else if ( countClick == 1) { // If just one click, you can select the band by rotating the encoder
        if ( (cmdStep | cmdBandwidth | cmdAgc | cmdVolume | cmdSoftMuteMaxAtt | cmdMode | cmdBand) ) {
          disableCommands();
          showStatus();
          showCommandStatus((char *) "VFO ");
        } else {
          cmdBand = !cmdBand;
          showCommandStatus((char *) "Band");
        }
      } else { // GO to MENU if more than one click in less than 1/2 seconds.
        cmdMenu = !cmdMenu;
        if (cmdMenu) showMenu();
      }
      delay(MIN_ELAPSED_TIME);
      elapsedCommand = millis();
    }
  }


  // Show RSSI status only if this condition has changed
  if ((millis() - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME * 6)
  {
    rx.getCurrentReceivedSignalQuality();
    int aux = rx.getCurrentRSSI();
    if (rssi != aux &&  !(cmdStep | cmdBandwidth | cmdAgc | cmdVolume | cmdSoftMuteMaxAtt | cmdMode) )
    {
      rssi = aux;
      showRSSI();
    }
    elapsedRSSI = millis();
  }
  /*
  // Disable commands control
  if ((millis() - elapsedCommand) > ELAPSED_COMMAND)
  {
    if ((currentMode == LSB || currentMode == USB))
    {
      bfoOn = false;
      showBFO();
    }
    showStatus();
    disableCommands();
    elapsedCommand = millis();
  }
  */

  if ( (millis() - elapsedClick) > ELAPSED_CLICK ) {
    countClick = 0;
    elapsedClick = millis();
  }
  delay(5);
}