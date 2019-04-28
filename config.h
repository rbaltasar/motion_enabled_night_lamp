#if !defined CONFIG_H
#define CONFIG_H

/* Hardware configuration */
#define NUM_LEDS 14 //Number of leds
#define PRESENCE_SENSOR_1 3
#define PRESENCE_SENSOR_2 2
#define LED_PIN 7

/* Configuration parameters */
#define ACTIVE_TIME 30000 //Minimum amount of time that the lamp is ON
#define DETECTION_INTERVAL 10000 //Maximum time difference between motion events to switch ON
#define LIGHT_THRESHOLD 1 //Maximum light amount to switch lights ON
#define STARTUP_EFFECT 1 //[0]: No startup effect / [1]: Startup effect
#define LIGHT_EFFECT 2 //[0]: No effect / [1]: Led Array / [2]: Fade
#define KEEP_ON_CONDITION 1 //[0]: SINGLE_MOTION / [1]: DOUBLE_MOTION

/* Effects parameters */
#define DELAY_FADE 70
#define DELAY_ARRAY 100

/* Led lighting */
#define LIGHT_VALUE_R 150//200
#define LIGHT_VALUE_G 200//180
#define LIGHT_VALUE_B 200//100

/* Other parameters */
#define LOOP_TIME 1000 //Sleep delay in main loop (power saving)
#define COLOR_MODE GRB//Could be GRB

/* Debugging */
#define DEBUG_ENABLED 1

#endif
