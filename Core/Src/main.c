/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c2; // CubeMX'de I2C1 kullanıldıysa
int16_t gyro_offset[3] = {0, 0, 0}; // Gyro için offset değerleri
int16_t accel_offset[3] = {0, 0, 0}; // Accelerometer için offset değerleri
int16_t accel[3];
int16_t gyro[3];
float gyro_x_dps;
	                  float gyro_y_dps;
	                  float gyro_z_dps;

	                  float accel_x_g;
	                  float accel_y_g;
	                  float accel_z_g;



#define MPU6050_ADDR 0x68 << 1 // MPU6050 I2C adresi (AD0 GND'ye bağlıysa)

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);

void MPU6050_Init(void) {
    uint8_t check, data;

    // MPU6050 ID kontrolü
    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, 0x75, 1, &check, 1, HAL_MAX_DELAY);
    if (check == 0x68) {
        printf("MPU6050 Bağlandı!\n");

        // Güç yönetimini ayarla (0x6B register, 0x00 değeri)
        data = 0x00;
        HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, 0x6B, 1, &data, 1, HAL_MAX_DELAY);

        // Jiroskop yapılandırması (±250°/s, 0x1B register, 0x00 değeri)
        data = 0x00;
        HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, 0x1B, 1, &data, 1, HAL_MAX_DELAY);

        // İvme ölçer yapılandırması (±2g, 0x1C register, 0x00 değeri)
        data = 0x00;
        HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, 0x1C, 1, &data, 1, HAL_MAX_DELAY);
    } else {
        printf("MPU6050 Bulunamadı!\n");
    }
}

void MPU6050_ReadData(int16_t* accel, int16_t* gyro) {
    uint8_t buffer[14];

    // 0x3B adresinden 14 bayt veri oku (Accel X, Y, Z + Temp + Gyro X, Y, Z)
    HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, 0x3B, 1, buffer, 14, HAL_MAX_DELAY);

    // İvme ölçer verilerini ayrıştır
    accel[0] = (buffer[0] << 8) | buffer[1]; // X
    accel[1] = (buffer[2] << 8) | buffer[3]; // Y
    accel[2] = (buffer[4] << 8) | buffer[5]; // Z

    // Jiroskop verilerini ayrıştır
    gyro[0] = (buffer[8] << 8) | buffer[9];  // X
    gyro[1] = (buffer[10] << 8) | buffer[11];// Y
    gyro[2] = (buffer[12] << 8) | buffer[13];// Z
}


void MPU6050_Calibrate(void) {
    int32_t gyro_sum[3] = {0, 0, 0};
    int32_t accel_sum[3] = {0, 0, 0};
    uint8_t samples = 100; // Kalibrasyon için alınacak örnek sayısı

    for (int i = 0; i < samples; i++) {
        // Sensörden ham verileri oku
        MPU6050_ReadData(accel, gyro);

        // Örnekleri toplama ekle
        gyro_sum[0] += gyro[0];
        gyro_sum[1] += gyro[1];
        gyro_sum[2] += gyro[2];

        accel_sum[0] += accel[0];
        accel_sum[1] += accel[1];
        accel_sum[2] += accel[2];

        HAL_Delay(10); // Her ölçüm arasında kısa bir gecikme
    }

    // Ortalama offset değerlerini hesapla
    gyro_offset[0] = gyro_sum[0] / samples;
    gyro_offset[1] = gyro_sum[1] / samples;
    gyro_offset[2] = gyro_sum[2] / samples;

    accel_offset[0] = accel_sum[0] / samples;
    accel_offset[1] = accel_sum[1] / samples;
    accel_offset[2] = accel_sum[2] / samples;
}


void MPU6050_ApplyOffsets(void) {
    // Offsetleri ham verilere uygula
    gyro[0] -= gyro_offset[0];
    gyro[1] -= gyro_offset[1];
    gyro[2] -= gyro_offset[2];

    accel[0] -= accel_offset[0];
    accel[1] -= accel_offset[1];
    accel[2] -= accel_offset[2];
}



/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	HAL_Init();
	    SystemClock_Config();
	    MX_GPIO_Init();
	    MX_I2C2_Init();

	    int16_t accel[3], gyro[3];

	    // MPU6050 başlat
	    MPU6050_Init();
	    MPU6050_Calibrate();
  while (1)
  {
    /* USER CODE END WHILE */
	  // Verileri oku
	  MPU6050_ReadData(accel, gyro);

	         // Offsetleri uygula
	         MPU6050_ApplyOffsets();
	          gyro_x_dps = gyro[0] / 131.0;
	                   gyro_y_dps = gyro[1] / 131.0;
	                   gyro_z_dps = gyro[2] / 131.0;

	                  accel_x_g = accel[0] / 16384.0;
	                  accel_y_g = accel[1] / 16384.0;
	                  accel_z_g = accel[2] / 16384.0;

	          HAL_Delay(100); // 0.1 saniyede bir veri oku
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10B17DB5;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */