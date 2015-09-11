/*******************************************************************************
* @Title: sched.c
*
* @Author: Phil Smith
*
* @Date: Thu, 10-Sep-15 01:23AM
*
* @Project: Multithreading Rasperry Pi
*
* @Purpose: This module creates the main scheduler for all required tasks.
*
*
*******************************************************************************/
#include<wiringPi.h>
#include<signal.h>
#include<time.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>


#define LED1 18
#define LED2 23
#define BUT1 22

/* To use RATE60HZ, FRAMEPERSEC = 60 */
#define RATE60HZ    16666666L
/* To use RATE120HZ, FRAMEPERSEC = 120 */
#define RATE120HZ   8333333L
/* To use RATE2MHZ, FRAMEPERSEC = 2000 */
#define RATE2MHZ    500000L

/* These two lines actually set the frequency for execution */
#define RATECMD     RATE2MHZ
#define FRAMEPERSEC 2000

/* Function prototypes */
void init();
void run();
void led_1_ctrl();
void led_2_ctrl();
void button_ctrl();

/* The 'tick' is caught everytime the Linux timer event expires. It can
 * only be as accurate as the clocksource allows. The default on the Pi 
 * is the STC.
 * (cat /sys/devices/system/clocksource/clocksource0/current_clocksource
 *   or /sys/devices/system/clocksource/clocksource0/available_clocksource)
 * To timers that Linux is using (current_clocksource) or can use (available_clocksource)
 */
short tick=0;


/* The 'frame' is which fraction of the second is currently being executed.
 * For example a 60Hz timebase yields 60 frames in a second and a 2000Hz
 * timebase yields 2000 frames. You can adjust the frequency of your
 * tasks by changing which frames they run on. For another example,
 * a task set to run only on frame 0 with a 60Hz timebase will run 1 time
 * every second or 1Hz. A task set to run on every even frame will run
 * 30 times every second or 30Hz. No matter which frame the tasks run on, they
 * should be evenly spaced throughout the second or else you will jitter.
 */
short frame=0;


/**
 * main():
 * Standard main entry point. Calls the init() routine and runs the
 * real-time loop.
 **/
int main(){

    /* Initialize the system */
    printf("Initializing executive loop.\n"); 
    init();

    /* Run the real-time loop */
    printf("Starting executive loop.\n"); 
    run();
}

/**
 * init():
 * Setup the pins for wiringPi.
 **/
void init(){
    /* Initialize the wiringPi library */
    wiringPiSetupGpio();
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(BUT1, INPUT);
}

/**
 * catch_tick():
 * Catches the timer expiration and sets the tick count. This function
 * is the ISR for the timer event.
 **/
void catch_tick(){
    tick++;
}

/**
 * run():
 * Runs the real-time loop and calls the synchronous tasks.
 **/
void run(){

    timer_t timer;
    struct itimerspec timer_info;
    struct sigevent sig;
    struct sigaction sa;

    struct timespec start_time;
    struct timespec stop_time;

    /******************************
     * Setup the real-time timer. *
     ******************************/
    /* SIGEV_SIGNAL tells the timer to kick off a signal when it
     * expires. In this case it will send SIGALRM. */
    sig.sigev_notify = SIGEV_SIGNAL;
    sig.sigev_signo = SIGALRM;

    /* Register a signal handler since the timer expiration will
     * will kick off a signal. In this case set 'catch_tick()' as
     * the ISR and SIGALRM as the signal to catch. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = catch_tick;
    sigemptyset( &sa.sa_mask );
    sigaction( SIGALRM, &sa, NULL);

    /* Create the timer */
    timer_create( CLOCK_MONOTONIC, &sig, &timer);
    
    /* Set the initial timeout then the reset interval */
    timer_info.it_value.tv_sec = 0;
    timer_info.it_value.tv_nsec = RATECMD;
    
    timer_info.it_interval.tv_sec = 0;
    timer_info.it_interval.tv_nsec = RATECMD;

    /* Arm the timer */
    if( timer_settime( timer, 0, &timer_info, NULL )){
        printf("Could not set timer!\n");
        sleep(1);
        exit(EXIT_FAILURE);
    }


    /* Start the main loop. In reality this can use a nicer exit
     * condition - while( some_flag != false ) */
    while(true){

        /* Catch timer ticks */
        if( tick ){
            /* Anything within this tick area will be called at
             * the base frequency. */
            
            /* The executive should always keep track of the current
             * frame. */
            frame++;
            
            if( frame >= FRAMEPERSEC ){
                /* This function will run at 1Hz no matter the time base.
                 * It is only allowed to run on the single frame that resets
                 * the frame count. */
                led_1_ctrl();

                /* The frame count should rollover periodically so
                 * that schedules can be built using frame number. */
                frame = 0;
            }

            /* This function will run at RATECMD. It gets called every
             * time a tick is received which is, by definition, the commanded
             * rate. */
            button_ctrl();

            /* Always reset the tick back to zero to stop this
             * loop from re-running early. */
            tick = 0;

        }
        /* Pause until the next clock tick comes in */
        pause();
    }
}

/**
 * Toggles the first LED.
 **/
void led_1_ctrl(){

#ifdef __DEBUG__
    printf("Thread 1!\n");
#endif
    int state = digitalRead(LED1);
    if( state ){
        digitalWrite(LED1, LOW);
    }else{
        digitalWrite(LED1, HIGH);
    }
}

/**
 * Toggles the second LED.
 **/
void led_2_ctrl(){

#ifdef __DEBUG__
    printf("Thread 2!\n");
#endif
    int state = digitalRead(LED2);
    if( state ){
        digitalWrite(LED2, LOW);
    }else{
        digitalWrite(LED2, HIGH);
    }
}

/**
 * Toggles the button.
 **/
void button_ctrl(){
    if( !digitalRead(BUT1) ){
#ifdef __DEBUG__
        printf("Button Pressed!\n");
#endif
        digitalWrite(LED2, HIGH);
    }else{
        digitalWrite(LED2, LOW);
    }
}
