#include <Wire.h>
#include <BH1750.h>
#include <FastLED.h>
#include "config.h"

/* Light sensor */
BH1750 lightMeter;
float light_amount = 0;

/* Led stripe */
CRGB leds[NUM_LEDS];

/* State variables */
unsigned long volatile last_detection[2];
unsigned long last_activation = 99999;
unsigned long last_iteration = 0;
bool light_state = false;

void setup()
{
#if (DEBUG_ENABLED == 1)
  Serial.begin(115200);
#endif

  /* Attach door interrupt callback */
  attachInterrupt(digitalPinToInterrupt(PRESENCE_SENSOR_1), presence_sensor_1_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(PRESENCE_SENSOR_2), presence_sensor_2_isr, RISING);

  /* Do hardware setup */
  setup_hardware();

  /* Reset state variables */
  last_detection[0] = 0;
  last_detection[1] = 0;

#if (STARTUP_EFFECT == 1)
  /*Do startup effect */
  startup_effect();
#endif
  
}


/* Configure the hardware */
void setup_hardware()
{

  /* In-built LED */
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED_BUILTIN, HIGH); //This means led OFF

 /* Initialize I2C bus */
  Wire.begin();
  lightMeter.begin();

  /* Initialize LED controller */
  FastLED.addLeds<WS2812, LED_PIN, COLOR_MODE>(leds, NUM_LEDS);

#if (DEBUG_ENABLED == 1)
  Serial.println("Hardware setup completed");
#endif
}

#if (STARTUP_EFFECT == 1)
void startup_effect()
{
  for(uint8_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(255,0,0);
    FastLED.show();
    delay(50);
  }

  for(uint8_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0,255,0);
    FastLED.show();
    delay(50);
  }
  
  for(uint8_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0,0,255);
    FastLED.show();
    delay(50);
  }

  for(uint8_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0,0,0);
    FastLED.show();
    delay(50);
  }
}
#endif

/* Get the amount of light */
float get_light_amount()
{
  light_amount = lightMeter.readLightLevel();

#if (DEBUG_ENABLED == 1)
  Serial.print("Light amount: ");
  Serial.println(light_amount);
#endif

  return light_amount;
}

void presence_sensor_1_isr()
{
#if (DEBUG_ENABLED == 1)
  Serial.println("Interrupt 1 detected");
#endif
  last_detection[0] = millis();
  
}

void presence_sensor_2_isr()
{
#if (DEBUG_ENABLED == 1)
  Serial.println("Interrupt 2 detected");
#endif
  last_detection[1] = millis();
  
}

void set_lights(bool state)
{

  if(state != light_state)
  {
    light_state = state;

    if(state)
    {
#if (DEBUG_ENABLED == 1)
      Serial.println("LIGHTS ON");
#endif
      for(uint8_t i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB(LIGHT_VALUE_R,LIGHT_VALUE_G,LIGHT_VALUE_B);
        FastLED.show();
        delay(100);
      }      
    }
    else
    {
#if (DEBUG_ENABLED == 1)
      Serial.println("LIGHTS OFF");
#endif
      for(uint8_t i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB(0,0,0);
        FastLED.show();
        delay(100);
      }    
    }  
    FastLED.show();      
  }   
}

void apply_logic()
{

  /* Evaluate only if the activation time has expired */
  unsigned long current_time = millis();  
  if(abs(current_time - last_activation) > ACTIVE_TIME)
  {    
    /* Compute the time difference between detections */  
    noInterrupts();
    unsigned long detection_difference_0 = abs(last_detection[0] - last_detection[1]);
    /* Compute the time since the last detection */
    unsigned long time_since_detection_0 = current_time - last_detection[0];
    unsigned long time_since_detection_1 = current_time - last_detection[1];   
    interrupts(); 

    /* If the light is already ON */
    if(light_state)
    {
      /* Decide to continue ON */
#if (KEEP_ON_CONDITION == 0)
      if( (time_since_detection_0 < DETECTION_INTERVAL) || (time_since_detection_1 < DETECTION_INTERVAL) )
#else
      if( (time_since_detection_0 < DETECTION_INTERVAL) && (time_since_detection_1 < DETECTION_INTERVAL) )
#endif
      {        
        last_activation = current_time;        
      }
      /* Decide to switch OFF */
      else
      {
#if (DEBUG_ENABLED == 1)
        Serial.println("Large difference between detections. Switching off");
        Serial.println(time_since_detection_0);
        Serial.println(time_since_detection_1);
#endif
        set_lights(false);
      } 
    }
    /* If the light is OFF */
    else
    {
      /* Get light amount */
      float light = get_light_amount();  
      
      /* Decide to switch ON */
      if( (time_since_detection_0 < DETECTION_INTERVAL) && (time_since_detection_1 < DETECTION_INTERVAL) && (light < LIGHT_THRESHOLD) )
      {
        last_activation = current_time;  
        set_lights(true);      
      }
    }       
  }
}

#if (DEBUG_ENABLED == 1)
void print_status()
{
  unsigned long current_time = millis(); 
  
  if( ((current_time - last_detection[0]) < DETECTION_INTERVAL) && ((current_time - last_detection[1]) < DETECTION_INTERVAL ) )
  {
    Serial.print("###################   ###################");
  }

  else if( (current_time - last_detection[0]) < DETECTION_INTERVAL )
  {
    Serial.print("###################");
  }
  else if( (current_time - last_detection[1]) < DETECTION_INTERVAL )
  {
    Serial.print("                      ###################");
  }

  Serial.println("");
  
}
#endif


void loop() {

  unsigned long current_time = millis();  
  
  if(current_time - last_iteration > LOOP_TIME)
  {
    
    last_iteration = current_time;    
    apply_logic();
#if (DEBUG_ENABLED == 1)
    print_status();
#endif
  }

  delay(LOOP_TIME);  

}
