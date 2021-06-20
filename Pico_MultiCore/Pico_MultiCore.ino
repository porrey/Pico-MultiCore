//
// The amount of time, in milliseconds, to
// keep the LED on or off.
//
#define LED_DELAY 1000

//
// The amount of time, in milliseconds, to
// keep Core 1 paused.
//
#define PAUSE_TIME 5000

//
// Keeps track of the last known state
// of the LED state.
//
volatile uint32_t _previousValue = LOW;

//
// Indicates if Core 1 currently paused or not.
//
volatile bool _paused = false;

//
// The time when Core 1 was paused.
//
volatile uint32_t _timeStamp = 0;

//
// Setup for Core 0.
//
void setup()
{
  Serial.begin(115200);
}

//
// Setup for Core 1.
//
void setup1()
{
  pinMode(LED_BUILTIN, OUTPUT);
}

//
// Loop for Core 0.
//
void loop()
{
  //
  // Checked if enough time has elapsed
  // to resume the core 1.
  //
  if (_paused && (millis() - _timeStamp) > PAUSE_TIME)
  {
    Serial.println("Resuming second core.");
    rp2040.resumeOtherCore();
    _paused = false;
  }

  //
  // Check the state of the BOOTSEL button. BOOTSEL
  // cannot be checked when core 1 is paused.
  //
  if (!_paused && BOOTSEL)
  {
    //
    // Wait for the button to be released.
    //
    while (BOOTSEL) {}

    //
    // Get the current timestamp.
    //
    _timeStamp = millis();

    Serial.print("Pausing the second core for "); Serial.print(PAUSE_TIME / 1000); Serial.println(" seconds.");
    rp2040.idleOtherCore();
    _paused = true;
  }

  //
  // Check if data is available from the other core.
  //
  if (rp2040.fifo.available() > 0)
  {
    //
    // Get the latest about LED status
    // from the FIFO.
    //
    uint32_t value = rp2040.fifo.pop();

    //
    // Display the state of the LED.
    //
    if (value == HIGH && _previousValue == LOW)
    {
      Serial.println("The LED is ON.");
      _previousValue = HIGH;
    }
    else if (value == LOW && _previousValue == HIGH)
    {
      Serial.println("The LED is OFF.");
      _previousValue = LOW;
    }
  }
}

//
// Loop for Core 1.
//
void loop1()
{
  //
  // Turn the built-in LED on, send the state to
  // the other core and pause.
  //
  digitalWrite(LED_BUILTIN, HIGH);
  rp2040.fifo.push_nb(HIGH);
  delay(LED_DELAY);

  //
  // Turn the built-in LED off, send the state to
  // the other core and pause.
  //
  digitalWrite(LED_BUILTIN, LOW);
  rp2040.fifo.push_nb(LOW);
  delay(LED_DELAY);
}
