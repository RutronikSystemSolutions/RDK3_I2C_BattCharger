/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RDK3_I2C_BattCharger
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2022-08-22
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Authors: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "dio59020.h"
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

void handle_error(void);

int main(void)
{

	dio_monitor_reg_status_t dio_m_status;
	charge_stat_t charge_stat = CHRG_FAULT;
	chrg_fault_t chrg_fault = CHRG_NORMAL;
	cy_rslt_t result;


    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
    	handle_error();
    }

    /*Enable debug output via KitProg UART*/
    result = cy_retarget_io_init( KITPROG_TX, KITPROG_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("\x1b[2J\x1b[;H");

    /*Initialize LEDs*/
    result = cyhal_gpio_init( LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    result = cyhal_gpio_init( LED3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    /*Enable interrupts*/
    __enable_irq();


    /*Inititialize the I2C for the DIO59020 Charger*/
    result = dio_i2c_init();
    if (result != CY_RSLT_SUCCESS)
    {
    	printf("I2C peripheral initialization failure.\r\n");
    	handle_error();
    }
    printf("I2C peripheral ready.\r\n");

    /* DIO59020 Charger Setup*/
    if(dio_online())
    {
    	printf("The DIO59020 responded with correct ID.\r\n");

    	/*Disable the charger*/
		dio_charger_disable();

		/*Set the input current limit*/
		dio_set_current_lim(LIMIT_INF);

		/*Set the battery charge current*/
		dio_set_batt_current(CURR_VREF_101_8);

		/*Set the battery charge termination current*/
		dio_set_batt_term_curr(CURR_VREF_12_5);

		/*Configure OTG pin*/
		dio_otg_pin_config(true, false);

		/*Enable the charger*/
		dio_charger_enable();
    }
    else
    {
    	printf("The DIO59020 does not respond with correct ID.\r\n");
    	handle_error();
    }


    /*Report the status and some properties of the DIO59020 charger*/
   for (;;)
    {
	   printf("\x1b[2J\x1b[;H");
	   printf(" \r\n");

	   /*Check and report the status of the DIO59020*/
	   result = dio_get_status(&charge_stat);
	   if (result == CY_RSLT_SUCCESS)
	   {
		   printf("Charger Status: ");
		   if(charge_stat == CHRG_READY)
		   {printf("Charger is in ready state.\r\n");}
		   if(charge_stat == CHRG_IN_PROGRESS)
		   {printf("Charging is in progress.\r\n");}
		   if(charge_stat == CHRG_DONE)
		   {printf("Charging done.\r\n");}
		   if(charge_stat == CHRG_FAULT)
		   {
			   printf("Charger fault: ");
			   dio_get_fault(&chrg_fault );
			   switch(chrg_fault)
			   {
			   	   case CHRG_NORMAL:
			   	   {
			   		   printf("Charger OK.\r\n");
			   		   break;
			   	   }
			   	   case CHRG_VBUS_OVP:
			   	   {
			   		   printf("Charger VBUS Over-voltage.\r\n");
			   		   break;
			   	   }
			   	   case CHRG_SLEEP_MODE:
			   	   {
			   		   printf("Charger is in sleep mode.\r\n");
			   		   break;
			   	   }
			   	   case CHRG_POOR_INPUT:
			   	   {
			   		   printf("Charger source input is too weak.\r\n");
			   		   break;
			   	   }
			   	   case CHRG_BATT_OVP:
			   	   {
			   		   printf("Charger battery over-volatge.\r\n");
			   		   break;
			   	   }
			   	   case CHRG_THERMAL:
			   	   {
			   		   printf("Charger has over-heated.\r\n");
			   		   break;
			   	   }
			   	   case CHRG_NO_BATT:
			   	   {
			   		   printf("No battery is detected.\r\n");
			   		   break;
			   	   }
			   	   default:
			   	   {
			   		   printf("Undefined.\r\n");
			   	   }
			   }
		   }
	   }

	   /*Monitor charging properties*/
	   result = (cy_rslt_t)dio_monitor_read(&dio_m_status);
	   if (result == CY_RSLT_SUCCESS)
	   {
		   printf("--------- DIO59020 Charger Information -------\r\n");

		   if(dio_m_status.CV_status)
		   {printf("Constant Voltage Charging.\r\n");}
		   else
		   {printf("Constant Current Charging.\r\n");}

		   if(dio_m_status.VBUS_VALID_status)
		   {printf("VBUS is Valid.\r\n");}
		   else
		   {printf("VBUS Not Valid.\r\n");}

		   if(dio_m_status.IBUS_status)
		   {printf("Charge Current Not Limited by IBUS.\r\n");}
		   else
		   {printf("IBUS Limiting Charging Current.\r\n");}

		   if(dio_m_status.CHG_status)
		   {printf("Charging Current Not Controlled by ICHARGE.\r\n");}
		   else
		   {printf("Charging Current Controlled by ICHARGE Control Loop.\r\n");}

		   if(dio_m_status.T_100_status)
		   {printf("TJ > 100°C .\r\n");}
		   else
		   {printf("TJ < 100°C.\r\n");}

		   if(dio_m_status.LINCHG_status)
		   {printf("30mA linear charger Enabled.\r\n");}
		   else
		   {printf("30mA linear charger Not Enabled.\r\n");}

		   if(dio_m_status.VBAT_CMP_status)
		   {printf("VBAT > VSHORT.\r\n");}
		   else
		   {printf("VBAT < VSHORT.\r\n");}

		   if(dio_m_status.ITERM_status)
		   {printf("VCSIN - VBAT > VITERM.\r\n");}
		   else
		   {printf("VCSIN - VBAT < VITERM.\r\n");}

		   printf("----------------------------------------------\r\n");
	   }

	   cyhal_gpio_toggle(LED2);
	   CyDelay(1000);
    }
}

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();
    cyhal_gpio_write(LED2, CYBSP_LED_STATE_OFF);
    cyhal_gpio_write(LED3, CYBSP_LED_STATE_ON);
    CY_ASSERT(0);
}
/* [] END OF FILE */

