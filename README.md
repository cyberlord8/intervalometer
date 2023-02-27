# intervalometer
 
  InterValometer sketch for PicoW - Copyright 2023 Timothy Millea - Released under GPLv3 license
  
  This sketch closes a relay or other device/component that controls the remote shutter on a digital camera.
  
  Consult camera manual on how to connect relay to remote shutter for your specific camera.
  
  Most cameras have a short to ground remote shutter control, but check your camera's specifics so you don't damage your camera.

Two modes of operation:

1. Serial port control through the USB port connected to a PC

      Current commands:

        ~NUM - Sets the number of exposures to take (-1 infinite)
        ~INT - Sets the length of the shutter open in milli seconds
        ~SPC - Sets the delay between sequences (shutter closed)
        ~MIR - Sets the mirror lift delay (must match camera setting)
        ~RUN - Starts the exposure sequence using the prescribed values
        ~STP - Stops the exposure sequence

2. Push button starts and stops exposure sequence, check code for pin number.
