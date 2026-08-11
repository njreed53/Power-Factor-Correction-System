// Main loop code
  while (1)
  {
      if(buffer_full)
      {
          buffer_full = 0;
          float v_mean = 0, i_mean = 0;
          float v_rms = 0, i_rms = 0;
          float v, c, v_real, i_real;
          float real_power = 0, apparent_power = 0;
          float volts_per_count = 3.3f / 4095.0f;
          
          // MEAN CALCULATION (To remove DC offset / center at zero)
          for(int i = 0; i < SAMPLE_COUNT; i++)
          {
              v_mean += adc_dma_buffer[2*i];
              i_mean += adc_dma_buffer[2*i + 1];
          }
          v_mean /= SAMPLE_COUNT;
          i_mean /= SAMPLE_COUNT;

          // RMS + REAL POWER + APPARENT POWER
          for(int i = 0; i < SAMPLE_COUNT; i++)
          {
              v = adc_dma_buffer[2*i] - v_mean;
              c = adc_dma_buffer[2*i + 1] - i_mean;

              v_real = v * volts_per_count;
              i_real = (c * volts_per_count) / SENSOR_GAIN;		// PRIORITY // CT: Is = (Ip / 1000) // Vs = Is * (50 0hm) // Pot gain is 20, 50/1000 = .05*20 = 1
              	  	  	  	  	  	  	  	  	  	  	  	  	// SENSOR GAIN SHOULD BE 1
              v_rms += (v_real * v_real);
              i_rms += (i_real * i_real);

              real_power += (v_real * i_real);
          }
          v_rms = sqrtf(v_rms / SAMPLE_COUNT);
          i_rms = sqrtf(i_rms / SAMPLE_COUNT);

          // --------------------------------------------------------------------------------
          static uint32_t last_switch_time = 0;

    	  // Emergency_Shutoff
          int Emergency;
          if (HAL_GPIO_ReadPin(Emergency_Shutoff_GPIO_Port, Emergency_Shutoff_Pin) == GPIO_PIN_SET)
          {Emergency = 1;}

          // Over/Under Voltage Protection (Relay 8)
          if (v_rms < 108.0f || v_rms > 132.0f || Emergency)
          {
              Relay_Off(8); // Shut off main
              CapBankSwitchLogic(0);
              last_switch_time = HAL_GetTick();
              UART_PutString("VOLTAGE FAULT \r\n");
          }
          else if (HAL_GetTick() - last_switch_time > 3000)
          {Relay_On(8);}

          // Load 1 / Load 2
          if (HAL_GPIO_ReadPin(Load1_GPIO_Port, Load1_Pin) == GPIO_PIN_SET)
          {Relay_On(6);}
          else if (HAL_GetTick() - last_switch_time > 3000)
          {Relay_Off(6);}
          if (HAL_GPIO_ReadPin(Load2_GPIO_Port, Load2_Pin) == GPIO_PIN_SET)
          {Relay_On(7);}
          else if (HAL_GetTick() - last_switch_time > 3000)
          {Relay_Off(7);}
          // ----------------------------------------------------------

          real_power /= SAMPLE_COUNT;
          apparent_power = (v_rms * i_rms);

          // SIGNAL VALIDATION + POWER FACTOR
          if(v_rms < MIN_V || i_rms < MIN_I)
          {
              Relay_Off(1);
              Relay_Off(2);
              Relay_Off(3);
              Relay_Off(4);
              Relay_Off(5);

              relay_state = 0;

              UART_PutString("Signal too small \r\n");
              HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
              //continue; // Returns to top of loop											UNCOMMENT UNCOMMENT UNCOMMENT UNCOMMENT
          }
          else
          {
              pf = real_power / apparent_power;
              float angle = acosf(pf);					// in radians
              pf_corrected = cosf(angle + 0.349); // 20 degrees*(pi/180) = 0.349 radians // 20 degree offset from CT	// Check again after voltage module
          }

          // Clamp
          if(pf_corrected >= 1) pf_corrected = 0.999f;	 // Prevent acos/tan instability
          if(pf_corrected <= -1) pf_corrected = -0.999f; // Prevent acos/tan instability

          // Filter
          float pf_safe = PF_Filter(pf_corrected);

          // REACTIVE POWER
          float Qcurrent = real_power * tanf(acosf(pf_safe));
          float Qtarget  = real_power * tanf(acosf(PF_TARGET));
          float Qc = Qcurrent - Qtarget;
          float C = (Qc / (2.0f * M_PI * freq * (v_rms * v_rms))) * 1000000.0f;

          // DEAD BAND
          if(pf_safe > (PF_TARGET - PF_HYST) && pf_safe < (PF_TARGET + PF_HYST))
          {
              snprintf(msg, 50, "PF: %.2f (stable) \r\n", pf_safe);
              UART_PutString(msg);
              HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
              //continue;																UNCOMMENT UNCOMMENT UNCOMMENT UNCOMMENT
          }

          // RELAY DELAY
          if(HAL_GetTick() - last_switch_time < 3000)
        	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
              //continue;																UNCOMMENT UNCOMMENT UNCOMMENT UNCOMMENT

          // CONTROL LOGIC
          if(pf_safe < (PF_TARGET - PF_HYST))
          {
              // Need more capacitance
              CapBankSwitchLogic(C);
              last_switch_time = HAL_GetTick();
          }
          else if(pf_safe > (PF_TARGET + PF_HYST))
          {
        	  CapBankSwitchLogic(0);
        	  last_switch_time = HAL_GetTick();
          }

          // Print values to terminal
          snprintf(msg, 50, "V: %.2f V, I: %.2f A, PF: %.3f \r\n", v_rms, i_rms, pf_safe);
          UART_PutString(msg);


          // LCD_PutChar(sel+0x30); or just the number you want: 0x3#
          // Print values to LCD
          LCD_ClearDisplay();
          snprintf(lcd_string, 17, "PF: %.2f", pf_safe);
          //LCD_Position(0, 0);
          LCD_PrintString(lcd_string);
          snprintf(lcd_string, 17, "V:%.1f I:%.1f", v_rms, i_rms);
          LCD_Position(1, 0);
          LCD_PrintString(lcd_string);

          HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
      }
}
