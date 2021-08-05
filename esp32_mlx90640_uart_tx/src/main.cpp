/*
ESP32 Pins (Lolin 32 lite)
SDA pin 19
SCL pin 23 
 */
#include <Arduino.h>
// #include <WiFi.h>
#include <Wire.h>
#include <WebSocketsServer.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#define TA_SHIFT -64; // Default shift for MLX90640 in open air is 8
static float mlx90640To[768];

TaskHandle_t mlx90640TaskHandle;
xQueueHandle xQueue;

void mlx90640Task(void *parameter);
void receiveTask(void *parameter);

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	delay(1000);
	xQueue = xQueueCreate(1, sizeof(mlx90640To));
	xTaskCreatePinnedToCore(
		mlx90640Task,		 	/* pvTaskCode */
		"mlx90640_Task", 		/* pcName */
		100000,		 			/* usStackDepth */
		NULL,		 			/* pvParameters */
		1,			 			/* uxPriority */
		&mlx90640TaskHandle,	/* pxCreatedTask */
		0);			 			/* xCoreID */
	xTaskCreate(
		receiveTask,   			/* Task function. */
		"Receive_Task", 		/* name of task. */
		10000,		   			/* Stack size of task */
		NULL,		   			/* parameter of the task */
		1,			   			/* priority of the task */
		NULL);		   			/* Task handle to keep track of created task */
}

void loop()
{
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
}

void receiveTask(void *parameter)
{
	/* time to block the task until data is available */
	const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
	while (true)
	{
		/* receive data from the queue */
		BaseType_t xStatus = xQueueReceive(xQueue, &mlx90640To, xTicksToWait);
		/* check whether receiving is ok or not */
		byte mlx90640Byte[768];
		if (xStatus == pdPASS)
		{
			for (int y = 0; y < 24; y++)
			{
				for (int x = 0; x < 32; x++)
				{
					float t = mlx90640To[32 * y + x];
					byte b;
					if (t > 40)
						b = 255;
					else if (t < 20)
						b = 0;
					else
						b = (t - 20) / 20 * 256;
					mlx90640Byte[32 * y + x] = b;
				}
			}
			Serial.write(mlx90640Byte, sizeof(mlx90640Byte));
			// for (int y = 0; y < 24; y++)
			// {
			//   String line = "                                ";
			//   for (int x = 0; x < 32; x++)
			//   {
			//     byte b = mlx90640Byte[32 * y + x];
			//     if (b > 51)
			//       line[x] = '@';
			//     else
			//       line[x] = ' ';
			//   }
			//   Serial.println(line);
			// }
			// Serial.println("-----------------------------");
			// Serial.println(sizeof(byte));
			// Serial.println(sizeof(mlx90640Byte));
		}
	}
	vTaskDelete(NULL);
}

// Capture thermal image on a different thread
void mlx90640Task(void *parameter)
{
	const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

	Wire.setClock(400000L);
	Wire.begin();
	paramsMLX90640 mlx90640;
	Wire.beginTransmission((uint8_t)MLX90640_address);
	if (Wire.endTransmission() != 0)
	{
		Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
		while (1) ;
	}
	Serial.println("MLX90640 online!");

	//Get device parameters - We only have to do this once
	int status;
	uint16_t eeMLX90640[832];
	status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);

	if (status != 0)
	{
		Serial.println("Failed to load system parameters");
	}
	status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
	if (status != 0)
	{
		Serial.println("Parameter extraction failed");
	}
	MLX90640_SetRefreshRate(MLX90640_address, 0x05);
	Wire.setClock(1000000L);
	float mlx90640Background[768];
	while (true)
	{
		//      String startMessage = "Capturing thermal image on core ";
		//      startMessage.concat(xPortGetCoreID());
		//      Serial.println( startMessage );
		//      long startTime = millis();
		for (byte x = 0; x < 2; x++) //Read both subpages
		{
			uint16_t mlx90640Frame[834];
			int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
			if (status < 0)
			{
				Serial.print("GetFrame Error: ");
				Serial.println(status);
			}

			// float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
			float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

			float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
			float emissivity = 0.95;

			MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640Background);
		}
		//      long stopReadTime = millis();
		//      Serial.print("Read rate: ");
		//      Serial.print( 1000.0 / (stopReadTime - startTime), 2);
		//      Serial.println(" Hz");

		/* time to block the task until the queue has free space */
		const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
		xQueueSendToFront(xQueue, &mlx90640Background, xTicksToWait);

		const TickType_t xDelay = 20 / portTICK_PERIOD_MS; // 8 Hz is 1/8 second
		vTaskDelay(xDelay);
	}
}