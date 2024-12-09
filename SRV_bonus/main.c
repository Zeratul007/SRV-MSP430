/**
 * @file    main.c
 * @author  Haris Turkmanovic(haris@etf.rs)
 * @date    2021
 * @brief   SRV Zadatak 18
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* Hardware includes. */
#include "msp430.h"

/* User's includes */
#include "../common/ETF5529_HAL/hal_ETF_5529.h"





/** "Display task" priority */
#define mainDISPLAY_TASK_PRIO           ( 1 )
/** "ADC task" priority */
#define mainADC_TASK_PRIO               ( 3 )
/** "Diode task" priority */
#define mainDIODE_TASK_PRIO           ( 2 )
/** "ADC_GEN_ISR task" priority */
#define mainADC_ISR_GEN_PRIO               ( 4 )
/** "Button task" priority */
#define mainButton_TASK_PRIO               ( 4 )


/* Display queue parameters value*/
/* Queue with length 1 is mailbox*/
#define mainDISPLAY_QUEUE_LENGTH            1

static void prvSetupHardware( void );

/*Used to signal "Button press" event*/
xSemaphoreHandle    xEvent_Button;
/*This semaphore will be used to protect shared resource*/
xSemaphoreHandle    xGuard_Which_Pot;
/*This semaphore will be used to protect shared resource*/
xSemaphoreHandle    xGuard_Value;
/*Used to signal "Button press" event*/
xSemaphoreHandle    xDiode_Change;

uint8_t value;
uint8_t which_pot = 0;

/* This queue will be used to send data to display task*/
xQueueHandle        xDisplayQueue;
/**
 * @brief "Display Task" Function
 *
 * This task read data from xDisplayQueue but reading is not blocking.
 * xDisplayQueue is used to send data which will be printed on 7Seg
 * display. After data is received it is decomposed on high and low digit.
 */
static void prvDisplayTaskFunction( void *pvParameters )
{
    /* New received 8bit data */
    /* High and Low number digit*/
    uint8_t         digitLow, digitHigh;
    uint16_t         value_copy;
    for ( ;; )
    {
        /* Take mutex */
        xSemaphoreTake(xGuard_Value, portMAX_DELAY);

        value_copy = value;

        /* Give mutex */
        xSemaphoreGive(xGuard_Value);

        /* Extract high digit*/
        digitHigh = value_copy/10;
        /* Extract low digit*/
        digitLow = value_copy - digitHigh*10;

        HAL_7SEG_DISPLAY_1_ON;
        HAL_7SEG_DISPLAY_2_OFF;
        vHAL7SEGWriteDigit(digitLow);
        vTaskDelay(5);
        HAL_7SEG_DISPLAY_2_ON;
        HAL_7SEG_DISPLAY_1_OFF;
        vHAL7SEGWriteDigit(digitHigh);
        vTaskDelay(5);


    }
}

/**
 * @brief "ADC Task" Function
 *
 * This task periodically, with 200 sys-tick period, trigger ADC
 */
static void prvADC_Gen_ISR_TaskFunction( void *pvParameters )
{

    for ( ;; )
    {
       /*Trigger ADC Conversion*/
       ADC12CTL0 |= ADC12SC;

       vTaskDelay(200);
    }
}

static void prvButtonTaskFunction( void *pvParameters )
{
    uint16_t i;
    /*Initial button states are 1 because of pull-up configuration*/
    uint8_t     currentButtonState = 1;
    for ( ;; )
    {
        xSemaphoreTake(xEvent_Button,portMAX_DELAY);
        /*wait for a little to check that button is still pressed*/
        for(i = 0; i < 1000; i++);
        /*Check is SW3 still pressed*/
        currentButtonState = ((P1IN & 0x10) >> 4);
        if(currentButtonState == 0){
            /* If button is still pressed write that info to global variable*/
            /* Global variable is shared resource. To access it, first try to
             * take mutex */
            xSemaphoreTake(xGuard_Value, portMAX_DELAY);
            /* Write which button press event is detect */
            UCA1TXBUF = value;
            while(!(UCA1IFG&UCTXIFG));
            /* Give mutex */
            xSemaphoreGive(xGuard_Value);
            /* Signal to "Diode task" to change state */
            continue;

        }
        /*Check is SW4 still pressed*/
        currentButtonState = ((P1IN & 0x20) >> 5);
        if(currentButtonState == 0){
            /* If button is still pressed write that info to global variable*/
            /* Global variable is shared resource. To access it, first try to
             * take mutex */
            xSemaphoreTake(xGuard_Which_Pot, portMAX_DELAY);
            /* Write which button press event is detect */

            which_pot = 1 - which_pot;

            /* Give mutex */
            xSemaphoreGive(xGuard_Which_Pot);

            /* Signal to "Diode task" to change state */
            xSemaphoreGive(xDiode_Change);

            continue;

        }
    }
}

static void prvADCTaskFunction( void *pvParameters )
{
    uint8_t adc = 0;
    int channel = 0;
    for ( ;; )
    {

       xSemaphoreTake(xGuard_Which_Pot, portMAX_DELAY);

       channel = which_pot;

       xSemaphoreGive(xGuard_Which_Pot);

       xQueueReceive(xDisplayQueue, &adc, portMAX_DELAY);

       xSemaphoreTake(xGuard_Value, portMAX_DELAY);

       value = adc;

       xSemaphoreGive(xGuard_Value);

       ADC12CTL0      &=~ ADC12ENC;

       switch(channel){
       case 0:

           ADC12MCTL0     &=~ ADC12INCH_1;
           ADC12MCTL0     |= ADC12INCH_0;
           break;
       case 1:
           ADC12MCTL0     &=~ ADC12INCH_0;
           ADC12MCTL0     |= ADC12INCH_1;
           break;
       }
       ADC12CTL0      |= ADC12ENC;
    }
}

static void prvDiodeControlTaskFunction( void *pvParameters )
{
    uint8_t         diodeToTurnOn   =   LED4;
    uint8_t         diodeToTurnOff  =   LED3;
    halSET_LED(diodeToTurnOn);
    halCLR_LED(diodeToTurnOff);
    for ( ;; )
    {

        xSemaphoreTake(xDiode_Change, portMAX_DELAY);
        diodeToTurnOn = diodeToTurnOn == LED3? LED4 : LED3;
        diodeToTurnOff = diodeToTurnOn == LED3? LED4 : LED3;
        halSET_LED(diodeToTurnOn);
        halCLR_LED(diodeToTurnOff);
    }
}
/**
 * @brief main function
 */
void main( void )
{
    /* Configure peripherals */
    prvSetupHardware();

    /* Create tasks */
    xTaskCreate( prvDisplayTaskFunction,
                 "Display Task",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 mainDISPLAY_TASK_PRIO,
                 NULL
               );
    xTaskCreate( prvADC_Gen_ISR_TaskFunction,
                 "ADC_Gen_ISR Task",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 mainADC_ISR_GEN_PRIO,
                 NULL
               );
    xTaskCreate( prvButtonTaskFunction,
                     "Button Task",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     mainButton_TASK_PRIO,
                     NULL
                   );
    xTaskCreate( prvDiodeControlTaskFunction,
                     "Diode Task",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     mainDIODE_TASK_PRIO,
                     NULL
                   );
    xTaskCreate( prvADCTaskFunction,
                        "ADC Task",
                        configMINIMAL_STACK_SIZE,
                        NULL,
                        mainADC_TASK_PRIO,
                        NULL
                      );
    /* Create FreeRTOS objects  */
    xDisplayQueue       =   xQueueCreate(mainDISPLAY_QUEUE_LENGTH,sizeof(uint8_t));

    xEvent_Button           =   xSemaphoreCreateBinary();
    xGuard_Which_Pot           =   xSemaphoreCreateMutex();
    xGuard_Value           =   xSemaphoreCreateMutex();
    xDiode_Change           =   xSemaphoreCreateBinary();



    /* Start the scheduler. */
    vTaskStartScheduler();

    /* If all is well then this line will never be reached.  If it is reached
    then it is likely that there was insufficient (FreeRTOS) heap memory space
    to create the idle task.  This may have been trapped by the malloc() failed
    hook function, if one is configured. */	
    for( ;; );
}

/**
 * @brief Configure hardware upon boot
 */
static void prvSetupHardware( void )
{
    taskDISABLE_INTERRUPTS();

    /* Disable the watchdog. */
    WDTCTL = WDTPW + WDTHOLD;

    hal430SetSystemClock( configCPU_CLOCK_HZ, configLFXT_CLOCK_HZ );

    /* - Init buttons - */
    /*Set direction to input*/
    P1DIR &= ~0x30;
    /*Enable pull-up resistor*/
    P1REN |= 0x30;
    P1OUT |= 0x30;
    /*Enable interrupt for pins connected to SW3 and SW4*/
    P1IE  |= 0x30;
    P1IFG &=~0x30;
    /*Interrupt is generated during high to low transition*/
    P1IES |= 0x30;

    /*Initialize ADC */
    ADC12CTL0      = ADC12SHT02 + ADC12ON;       // Sampling time, ADC12 on
    ADC12CTL1      = ADC12SHP;                   // Use sampling timer
    ADC12IE        = 0x01;                       // Enable interrupt
    ADC12MCTL0     |= ADC12INCH_1;
    ADC12CTL0      |= ADC12ENC;
    P6SEL          |= 0x03;                      // P6.0 and P6.1 ADC option select

    /* Initialize UART */

    P4SEL       |= BIT4+BIT5;                    // P4.4,5 = USCI_AA TXD/RXD
    UCA1CTL1    |= UCSWRST;                      // **Put state machine in reset**
    UCA1CTL1    |= UCSSEL_2;                     // SMCLK
    UCA1BRW      = 1041;                         // 1MHz 115200
    UCA1MCTL    |= UCBRS_6 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA1CTL1    &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA1IE      |= UCRXIE;                       // Enable USCI_A1 RX interrupt

    /* initialize LEDs */
    vHALInitLED();
    /* initialize display*/
    vHAL7SEGInit();
    /*enable global interrupts*/
    taskENABLE_INTERRUPTS();
}
void __attribute__ ( ( interrupt( ADC12_VECTOR  ) ) ) vADC12ISR( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint16_t    temp;
    switch(__even_in_range(ADC12IV,34))
    {
        case  0: break;                           // Vector  0:  No interrupt
        case  2: break;                           // Vector  2:  ADC overflow
        case  4: break;                           // Vector  4:  ADC timing overflow
        case  6:                                  // Vector  6:  ADC12IFG0
            /* Scaling ADC value to fit on two digits representation*/
            temp    = ADC12MEM0>>6;
            xQueueSendToBackFromISR(xDisplayQueue, &temp, &xHigherPriorityTaskWoken);
            break;
        case  8:                                  // Vector  8:  ADC12IFG1
            break;
        case 10: break;                           // Vector 10:  ADC12IFG2
        case 12: break;                           // Vector 12:  ADC12IFG3
        case 14: break;                           // Vector 14:  ADC12IFG4
        case 16: break;                           // Vector 16:  ADC12IFG5
        case 18: break;                           // Vector 18:  ADC12IFG6
        case 20: break;                           // Vector 20:  ADC12IFG7
        case 22: break;                           // Vector 22:  ADC12IFG8
        case 24: break;                           // Vector 24:  ADC12IFG9
        case 26: break;                           // Vector 26:  ADC12IFG10
        case 28: break;                           // Vector 28:  ADC12IFG11
        case 30: break;                           // Vector 30:  ADC12IFG12
        case 32: break;                           // Vector 32:  ADC12IFG13
        case 34: break;                           // Vector 34:  ADC12IFG14
        default: break;
    }
    /* trigger scheduler if higher priority task is woken */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void __attribute__ ( ( interrupt( PORT1_VECTOR  ) ) ) vPORT1ISR( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* Give semaphore if button SW3 or button SW4 is pressed*/
    /* Note: This check is not truly necessary but it is good to
     * have it*/
    if(((P1IFG & 0x10) == 0x10) || ((P1IFG & 0x20) == 0x20)){
        xSemaphoreGiveFromISR(xEvent_Button, &xHigherPriorityTaskWoken);
    }
    /*Clear IFG register on exit. Read more about it in official MSP430F5529 documentation*/
    P1IFG &= ~ (((P1IFG & 0x10) == 0x10) ? 0x10 : 0x20);
    /* trigger scheduler if higher priority task is woken */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
