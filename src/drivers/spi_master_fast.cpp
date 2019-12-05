 /* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "spi_master_fast.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"

static SPI_config_t spi_config_table[2];
static NRF_SPI_Type *spi_base[2] = {NRF_SPI0, NRF_SPI1};
static NRF_SPI_Type *SPI;

uint32_t* spi_master_init(SPI_module_number_t spi_num, SPI_config_t *spi_config)
{
    if(spi_num > 1)
    {
        return 0;
    }
    memcpy(&spi_config_table[spi_num], spi_config, sizeof(SPI_config_t));

    /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
    nrf_gpio_cfg_output(spi_config->pin_SCK);
    nrf_gpio_cfg_output(spi_config->pin_MOSI);
    nrf_gpio_cfg_input(spi_config->pin_MISO, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(spi_config->pin_CSN);

    /* Configure pins, frequency and mode */
    spi_base[spi_num]->PSELSCK  = spi_config->pin_SCK;
    spi_base[spi_num]->PSELMOSI = spi_config->pin_MOSI;
    spi_base[spi_num]->PSELMISO = spi_config->pin_MISO;
    nrf_gpio_pin_set(spi_config->pin_CSN); /* disable Set slave select (inactive high) */

    spi_base[spi_num]->FREQUENCY = (uint32_t)spi_config->frequency << 24;

    spi_base[spi_num]->CONFIG = spi_config->config.SPI_cfg;

    spi_base[spi_num]->EVENTS_READY = 0;
    /* Enable */
    spi_base[spi_num]->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    return (uint32_t *)spi_base[spi_num];
}

bool spi_master_tx_rx(SPI_module_number_t spi_num, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data)
{
    volatile uint32_t *SPI_DATA_READY;
    uint32_t tmp; 
    if(tx_data == 0 || rx_data == 0)
    {
        return false;
    }
    
    SPI = spi_base[spi_num];
    SPI_DATA_READY = &SPI->EVENTS_READY;
    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(spi_config_table[spi_num].pin_CSN);
    
    *SPI_DATA_READY = 0; 
    
    SPI->TXD = (uint32_t)*tx_data++;
    tmp = (uint32_t)*tx_data++;
    while(--transfer_size)
    {
        SPI->TXD =  tmp;
        tmp = (uint32_t)*tx_data++;
        
        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while (*SPI_DATA_READY == 0);

        /* clear the event to be ready to receive next messages */
        *SPI_DATA_READY = 0;
        
        *rx_data++ = SPI->RXD; 
    }
      
    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (*SPI_DATA_READY == 0);

    *rx_data = SPI->RXD;

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(spi_config_table[spi_num].pin_CSN);

    return true;
}

bool spi_master_tx(SPI_module_number_t spi_num, uint16_t transfer_size, const uint8_t *tx_data)
{
    volatile uint32_t dummyread;
    
    if(tx_data == 0)
    {
        return false;
    }
    
    SPI = spi_base[spi_num];
    
    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(spi_config_table[spi_num].pin_CSN);
    
    SPI->EVENTS_READY = 0; 
    
    SPI->TXD = (uint32_t)*tx_data++;
    
    while(--transfer_size)
    {
        SPI->TXD =  (uint32_t)*tx_data++;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while (SPI->EVENTS_READY == 0);
        
        /* clear the event to be ready to receive next messages */
        SPI->EVENTS_READY = 0;
        
        dummyread = SPI->RXD;
    }
    
    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (SPI->EVENTS_READY == 0);

    dummyread = SPI->RXD;

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(spi_config_table[spi_num].pin_CSN);
    
    return true;
}

bool spi_master_rx(SPI_module_number_t spi_num, uint16_t transfer_size, uint8_t *rx_data)
{
    if(rx_data == 0)
    {
        return false;
    }
    
    SPI = spi_base[spi_num];
    
    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(spi_config_table[spi_num].pin_CSN);

    SPI->EVENTS_READY = 0; 
    
    SPI->TXD = 0;
    
    while(--transfer_size)
    {
        SPI->TXD = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while (SPI->EVENTS_READY == 0);
        
        /* clear the event to be ready to receive next messages */
        SPI->EVENTS_READY = 0;
        
        *rx_data++ = SPI->RXD;
    }
    
    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (SPI->EVENTS_READY == 0);

    *rx_data = SPI->RXD;

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(spi_config_table[spi_num].pin_CSN);

    return true;
}
