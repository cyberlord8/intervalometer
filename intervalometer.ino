/*
  InterValometer sketch for PicoW Copyright 2023 Timothy Millea - Released under GPLv3 license

  This sketch closes a relay or other device/component that controls the remote shutter on a digital camera.

  Consult camera manual on how to connect relay to remote shutter for your specific camera.

  Most cameras have a short to ground remote shutter control, but check your camera's 
  specifics so you don't damage your camera.
*/
ulong debounceTime = 0;

int runningLED = LED_BUILTIN;  //onboard Pico LED
int shutterRelay = 11;         // pin for camera shutter release control relayint
int runButton = 12;            //input pin to start/stop the Intervalometer sequence
// int stopButton = 13;           //input pin to start/stop the Intervalometer sequence

int numberExposures = 60;                      //number of exposure to take in sequence
int numberExposuresCounter = numberExposures;  //counter for number of exposures left
int intervalLength = 60000;                    //shutter open interval (milli seconds)
int intervalSpacing = 250;                     //shutter closed interval (milli seconds)
int mirrorDelay = 0;                           //mirror lag delay (milli seconds)

bool isRunning = false;  //is sequence routine running
bool hasStopped = false;
bool stringComplete = false;  //do we have a complete string from serial port input

String inputString;  // a string to hold incoming data

void buttonInterupt() {
  if (millis() - debounceTime > 250) {
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

  pinMode(shutterRelay, OUTPUT);     //relay goes on this pin
  pinMode(runningLED, OUTPUT);       //onboard LED
  pinMode(runButton, INPUT_PULLUP);  //push button to start exposure sequence

  attachInterrupt(digitalPinToInterrupt(runButton), buttonInterupt, FALLING);

  digitalWrite(runningLED, HIGH);
  delay(5000);
  digitalWrite(runningLED, LOW);

  Serial.println();
  Serial.println();
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println("                          InterValometer");
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println("~ Command mode");
  Serial.println("Current commands:");
  Serial.println("~NUM - Sets the number of exposures to take (-1 infinite)");
  Serial.println("~INT - Sets the length of the shutter open in milli seconds");
  Serial.println("~SPC - Sets the delay between sequences (shutter closed)");
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
}  //setup

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

    digitalWrite(shutterRelay, HIGH);
    delay(intervalLength + mirrorDelay);
    digitalWrite(shutterRelay, LOW);
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
    digitalWrite(shutterRelay, LOW);
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
    if (inputString.startsWith("INT")) {
      inputString.remove(0, 4);        //remove "INTV" and the space
      inputString.trim();              //trim any white space
      if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current Interval Length: ");
        Serial.println(intervalLength);  //print the current WPM
        return;
      }
      intervalLength = inputString.toInt();  // set WPM to value supplied
      Serial.print("Set INT to: ");
      Serial.println(intervalLength);
    }  // if INT command
    else if (inputString.startsWith("NUM")) {
      inputString.remove(0, 4);        //remove "NUM" and the space
      inputString.trim();              //trim any white space
      if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current NUM: ");
        Serial.println(numberExposures);  //print the current NUM
        return;
      }
      numberExposures = inputString.toInt();  // set NUM to value supplied
      if (!isRunning) {
        numberExposuresCounter = numberExposures;
      }
      Serial.print("Set NUM to: ");
      Serial.println(numberExposures);
    }  // if NUM command
    else if (inputString.startsWith("SPC")) {
      inputString.remove(0, 4);        //remove "INTV" and the space
      inputString.trim();              //trim any white space
      if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current SPC: ");
        Serial.println(intervalSpacing);  //print the current WPM
        return;
      }
      intervalSpacing = inputString.toInt();  // set WPM to value supplied
      Serial.print("Set SPC to: ");
      Serial.println(intervalLength);
    }  // if SPC command
    else if (inputString.startsWith("MIR")) {
      inputString.remove(0, 4);        //remove "INTV" and the space
      inputString.trim();              //trim any white space
      if (inputString.toInt() == 0) {  //if no value was supplied
        Serial.print("Current MIR: ");
        Serial.println(mirrorDelay);  //print the current WPM
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
