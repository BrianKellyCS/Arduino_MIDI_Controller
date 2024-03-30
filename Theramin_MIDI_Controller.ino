#include <NewPing.h>
#include <stdlib.h> // for random function
#include <Arduino_FreeRTOS.h>
#include "queue.h"
#include <LiquidCrystal.h>


// set up the ultrasonic sensor
#define TRIGGER_PIN1  8
#define ECHO_PIN1     9
#define TRIGGER_PIN2  10
#define ECHO_PIN2     11
#define MAX_DISTANCE1 80 // Maximum distance to measure (in centimeters).
#define MAX_DISTANCE2 20
#define MIDI_CHANNEL1 1
#define MIDI_CHANNEL2 2
#define DRUM_BUTTON 2



// Initialize the LCD
// LCD pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd( 12,13,4,5,6,7);

const char *note_names[] = {
  "C ", "C#", "D ", "D#", "E ", "F ",
  "F#", "G ", "G#", "A ", "A#", "B "
};

byte Sound[8] = {
  0b00001,
  0b00011,
  0b00101,
  0b01001,
  0b01001,
  0b01011,
  0b11011,
  0b11000
};

// Define scales
int harmonic_minor_scale[] = {0, 2, 3, 5, 7, 8, 11, 12};
int major_scale[] = {0, 2, 4, 5, 7, 9, 11, 12};
int minor_pentatonic_scale[] = {0, 3, 5, 7, 10, 12};


// Define an array of scale pointers
int *available_scales[] = {harmonic_minor_scale, major_scale, minor_pentatonic_scale};

// Define an array of scale sizes
int scale_sizes[] = {8, 8, 6};

// Define the current scale index (0-5), change this to switch scales
int current_scale_index = 0; // Example: set to 1 for major_scale

int speeds[] = {300,185,100}; //Slow, Normal, Fast
int bpm = speeds[1]; //Normal 


int START_NOTE = 24;
byte VELOCITY = 127;
const int MIDI_OFF = 128;
const int MIDI_ON = 144;
int volume = 0;
int note = 0;
int current_note = 0;
int drum_volume = 127;
int kickCount = 0;


NewPing sonar1(TRIGGER_PIN1, ECHO_PIN1, MAX_DISTANCE1); // NewPing setup of pins and maximum distance.
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE2); // NewPing setup of pins and maximum distance.

byte last_note = note;

bool stoppedFlag = false;



QueueHandle_t sensorDataQueue;

volatile bool drumsEnabled = false;
volatile unsigned long lastDebounceTime = 0;


typedef struct 
{
  int current_note;
  int volume;
} SensorData;

void setup() 
{
  // initialize serial communication
  
  lcd.begin(16,2);
  Serial.begin(9600);
  //Music character on LCD
  lcd.createChar(0, Sound);

  // Create the queue
  sensorDataQueue = xQueueCreate(1, sizeof(SensorData));




  // Create tasks for handleSensors and handleDrums
  xTaskCreate(handleSensorsTask, "HandleSensors", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(handleDrumsTask, "HandleDrums", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(handleDrumButtonTask, "HandleDrumButton", 64, NULL, 1, NULL);



  // Start the scheduler
  vTaskStartScheduler();

  


}

void loop() 
{
}








void handleDrumButtonTask(void *pvParameters) 
{
  // Set the button pin mode
  pinMode(DRUM_BUTTON, INPUT_PULLUP);

  // Initialize the button state and last state
  int buttonState = HIGH;
  int lastButtonState = HIGH;

  for (;;) {
    // Read the button state
    int reading = digitalRead(DRUM_BUTTON);

    // Check if the button state has changed
    if (reading != lastButtonState) {
      // Reset the debounce timer
      lastDebounceTime = millis();
    }

    // If enough time has passed since the last bounce, update the button state
    if ((millis() - lastDebounceTime) > 20) {
      // If the button state has changed
      if (reading != buttonState) {
        buttonState = reading;

        // If the button is pressed, toggle the drumsEnabled state
        if (buttonState == LOW) {
          drumsEnabled = !drumsEnabled;
        }
      }
    }

    // Save the last button state
    lastButtonState = reading;

    // Delay the task to allow other tasks to run
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}



void handleSensorsTask(void *pvParameters) 
{
  for (;;) {
    handleSensors();

    SensorData sensorData;
    sensorData.current_note = current_note;
    sensorData.volume = volume;


    // Display sensor note and volume on LCD
    if(volume > 0)
    {
      lcd.setCursor(0,0); // Set cursor to first column of first row
      lcd.print("MIDI: ");
      lcd.print(current_note);
      lcd.setCursor(0,1);
      lcd.write(byte(0));
      int note_index = (current_note - START_NOTE) % 12;
      lcd.print(": ");
      lcd.print(note_names[note_index]);
    }



    xQueueSend(sensorDataQueue, &sensorData, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(bpm));
  }
}

void handleDrumsTask(void *pvParameters) 
{ 
  for (;;) {
    SensorData sensorData;

    if (xQueueReceive(sensorDataQueue, &sensorData, portMAX_DELAY)) {
      current_note = sensorData.current_note;
      volume = sensorData.volume;
      handleMidiChannel2();
    }

    vTaskDelay(pdMS_TO_TICKS(bpm));
  }
}


void handleSensors() 
{
  int distance1 = sonar1.ping_cm(); // Send ping, get distance in cm
  int distance2 = sonar2.ping_cm(); // Send ping, get distance in cm
  
  // Map distance to volume
  volume = map(distance2, 0, MAX_DISTANCE2, 0, 127);



  // Get the correct scale size based on the current scale index
  int scale_size = scale_sizes[current_scale_index];

  int index = map(distance1, 0, MAX_DISTANCE1, 0, scale_size * 4); // Map distance1 to index in the scale (4 octaves)
  int octave = index / scale_size; // Determine which octave we are in
  int note_index = index % scale_size; // Determine which note in the octave

  // Use the current scale index to select the scale
  int *current_scale = available_scales[current_scale_index];

  note = START_NOTE + current_scale[note_index] + 12 * octave;
  current_note = note;

  handleMidiChannel1(distance1, distance2, current_note);
}


void handleMidiChannel1(int distance1, int distance2, int current_note) 
{
  if (distance1 > 0)
  {
    if (current_note != last_note)
    {
      MIDI_TX(MIDI_OFF, last_note, 0, MIDI_CHANNEL1);
      MIDI_TX(MIDI_ON, current_note, volume, MIDI_CHANNEL1);
      last_note = current_note;
    }
    else
    {
      MIDI_TX(MIDI_ON, current_note, volume, MIDI_CHANNEL1);
    }
    stoppedFlag = false;
  }
  else if (distance1 == 0 && stoppedFlag == false)
  {
    MIDI_TX(MIDI_OFF, current_note, 0, MIDI_CHANNEL1);
    MIDI_TX(MIDI_OFF, last_note, 0, MIDI_CHANNEL1);
    stoppedFlag = true;
  }
}


void handleMidiChannel2() 
{
  int kick_note = 36;
  int snare_note = 38;
  int crash_note = 49;
  

  
  if (drumsEnabled)
  {
    // Send kick note
    MIDI_TX(MIDI_ON, kick_note, drum_volume, MIDI_CHANNEL2);

    // Increment kick count and reset to 1 if it reaches 5
    kickCount++;
    if (kickCount == 5) {
      kickCount = 1;
    }

    // Send snare and crash notes when kick count is 3
    if (kickCount == 3) {
      // Send snare note
      MIDI_TX(MIDI_ON, snare_note, drum_volume, MIDI_CHANNEL2);


      // Send crash note
      MIDI_TX(MIDI_ON, crash_note, drum_volume, MIDI_CHANNEL2);

    }
  }

}


// Transmit MIDI Message
void MIDI_TX(byte MESSAGE, byte PITCH, byte VELOCITY, byte CHANNEL) 
{
  // Construct the MIDI message
  byte midiMessage = MESSAGE | ((CHANNEL - 1) & 0x0F);

  // Send the MIDI message
  Serial.write(midiMessage);
  Serial.write(PITCH);
  Serial.write(VELOCITY);
}
