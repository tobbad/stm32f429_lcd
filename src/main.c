//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "lcd_log.h"
#include "Timer.h"
#include "BlinkLed.h"

// ----------------------------------------------------------------------------
//
// STM32F4 led blink sample (trace via DEBUG).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// To demonstrate POSIX retargetting, reroute the STDOUT and STDERR to the
// trace device and display messages on both of them.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// On DEBUG, the uptime in seconds is also displayed on the trace device.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


/**
  * @brief  LCD configuration.
  * @param  None
  * @retval None
  */
static void LCD_Config(void)
{
    /* LCD Initialization */
    BSP_LCD_Init();

    /* LCD Layers Initialization */
    BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, (LCD_FRAME_BUFFER + BUFFER_OFFSET));

    /* Configure the transparency for foreground : Increase the transparency */
    BSP_LCD_SetTransparency(LCD_BACKGROUND_LAYER, 0);
    BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);

    /* LCD Log initialization */
    LCD_LOG_Init();

    LCD_LOG_SetHeader((uint8_t *)"LTDC Application");
    LCD_UsrLog("*** Application started. ***\n");
    LCD_LOG_SetFooter ((uint8_t *)"                   Baerospace GmbH");
}


int
main(int argc, char* argv[])
{
    // By customising __initialize_args() it is possible to pass arguments,
    // for example when running tests with semihosting you can pass various
    // options to the test.
    // trace_dump_args(argc, argv);

    /* Initialize the SDRAM */
    BSP_SDRAM_Init();

	/* Initialize LCD driver */
    LCD_Config();

    // Send a greeting to the trace device (skipped on Release).
    trace_puts("Hello ARM World!");

    // The standard output and the standard error should be forwarded to
    // the trace device. For this to work, a redirection in _write.c is
    // required.
    puts("Standard output message.");
    fprintf(stderr, "Standard error message.\n");

    // At this stage the system clock should have already been configured
    // at high speed.
    trace_printf("System clock: %u Hz\n", SystemCoreClock);

    timer_start();

    blink_led_init();


    uint32_t seconds = 00;
    uint32_t minutes = 00;
    uint32_t hours = 0;
    uint32_t days=0;

    // Infinite loop
    while (1)
    {
        blink_led_on();
        timer_sleep(seconds == 0 ? TIMER_FREQUENCY_HZ : BLINK_ON_TICKS);

        blink_led_off();
        timer_sleep(BLINK_OFF_TICKS);

        seconds = (seconds+1)%60;
        if ( seconds == 0)
        {
        	minutes = (minutes+1)%60;
        	if ( minutes == 0 )
        	{
        		hours = (hours+1)%24;
        		if (hours == 0)
        		{
        			days+=1;
        		}
        	}
        }
        // Count seconds on the trace device.
        LCD_UsrLog("%2d days %02u:%02u:%02u\n",days, hours, minutes, seconds);
    }
    // Infinite loop, never return.
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
