# Arduino MIDI Controller Project

This project uses an Arduino Uno, equipped with two ultrasonic sensors, a Liquid Crystal Display and a button, into a Theremin style MIDI Controller. Using FreeRTOS for multitasking, this controller sends MIDI signals to a DAW (Digital Audio Workstation), creating musical notes based on the proximity of the user's hands to the sensors. This README provides detailed instructions on how to clone the project, set up the required software, and operate the device.

## Prerequisites

Before you begin, ensure you have the following software installed on your computer:

- **Arduino IDE**: Used for uploading the sketch to the Arduino Uno.
- **Hairless MIDI<->Serial Bridge**: Bridges the gap between serial and MIDI data.
- **LoopMIDI**: Creates a virtual loopback MIDI cable.
- **Reaper (DAW)**: The DAW that will receive and interpret MIDI data from the Arduino.

You can download these from their respective websites:

- Hairless MIDI<->Serial Bridge: [https://projectgus.github.io/hairless-midiserial/](https://projectgus.github.io/hairless-midiserial/)
- LoopMIDI: [https://www.tobias-erichsen.de/software/loopmidi.html](https://www.tobias-erichsen.de/software/loopmidi.html)
- Reaper: [https://www.reaper.fm/download.php](https://www.reaper.fm/download.php)

## Installation

### Cloning the Project

1. Open a terminal or command prompt.
2. Navigate to the directory where you want to clone the project.
3. Execute the following command to clone the repository:
   ```bash
   git clone https://github.com/BrianKellyCS/Arduino_MIDI_Controller
   ```
### Setting Up the Project Directory

After cloning, navigate into the project directory:
    ```
    cd Arduino_MIDI_Controller
    ```

### Software Setup

Follow the installation instructions for each of the pre-requisite software mentioned above. Ensure that each software is correctly configured before proceeding.

## Configuration

### Arduino

1. Connect the Arduino Uno to your computer via USB.
2. Open the Arduino IDE.
3. Open the project sketch file from the cloned project directory.
4. Select the correct board and port in the Arduino IDE.
5. Upload the sketch to your Arduino Uno.

### Hairless MIDI<->Serial Bridge

1. Open Hairless MIDI<->Serial Bridge.
2. Set the serial port to the one your Arduino is connected to.
3. Set the baud rate to 9600 (matching the Arduino sketch).

### LoopMIDI

1. Open LoopMIDI.
2. Create a new virtual MIDI port. Note the name of the port.

### Reaper (DAW)

1. Open Reaper.
2. Go to Preferences > MIDI Devices.
3. Enable the virtual MIDI port you created in LoopMIDI as an input device.
4. To fully utilize this Theremin MIDI Controller, ensure you have MIDI instrument plugins and a drum plugin set up in your DAW. These plugins will interpret the MIDI signals from the controller and produce the corresponding sounds and rhythms.

## Using the Device

1. Position your hands near the ultrasonic sensors to control the MIDI signals. The distance from the sensors will determine the note and volume.
2. Press the drum button to toggle drum functionality. This will enable or disable MIDI Channel 2 output for drums based on sensor input.
3. The LCD display will show the current note and volume based on your hand's proximity to the sensors.
4. Your movements are now translated into MIDI data, which is sent to Reaper through the Hairless MIDI<->Serial Bridge and LoopMIDI setup.
   

## Troubleshooting

- **No MIDI data in Reaper**: Ensure that Hairless MIDI<->Serial Bridge is correctly configured and that the correct COM port is selected. Also, verify that LoopMIDI's virtual port is selected as the output in Hairless MIDI and as the input in Reaper.
- **Unresponsive sensors**: Check the wiring of the ultrasonic sensors to the Arduino and ensure they are powered on.
- **Errors uploading to Arduino**: Verify that the correct board and port are selected in the Arduino IDE. Also, ensure that no other programs are using the COM port (e.g., Hairless MIDI<->Serial Bridge must be closed while uploading).
