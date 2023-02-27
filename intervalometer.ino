/*
  InterValometer sketch for PicoW Copyright 2023 Timothy Millea - Released under GPLv3 license

  This sketch closes a relay or other device/component that controls the remote shutter on a digital camera.

  Consult camera manual on how to connect relay to remote shutter for your specific camera.

  Most cameras have a short to ground remote shutter control, but check your camera's 
  specifics so you don't damage your camera.
*/

#define DEFAULT_EXPOSURES -1;
#define DEFAULT_INTERVAL 1000;
#define DEFAULT_SPACING 250;
#define DEFAULT_MIRROR 0;

ulong debounceTime = 0;
ulong bounceCheck = 250;

int runningLED = LED_BUILTIN;  //onboard Pico LED
int shutterRelayPin = 11;      // pin for camera shutter release control relayint
int runButton = 12;            //input pin to start/stop the Intervalometer sequence

long numberExposures = DEFAULT_EXPOSURES;       //number of exposure to take in sequence
long numberExposuresCounter = numberExposures;  //counter for number of exposures left
ulong intervalLength = DEFAULT_INTERVAL;        //shutter open interval (milli seconds)
ulong intervalSpacing = DEFAULT_SPACING;        //shutter closed interval (milli seconds)
ulong mirrorDelay = DEFAULT_MIRROR;             //mirror lag delay (milli seconds)

bool isRunning = false;  //is sequence routine running
bool hasStopped = false;
bool stringComplete = false;  //do we have a complete string from serial port input

String inputString;  // a string to hold incoming data

void buttonInterupt() {
  if ((millis() - debounceTime) > bounceCheck) {
    if (!isRunning) {
      isRunning = true;
      numberExposuresCounter = numberExposures;
      Serial.print("Number of Expsoures: ");
      Serial.println(numberExposures);
      Serial.print("Exposure time: ");
      Serial.println(intervalLength);
      Serial.print("Exposure interval: ");
      Serial.println(intervalSpacing);
      Serial.print("Mirror lag: ");
      Serial.println(mirrorDelay);
      Serial.println("Starting exposure sequence...");
      Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }  //if running sequence
    else {
      //uncomment to halt exposure sequence immediately
      // digitalWrite(runningLED, LOW);
      // digitalWrite(shutterRelay, LOW);
      isRunning = false;
      hasStopped = true;
      numberExposuresCounter = numberExposures;
      Serial.println("Stopping exposure sequence...");
    }  //else not running sequence
  }    //if we haven't detected a switch bounce
  debounceTime = millis();
}  //buttonInterupt

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(shutterRelayPin, OUTPUT);  //relay goes on this pin
  pinMode(runningLED, OUTPUT);       //onboard LED
  pinMode(runButton, INPUT_PULLUP);  //push button to start exposure sequence

  attachInterrupt(digitalPinToInterrupt(runButton), buttonInterupt, FALLING);

  digitalWrite(runningLED, HIGH);
  delay(5000);
  digitalWrite(runningLED, LOW);

  printMenu();
}  //setup

void printMenu() {
  Serial.println();
  Serial.println();
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println("                          InterValometer");
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println("~ Command mode");
  Serial.println("Current commands:");
  Serial.println("~NUM - Sets the number of exposures to take (-1 infinite)");
  Serial.println("~EXP - Sets exposure time in milli seconds (shutter open)");
  Serial.println("~INT - Sets the interval between exposures (shutter closed)");
  Serial.println("~MIR - Sets the mirror lift delay (must match camera setting)");
  Serial.println("~RUN - Starts the exposure sequence using the prescribed values");
  Serial.println("~STP - Stops the exposure sequence");
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.print("Exposure count: ");
  Serial.println(numberExposures);
  Serial.print("Exposure length: ");
  Serial.println(intervalLength);
  Serial.print("Delay between exposures: ");
  Serial.println(intervalSpacing);
  Serial.print("Mirror Delay: ");
  Serial.println(mirrorDelay);
  Serial.println();
  Serial.println("Ready!");
}  // printMenu

void loop() {
  // put your main code here, to run repeatedly:
  if (isRunning) {
    Serial.println("Running...");
    digitalWrite(runningLED, HIGH);
    Serial.print("Num: ");
    Serial.print(numberExposuresCounter);
    Serial.print(" - Int: ");
    Serial.print(intervalLength);
    Serial.print(" - Spc: ");
    Serial.print(intervalSpacing);
    Serial.print(" - Mir: ");
    Serial.println(mirrorDelay);
    if (numberExposuresCounter > 0) {
      float timeRemaining = ((numberExposuresCounter * intervalLength) + (numberExposuresCounter * intervalSpacing)) / 1000.0;
      if (timeRemaining < 60) {
        Serial.print("Seconds remaining: ");
        Serial.println(timeRemaining);
      }  //if less than 60 seconds
      else if (timeRemaining < 3600) {
        Serial.print("Time remaining: ");
        int hours = 0;
        int minutes = 0;
        float seconds = 0.0;
        minutes = timeRemaining / 60.0;
        seconds = timeRemaining - (minutes * 60.0);
        Serial.print(hours);
        Serial.print(":");
        Serial.print(minutes);
        Serial.print(":");
        Serial.println(seconds);
      }  //else if less than 3600 seconds
      else {
        Serial.print("Time remaining: ");
        int hours = 0;
        int minutes = 0;
        float seconds = 0.0;
        hours = timeRemaining / 60 / 60;
        minutes = (timeRemaining - (hours * 60 * 60)) / 60.0;
        if (minutes == 0) {
          seconds = (timeRemaining - (hours * 60 * 60));
        } else {
          seconds = timeRemaining - (hours * 60 * 60) - (minutes * 60.0);
        }
        Serial.print(hours);
        Serial.print(":");
        Serial.print(minutes);
        Serial.print(":");
        Serial.println(seconds);
      }  //else must be more than an hour remaining
    }    //if not infinite sequence

    digitalWrite(shutterRelayPin, HIGH);
    delay(intervalLength + mirrorDelay);
    digitalWrite(shutterRelayPin, LOW);
    delay(intervalSpacing);
    mySerialEvent();           //check serial port for input
    if (stringComplete) {      //if serial read has completed
      handleStringComplete();  //handle string input
    }                          //if read serial console complete
    if (numberExposuresCounter > -1) {
      if (--numberExposuresCounter == 0) {
        numberExposuresCounter = numberExposures;
        isRunning = false;
        digitalWrite(runningLED, LOW);
        Serial.print("Completed ");
        Serial.print(numberExposures);
        Serial.println(" exposures...");
        Serial.println("Stopping exposure sequence...");
        Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      }  //if we've reached the end of the sequence
    }    //if we're running a finite sequence
  }      //if running exposure sequence loop
  else {
    digitalWrite(runningLED, LOW);
    digitalWrite(shutterRelayPin, LOW);
    if (hasStopped) {
      hasStopped = false;
      Serial.println("Stopped!");
      Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }                          //if we just stopped - print "Stopped!"
    mySerialEvent();           //check serial port for input
    if (stringComplete) {      //if serial read has completed
      handleStringComplete();  //handle input string
    }                          //if read serial console complete
  }                            //else not running exposure sequence
}  //loop

void mySerialEvent() {
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    }  //if we have a newline then we're done reading serial for now
    if (stringComplete == false) {
      inputString += inChar;
    }  //if no newline then append the char to the string
  }    //while stuff ready to read from serial console
}  //serialEvent()

void handleStringComplete() {
  inputString.trim();
  Serial.println(inputString);
  if (inputString.startsWith("~")) {  //check for command character
    inputString.toUpperCase();
    inputString.remove(0, 1);  //remove the '~'
    if (inputString.startsWith("EXP")) {
      inputString.remove(0, 4);  //remove "EXP" and the space
      inputString.trim();        //trim any white space
      if (inputString.endsWith("S")) {
        inputString.remove(inputString.indexOf("S"), 1);
        Serial.println(inputString);
        intervalLength = inputString.toInt() * 1000;
      } else if (inputString.endsWith("M")) {
        inputString.remove(inputString.indexOf("M"), 1);
        Serial.println(inputString);
        intervalLength = inputString.toInt() * 1000 * 60;
      } else if (inputString.endsWith("H")) {
        inputString.remove(inputString.indexOf("H"), 1);
        Serial.println(inputString);
        intervalLength = inputString.toInt() * 1000 * 60 * 60;
      } else if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current exposure time: ");
        Serial.println(intervalLength);  //print the current EXP
        inputString = "";
        stringComplete = false;
        return;
      } else {
        intervalLength = inputString.toInt();  // set WPM to value supplied
      }
      Serial.print("Set EXP to: ");
      Serial.println(intervalLength);
    }  // if INT command
    else if (inputString.startsWith("NUM")) {
      inputString.remove(0, 4);        //remove "NUM" and the space
      inputString.trim();              //trim any white space
      if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current NUM: ");
        Serial.println(numberExposures);  //print the current NUM
        inputString = "";
        stringComplete = false;
        return;
      }
      numberExposures = inputString.toInt();  // set NUM to value supplied
      if (!isRunning) {
        numberExposuresCounter = numberExposures;
      }
      Serial.print("Set NUM to: ");
      Serial.println(numberExposures);
    }  // if NUM command
    else if (inputString.startsWith("INT")) {
      inputString.remove(0, 4);  //remove "INT" and the space
      inputString.trim();        //trim any white space
      if (inputString.endsWith("S")) {
        inputString.remove(inputString.indexOf("S"), 1);
        Serial.println(inputString);
        intervalSpacing = inputString.toInt() * 1000;
      } else if (inputString.endsWith("M")) {
        inputString.remove(inputString.indexOf("M"), 1);
        Serial.println(inputString);
        intervalSpacing = inputString.toInt() * 1000 * 60;
      } else if (inputString.endsWith("H")) {
        inputString.remove(inputString.indexOf("H"), 1);
        Serial.println(inputString);
        intervalSpacing = inputString.toInt() * 1000 * 60 * 60;
      } else if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current exposure interval: ");
        Serial.println(intervalSpacing);  //print the current WPM
        inputString = "";
        stringComplete = false;
        return;
      } else {
        intervalSpacing = inputString.toInt();
      }
      Serial.print("Set INT to: ");
      Serial.println(intervalSpacing);
    }  // if SPC command
    else if (inputString.startsWith("MIR")) {
      inputString.remove(0, 4);        //remove "INTV" and the space
      inputString.trim();              //trim any white space
      if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current MIR: ");
        Serial.println(mirrorDelay);  //print the current WPM
        inputString = "";
        stringComplete = false;
        return;
      }
      mirrorDelay = inputString.toInt();  // set WPM to value supplied
      Serial.print("Set MIR to: ");
      Serial.println(mirrorDelay);
    }  // if MIR command
    else if (inputString.startsWith("RUN") && !isRunning) {
      isRunning = true;
      numberExposuresCounter = numberExposures;
      Serial.print("Number of Expsoures: ");
      Serial.println(numberExposures);
      Serial.print("Exposure time: ");
      Serial.println(intervalLength);
      Serial.print("Exposure interval: ");
      Serial.println(intervalSpacing);
      Serial.print("Mirror lag: ");
      Serial.println(mirrorDelay);
      Serial.println("Starting exposure sequence...");
      Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }  // if RUN command
    else if (inputString.startsWith("STP") && isRunning) {
      isRunning = false;
      hasStopped = true;
      numberExposuresCounter = numberExposures;
      Serial.println("Stopping...");
    }  // if STP command
    else if (inputString.startsWith("RST")) {
      Serial.println("Stopping...");
      isRunning = false;
      hasStopped = true;
      Serial.println("Reseting variables to default values...");
      numberExposures = DEFAULT_EXPOSURES;
      intervalLength = DEFAULT_INTERVAL;
      intervalSpacing = DEFAULT_SPACING;
      mirrorDelay = DEFAULT_MIRROR;
      numberExposuresCounter = numberExposures;
      printMenu();
    }  // if RST command
    else {
      Serial.println("ERROR! Unrecognised command");
    }
  }  //if it's a command
  else {
    Serial.println("ERROR! Commands start with ~");
  }
  //reset variables
  inputString = "";
  stringComplete = false;
}
