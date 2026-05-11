/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * Polygalactic - An old school astroid shooting game
  * Developer: Dominik Glozinic
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "fonts.h"
#include <math.h>
#include <stdlib.h>
#include "spaceship.h"
#include "bullet.h"
#include "meteor.h"
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;

/* USER CODE BEGIN PV */
uint32_t adc_buffer[4];

Spaceship  ship;
BulletPool bullets;
MeteorPool meteors;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
static void DrawHUD(int8_t hp);
static void DrawGameOver(void);
static void DrawStartMenu(void);
static void WaitForStart(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static const char * const TITLE[] = {
    " ____  ___  _  _  _  _  ___   __   __   __   __  ____  __  ____ ",
    "(  _ \\/ __)( \\/ )( \\/ )/ __) / _\\ (  ) / _\\ / _\\(_  _)(  )/ ___)",
    " ) __/\\__ \\ )  (  )  /( (_ \\/    \\ )( /    \\/    \\ )(   )(( (__ ",
    "(__)  (___/(_/\\_)(_/\\_) \\___/\\_/\\_/(__)\_/\\_/\_/\_/(__) (__)\\___)  ",
};

#define TITLE_LINES  4
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();

  /* USER CODE BEGIN 2 */

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 4);
  HAL_Delay(50);

  ILI9341_Init();
  ILI9341_FillScreen(COLOR_BLACK);

  Ship_Init(&ship);
  Bullets_Init(&bullets);
  Meteors_Init(&meteors);


  DrawStartMenu();
  WaitForStart();

  // begin game
  ILI9341_FillScreen(COLOR_BLACK);
  Ship_Draw(&ship);
  DrawHUD(ship.hp);

  #define FRAME_MS      33u
  #define SHOT_COOLDOWN 200u
  #define SPAWN_FRAMES  120u

  uint32_t lastTick   = HAL_GetTick();
  uint32_t spawnTimer = 0;
  uint8_t  gameOver   = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {

        uint32_t now = HAL_GetTick();

        if (gameOver) {
        	HAL_Delay(10);
        	continue;
        }

        if (now - lastTick < FRAME_MS) continue;
        lastTick = now;

        Ship_Update(&ship, adc_buffer);

        int32_t rx = -((int32_t)adc_buffer[2] - JOY_CENTRE);
        int32_t ry =   (int32_t)adc_buffer[3]  - JOY_CENTRE;

        if (abs(rx) > JOY_DEADZONE || abs(ry) > JOY_DEADZONE) {
            if (now - ship.lastShotTick >= SHOT_COOLDOWN) {
                ship.lastShotTick = now;
                Bullets_Spawn(&bullets, ship.x, ship.y, ship.angle);
            }
        }

        Bullets_Update(&bullets);
        Meteors_Update(&meteors);
        Bullets_CheckMeteorCollisions(&bullets, &meteors);

        if (Meteors_CheckShipCollision(&meteors, ship.x, ship.y,
                                        (float)SHIP_SIZE)) {
            int8_t prevHp = ship.hp;
            if (Ship_TakeDamage(&ship)) {
                gameOver = 1;
                Ship_Draw(&ship);
                HAL_Delay(400);
                DrawGameOver();
                continue;
            }
            if (ship.hp != prevHp) DrawHUD(ship.hp);
        }

        spawnTimer++;
        if (spawnTimer >= SPAWN_FRAMES) {
            spawnTimer = 0;
            Meteors_Spawn(&meteors);
        }

        Ship_Draw(&ship);
        Bullets_Draw(&bullets);
        Meteors_Draw(&meteors);

    /* USER CODE END WHILE */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_SET);

  /*Configure GPIO pins : PA8 PA10 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void DrawStartMenu(void)
{

    ILI9341_WriteString(64, 18, "POLYGALACTIC", Font_16x26,
                         ILI9341_CYAN, COLOR_BLACK);


    ILI9341_WriteString(93, 52, "~ SHOOT THE ROCKS ~", Font_7x10,
                         ILI9341_YELLOW, COLOR_BLACK);

    // Horizontal rule lines
    ILI9341_FillRectangle(20, 68, 280, 1, ILI9341_CYAN);
    ILI9341_FillRectangle(20, 70, 280, 1, ILI9341_CYAN);


    ship.needsErase = 0;
    Ship_Draw(&ship);


    ILI9341_WriteString(88, 170, "Move to Start", Font_11x18,
                         ILI9341_WHITE, COLOR_BLACK);

    // Small corner stars
    ILI9341_FillRectangle(10,  10,  2, 2, ILI9341_WHITE);
    ILI9341_FillRectangle(308, 10,  2, 2, ILI9341_WHITE);
    ILI9341_FillRectangle(10,  228, 2, 2, ILI9341_WHITE);
    ILI9341_FillRectangle(308, 228, 2, 2, ILI9341_WHITE);
    ILI9341_FillRectangle(160, 8,   2, 2, ILI9341_WHITE);
    ILI9341_FillRectangle(50,  230, 2, 2, ILI9341_WHITE);
    ILI9341_FillRectangle(268, 230, 2, 2, ILI9341_WHITE);
}


static void WaitForStart(void)
{
    // Save resting position
    uint32_t restX = adc_buffer[1];
    uint32_t restY = adc_buffer[0];
    (void)restX;
    (void)restY;  // used via JOY_DEADZONE check

    while (1) {
        HAL_Delay(20);

        int32_t lx = -((int32_t)adc_buffer[1] - JOY_CENTRE);
        int32_t ly =   (int32_t)adc_buffer[0]  - JOY_CENTRE;

        if (abs(lx) > JOY_DEADZONE || abs(ly) > JOY_DEADZONE) {

            // Erase title area
            ILI9341_FillRectangle(0, 0, SCREEN_W, 75, COLOR_BLACK);

            // Erase "Move to Start"
            ILI9341_FillRectangle(0, 160, SCREEN_W, 30, COLOR_BLACK);

            // Erase corner stars
            ILI9341_FillRectangle(0, 220, SCREEN_W, 20, COLOR_BLACK);
            return;
        }
    }
}


static void DrawHUD(int8_t hp)
{
    #define HUD_X    5
    #define HUD_Y    5
    #define HUD_SZ   8
    #define HUD_GAP  3
    for (int i = 0; i < SHIP_MAX_HP; i++) {
        uint16_t col = (i < hp) ? ILI9341_GREEN : COLOR_BLACK;
        ILI9341_FillRectangle(HUD_X + i * (HUD_SZ + HUD_GAP), HUD_Y,
                               HUD_SZ, HUD_SZ, col);
        if (i >= hp) {
            ILI9341_DrawRectangle(HUD_X + i * (HUD_SZ + HUD_GAP), HUD_Y,
                                   HUD_SZ, HUD_SZ, ILI9341_DARKGREY);
        }
    }
}


static void DrawGameOver(void)
{
    ILI9341_FillScreen(COLOR_BLACK);
    ILI9341_WriteString(80, 95,  "GAME  OVER",        Font_16x26, ILI9341_RED,   COLOR_BLACK);
    ILI9341_WriteString(55, 140, "Reset to play again", Font_11x18, ILI9341_WHITE, COLOR_BLACK);
}

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
#ifdef USE_FULL_ASSERT
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
