#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_3
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_12

#define ADC_MAX_RAW 4095
#define ADC_MAX_VOLTAGE_MV 3300

void app_main() {
    adc_oneshot_unit_handle_t adcHandle;
    adc_cali_handle_t caliHandle;

    adc_oneshot_unit_init_cfg_t initConfig = {
        .unit_id = ADC_UNIT
    };

    adc_oneshot_new_unit(
        &initConfig,
        &adcHandle
    );

    adc_oneshot_chan_cfg_t channelConfig = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN
    };

    adc_oneshot_config_channel(
        adcHandle,
        ADC_CHANNEL,
        &channelConfig
    );

    adc_cali_curve_fitting_config_t caliConfig = {
        .unit_id = ADC_UNIT,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH

    }; 

    adc_cali_create_scheme_curve_fitting(
        &caliConfig,
        &caliHandle
    );

    int rawValue;
    float manualVoltage;
    int calibratedVoltage;
    float errorPercent;

    while (1)
    {
       adc_oneshot_read(
        adcHandle,
        ADC_CHANNEL,
        &rawValue
       );

        manualVoltage = ((float)rawValue / ADC_MAX_RAW) * ADC_MAX_VOLTAGE_MV;

        adc_cali_raw_to_voltage(
        caliHandle,
        rawValue,
        &calibratedVoltage
        );

        errorPercent =
        ((float)fabs(manualVoltage - calibratedVoltage)
        / calibratedVoltage) * 100.0f;
        
       printf("RAW: %d | U_manual: %2f mV | U_cali: %d mV | Error: %.2f%%\n",
         rawValue,
         manualVoltage,
         calibratedVoltage,
         errorPercent
        );

       vTaskDelay(pdMS_TO_TICKS(100));
    }
    


}