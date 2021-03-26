<h1 align="center">Welcome to AD7799-STM32 👋</h1>
<p>
  <a href="https://github.com/axoulc/AD7799-STM32/blob/main/LICENSE" target="_blank">
    <img alt="License: MIT License" src="https://img.shields.io/badge/License-MIT License-yellow.svg" />
  </a>
</p>

> STM32 library for the 24bits ADC Analog Device AD7799

## Default Configuration (on startup or after reset)
- Conversion mode : Continous
- PGA Gain : 128
- Channel : AIN1P-AIN1M
- Polarity : Bipolar
- Sample Rate : 16.7 Hz

## What is not implemented ?
- Continous Conversion

## SPI Configuration
- Data Size : 8bits
- First Bit : MSB First
- Baud Rate : up to 5 MBits
- Clock Polarity : High (Mode 3)
- Clock Phase : 2 Edge (Mode 3)

## Usage

```C
#include "AD7799.h"

AD7799 adc;

int main(void) {
	adc.adcspi = &hspi3; //Your SPI Handler
	adc.CSPort = ADC_CS_GPIO_Port; // Your CS Port
	adc.CSPin = ADC_CS_Pin; // Your CS Pin
	adc.vref = 3.0; // Your Voltage reference
	
	AD7799_Reset(&adc);
	if (AD7799_Init(&adc) != HAL_OK) {
	    Error_Handler();
	}
	AD7799_SetGain(&adc, AD7799_GAIN_64);
	AD7799_SetPolarity(&adc, AD7799_UNIPOLAR);
	
	if (AD7799_SingleConversion(&adc) != HAL_OK) {
	    Error_Handler();
	}
	printf("Raw value : %ld - Voltage Value : %f\r\n", adc.rawConversion, adc.voltConversion);
}
```

## Author

👤 **Axel Chabot**

* Github: [@axoulc](https://github.com/axoulc)

## Show your support

Give a ⭐️ if this project helped you!

## 📝 License

Copyright © 2021 [Axel Chabot](https://github.com/axoulc).<br />
This project is [MIT](https://github.com/axoulc/AD7799-STM32/blob/main/LICENSE) licensed.

***
_This README was generated with ❤️ by [readme-md-generator](https://github.com/kefranabg/readme-md-generator)_
