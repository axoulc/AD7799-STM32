/**
  ******************************************************************************
  * @file    AD7799.c
  * @brief   This file provides code to use ADC Analog AD7799
  * @author	 Axel Chabot
  ******************************************************************************
  */

#include "AD7799.h"
#include <math.h>

/**
 * Init function for ADC AD7799
 * @param ad7799
 * @return HAL_OK or HAL_ERROR when something wrong
 */
HAL_StatusTypeDef AD7799_Init(AD7799 *ad7799) {
	if ((AD7799_GetRegisterValue(ad7799, AD7799_REG_ID, 1) & 0x0F) != AD7799_ID) {
		return HAL_ERROR;
	} else {
		return HAL_OK;
	}
}

/**
 * Reset Function. Clear all configuration
 * @param ad7799
 */
void AD7799_Reset(AD7799 *ad7799) {
	uint8_t dataToSend[4] = { 0xff, 0xff, 0xff, 0xff };
	HAL_GPIO_WritePin(ad7799->CSPort, ad7799->CSPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(ad7799->adcspi, dataToSend, 4, AD7799_TIMEOUT_SPI);
	HAL_GPIO_WritePin(ad7799->CSPort, ad7799->CSPin, GPIO_PIN_SET);

	ad7799->mode = AD7799_MODE_CONT;
	ad7799->gain = AD7799_GAIN_128;
	ad7799->channel = AD7799_CH_AIN1P_AIN1M;
	ad7799->polarity = AD7799_BIPOLAR;
	ad7799->rate = AD7799_RATE_16_7HZ_65DB;
}

/**
 * Get register value from address
 * @param ad7799
 * @param regAddress
 * @param size
 * @return register value in 4 bytes
 */
uint32_t AD7799_GetRegisterValue(AD7799 *ad7799, uint8_t regAddress,
		uint8_t size) {
	uint8_t data[4] = { 0x00, 0x00, 0x00, 0x00 };
	uint32_t receivedData = 0x00;
	data[0] = AD7799_COMM_READ | AD7799_COMM_ADDR(regAddress);
	HAL_GPIO_WritePin(ad7799->CSPort, ad7799->CSPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(ad7799->adcspi, data, 1, AD7799_TIMEOUT_SPI);
	HAL_SPI_Receive(ad7799->adcspi, data, size, AD7799_TIMEOUT_SPI);
	HAL_GPIO_WritePin(ad7799->CSPort, ad7799->CSPin, GPIO_PIN_SET);
	if (size == 1) {
		receivedData += (data[0] << 0);
	}
	if (size == 2) {
		receivedData += (data[0] << 8);
		receivedData += (data[1] << 0);
	}
	if (size == 3) {
		receivedData += (data[0] << 16);
		receivedData += (data[1] << 8);
		receivedData += (data[2] << 0);
	}
	return receivedData;
}

/**
 * Set value(s) to address register
 * @param ad7799
 * @param regAddress
 * @param regValue
 * @param size
 */
void AD7799_SetRegisterValue(AD7799 *ad7799, uint8_t regAddress,
		uint32_t regValue, uint8_t size) {
	uint8_t data[4] = { 0x00, 0x00, 0x00, 0x00 };
	data[0] = AD7799_COMM_WRITE | AD7799_COMM_ADDR(regAddress);
	if (size == 1) {
		data[1] = (uint8_t) regValue;
	}
	if (size == 2) {
		data[2] = (uint8_t) ((regValue & 0x0000FF) >> 0);
		data[1] = (uint8_t) ((regValue & 0x00FF00) >> 8);
	}
	if (size == 3) {
		data[3] = (uint8_t) ((regValue & 0x0000FF) >> 0);
		data[2] = (uint8_t) ((regValue & 0x00FF00) >> 8);
		data[1] = (uint8_t) ((regValue & 0xFF0000) >> 16);
	}
	HAL_GPIO_WritePin(ad7799->CSPort, ad7799->CSPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(ad7799->adcspi, data, 1 + size, AD7799_TIMEOUT_SPI);
	HAL_GPIO_WritePin(ad7799->CSPort, ad7799->CSPin, GPIO_PIN_SET);

}

/**
 * Return 1 when conversion is done
 * @param ad7799
 * @return
 */
uint8_t AD7799_Ready(AD7799 *ad7799) {
	uint8_t rdy = 0;
	rdy = (AD7799_GetRegisterValue(ad7799, AD7799_REG_STAT, 1) & 0x80);
	return (!rdy);
}

/**
 * Set the functioning mode
 * @param ad7799
 * @param mode
 */
void AD7799_SetMode(AD7799 *ad7799, AD7799_Mode mode) {
	uint32_t command;
	command = AD7799_GetRegisterValue(ad7799, AD7799_REG_MODE, 2);
	command &= ~AD7799_MODE_SEL(0xFF);
	command |= AD7799_MODE_SEL((uint32_t ) mode);
	AD7799_SetRegisterValue(ad7799, AD7799_REG_MODE, command, 2);
	ad7799->mode = mode;
}

/**
 * Set the sample rate
 * @param ad7799
 * @param rate
 */
void AD7799_SetRate(AD7799 *ad7799, AD7799_Rate rate) {
	uint32_t command;
	command = AD7799_GetRegisterValue(ad7799, AD7799_REG_MODE, 2);
	command &= ~AD7799_MODE_RATE(0xFF);
	command |= AD7799_MODE_RATE((uint32_t ) rate);
	AD7799_SetRegisterValue(ad7799, AD7799_REG_MODE, command, 2);
	ad7799->rate = rate;
}

/**
 * Set the differential channel
 * @param ad7799
 * @param channel
 */
void AD7799_SetChannel(AD7799 *ad7799, AD7799_Channel channel) {
	uint32_t command;
	command = AD7799_GetRegisterValue(ad7799, AD7799_REG_CONF, 2);
	command &= ~AD7799_CONF_CHAN(0xFF);
	command |= AD7799_CONF_CHAN((uint32_t ) channel);
	AD7799_SetRegisterValue(ad7799, AD7799_REG_CONF, command, 2);
	ad7799->channel = channel;
}

/**
 * Set PGA Gain
 * @param ad7799
 * @param gain
 */
void AD7799_SetGain(AD7799 *ad7799, AD7799_Gain gain) {
	uint32_t command;
	command = AD7799_GetRegisterValue(ad7799, AD7799_REG_CONF, 2);
	command &= ~AD7799_CONF_GAIN(0xFF);
	command |= AD7799_CONF_GAIN((uint32_t ) gain);
	AD7799_SetRegisterValue(ad7799, AD7799_REG_CONF, command, 2);
	ad7799->gain = gain;
}

/**
 * Set reference detection
 * @param ad7799
 * @param state
 */
void AD7799_SetReference(AD7799 *ad7799, uint8_t state) {
	uint32_t command = 0;
	command = AD7799_GetRegisterValue(ad7799, AD7799_REG_CONF, 2);
	command &= ~AD7799_CONF_REFDET(1);
	command |= AD7799_CONF_REFDET(state);
	AD7799_SetRegisterValue(ad7799, AD7799_REG_CONF, command, 2);
}

/**
 * Set unipolar or bipolar conversion
 * @param ad7799
 * @param polarity
 */
void AD7799_SetPolarity(AD7799 *ad7799, AD7799_Polarity polarity) {
	uint32_t command = 0;
	command = AD7799_GetRegisterValue(ad7799, AD7799_REG_CONF, 2);
	command &= ~AD7799_CONF_POLAR(1);
	command |= AD7799_CONF_POLAR((uint32_t ) polarity);
	AD7799_SetRegisterValue(ad7799, AD7799_REG_CONF, command, 2);
	ad7799->polarity = polarity;
}

/**
 * Perform single conversion
 * @param ad7799
 * @return HAL_OK or HAL_TIMEOUT when conversion take too long
 */
HAL_StatusTypeDef AD7799_SingleConversion(AD7799 *ad7799) {
	AD7799_SetMode(ad7799, AD7799_MODE_SINGLE);

	uint32_t startTime = HAL_GetTick();
	while (!AD7799_Ready(ad7799)) {
		if (HAL_GetTick() - startTime
				> (uint32_t) 1.5 * settle_time_ms[(uint8_t) ad7799->rate]) {
			return HAL_TIMEOUT;
		}
	}

	ad7799->rawConversion = AD7799_GetRegisterValue(ad7799, AD7799_REG_DATA, 3);

	AD7799_RawToVolt(ad7799);

	return HAL_OK;
}

/**
 * Convert raw value to voltage
 * @param ad7799
 */
void AD7799_RawToVolt(AD7799 *ad7799) {
	float gain = (float) pow(2.0, (double) ad7799->gain);

	if (ad7799->polarity == AD7799_UNIPOLAR) {
		float fullscale = (float) pow(2.0, (double) N);
		ad7799->voltConversion = (float) ad7799->rawConversion
				* (ad7799->vref / (fullscale * gain));
	} else {
		float fullscale = (float) pow(2.0, (double) (N - 1));
		ad7799->voltConversion = (((float) ad7799->rawConversion - fullscale)
				* ad7799->vref) / (fullscale * gain);
	}
}
