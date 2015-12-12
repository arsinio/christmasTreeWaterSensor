#include <Adafruit_NeoPixel.h>


#define BRIGHTNESS_RED          40
#define BRIGHTNESS_GREEN        32
#define BRIGHTNESS_BLUE         32

#define SENSOR_PIN              0

//#define SENSOR_VALUE_EMPTY      275
#define SENSOR_VALUE_LOWWARN    300
#define SENSOR_VALUE_FULL       335
#define SENSOR_HYSTERESIS       15

#define NUM_SAMPLES             50


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, 7, NEO_GRB + NEO_KHZ800);
static uint16_t sensorVals[NUM_SAMPLES];
static uint8_t currSampleIndex = 0;


void showWreath(uint8_t pcntFullIn)
{
  // do the bow first
  pixels.setPixelColor(0, BRIGHTNESS_RED, 0, 0);
  pixels.setPixelColor(1, BRIGHTNESS_RED, 0, 0);

  // calculate the number of segments to show
  uint8_t numSegs = ((uint16_t)pcntFullIn * ((uint16_t)11)) / 100;

  // now the left side of the wreath
  for( uint8_t i = 2; i < numSegs+2; i++ )
  {
    pixels.setPixelColor(i, 0, BRIGHTNESS_GREEN, 0);
  }
  for( uint8_t i = numSegs+2; i < 13; i++ )
  {
    pixels.setPixelColor(i, 0, 0, 0);
  }

  // now the right side of the wreath
  for( uint8_t i = 13; i < 13 + (11 - numSegs); i++ )
  {
    pixels.setPixelColor(i, 0, 0, 0);
  }
  for( uint8_t i = 13 + (11 - numSegs); i < 24; i++ )
  {
    pixels.setPixelColor(i, 0, BRIGHTNESS_GREEN, 0);
  }
 
  // push to the pixels
  pixels.show();
}


void pulseBlue(void)
{
  static bool dirUp = true;
  static uint8_t currBrightness = 0;

  if( dirUp )
  {
    currBrightness++;
    if( currBrightness == BRIGHTNESS_BLUE ) dirUp = false;
  }
  else
  {
    currBrightness--;
    if( currBrightness == 0 ) dirUp = true;
  }

  // send to the pixels and show
  for( uint8_t i = 0; i < 24; i++ )
  {
    pixels.setPixelColor(i, 0, 0, currBrightness);
  }
  pixels.show();

  // delay so we we're not a strobe light
  delay(20);
}


void setup()
{
  pixels.begin();
  pixels.show();
  
  Serial.begin(9600);
}


void loop()
{ 
  // average our sensor value
  sensorVals[currSampleIndex++] = 1024 - analogRead(SENSOR_PIN);
  if( currSampleIndex == NUM_SAMPLES ) currSampleIndex = 0;
  uint16_t sensorVal = 0;
  for( uint8_t i = 0; i < NUM_SAMPLES; i++ )
  {
    sensorVal += sensorVals[i];
  }
  sensorVal /= NUM_SAMPLES;
  
  Serial.println(sensorVal);

  static bool wasPreviouslyLow = false;
  if( (sensorVal < SENSOR_VALUE_LOWWARN) || 
      (wasPreviouslyLow && (sensorVal < (SENSOR_VALUE_LOWWARN + SENSOR_HYSTERESIS))) )
  {
    wasPreviouslyLow = true;
    pulseBlue();
  }
  else if( (!wasPreviouslyLow && (sensorVal >= SENSOR_VALUE_LOWWARN)) ||
           (wasPreviouslyLow && (sensorVal >= (SENSOR_VALUE_LOWWARN + SENSOR_HYSTERESIS))) )
  {
    wasPreviouslyLow = false;
    int clampedSensorVal = (sensorVal > SENSOR_VALUE_FULL) ? SENSOR_VALUE_FULL : sensorVal;
    showWreath( ((clampedSensorVal - SENSOR_VALUE_LOWWARN) * 100) / (SENSOR_VALUE_FULL - SENSOR_VALUE_LOWWARN) );
  }
}
