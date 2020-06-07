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
//#if (DEBUG_ENABLED == 1)
  Serial.begin(115200);
//#endif

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
//#if (DEBUG_ENABLED == 1)
  Serial.println("Interrupt 1 detected");
//#endif
  last_detection[0] = millis();
  
}

void presence_sensor_2_isr()
{
//#if (DEBUG_ENABLED == 1)
  Serial.println("Interrupt 2 detected");
//#endif
  last_detection[1] = millis();
  
}

void led_array_effect(bool state)
{
  if(state)
  {  
    for(uint8_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(LIGHT_VALUE_R,LIGHT_VALUE_G,LIGHT_VALUE_B);
      FastLED.show();
      delay(DELAY_ARRAY);
    } 
  }
  else
  {
    for(uint8_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(0,0,0);
      FastLED.show();
      delay(DELAY_ARRAY);
    } 
  }
}

void fade_effect(bool state)
{

  bool exitCondition = false;

  if(state)
  {
    uint8_t step_r = 0;
    uint8_t step_g = 0;
    uint8_t step_b = 0;
    do
    { 
      for(uint8_t i = 0; i < NUM_LEDS; i++) 
      {
        leds[i] = CRGB(step_r,step_g,step_b);
      }

      ++step_r > LIGHT_VALUE_R ? step_r = LIGHT_VALUE_R : step_r = step_r;
      ++step_g > LIGHT_VALUE_G ? step_g = LIGHT_VALUE_G : step_g = step_g;
      ++step_b > LIGHT_VALUE_B ? step_b = LIGHT_VALUE_B : step_b = step_b;

      if( (step_r == LIGHT_VALUE_R) && (step_g == LIGHT_VALUE_G) && (step_b == LIGHT_VALUE_B) ) exitCondition = true;
      
      FastLED.show();
      delay(DELAY_FADE);
    } while(!exitCondition);
  }
  else
  {
    uint8_t step_r = LIGHT_VALUE_R;
    uint8_t step_g = LIGHT_VALUE_G;
    uint8_t step_b = LIGHT_VALUE_B;
    do
    { 
      for(uint8_t i = 0; i < NUM_LEDS; i++) 
      {
        leds[i] = CRGB(step_r,step_g,step_b);
      }

      --step_r <= 1 ? step_r = 1 : step_r = step_r;
      --step_g <= 1 ? step_g = 1 : step_g = step_g;
      --step_b <= 1 ? step_b = 1 : step_b = step_b;

      if( (step_r == 1) && (step_g == 1) && (step_b == 1) )
      {
        exitCondition = true;
        for(uint8_t i = 0; i < NUM_LEDS; i++) 
        {
          leds[i] = CRGB(0,0,0);
        }
      }
      
      FastLED.show();
      delay(DELAY_FADE);
    } while(!exitCondition); 
  }
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

#if (LIGHT_EFFECT == 1)
      led_array_effect(state);
#elif (LIGHT_EFFECT == 2)
      fade_effect(state);
#else
      for(uint8_t i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB(LIGHT_VALUE_R,LIGHT_VALUE_G,LIGHT_VALUE_B);        
      }
      FastLED.show();
#endif      
    }
    else
    {
#if (DEBUG_ENABLED == 1)
      Serial.println("LIGHTS OFF");
#endif

#if (LIGHT_EFFECT == 1)
      led_array_effect(state);
#elif (LIGHT_EFFECT == 2)
      fade_effect(state);
#else
      for(uint8_t i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB(0,0,0);        
      }
      FastLED.show();
#endif
    }      
  }   
}

void on_delay()
{
  Serial.println("Starting ON delay");
  
  int const delay_time = 1000;
  unsigned long num_delays = ACTIVE_TIME / delay_time; // 1000ms delay

  for(int i = 0; i < num_delays; i++){
    Serial.print("Time left: ");
    Serial.print((num_delays - i) * delay_time);
    Serial.println(" ms");
    delay(delay_time);
  }
}

void apply_logic()
{

    /* Compute the time difference between detections */  
    noInterrupts();
    unsigned long detection_difference_0 = abs(last_detection[0] - last_detection[1]);
    /* Compute the time since the last detection */
    unsigned long current_time = millis();
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
        on_delay();   
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
        set_lights(true);
        on_delay();
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
