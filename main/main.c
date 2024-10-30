#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

QueueHandle_t dataQueue;
EventGroupHandle_t eventGroup;

#define QUEUE_FULL_EVENT (1 << 0)

// Task สำหรับส่งข้อมูลไปยัง Queue
void ProducerTask(void *arg)
{
    uint16_t i = 0;
    while (1)
    {
        printf("ProducerTask: Sending %d to queue\n", i);
        xQueueSend(dataQueue, &i, portMAX_DELAY);
        i++;
        
        // เมื่อ Queue เต็ม ส่งสัญญาณ Event
        if (uxQueueMessagesWaiting(dataQueue) == 5)
        {
            xEventGroupSetBits(eventGroup, QUEUE_FULL_EVENT);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS); // หน่วง 0.5 วินาที
    }
}

// Task สำหรับรับข้อมูลจาก Queue
void ConsumerTask(void *arg)
{
    uint16_t receivedValue;
    while (1)
    {
        if (xQueueReceive(dataQueue, &receivedValue, portMAX_DELAY))
        {
            printf("ConsumerTask: Received %d from queue\n", receivedValue);
        }
    }
}

// Task ที่จะทำงานเมื่อ Queue เต็ม
void MonitorTask(void *arg)
{
    while (1)
    {
        xEventGroupWaitBits(eventGroup, QUEUE_FULL_EVENT, pdTRUE, pdFALSE, portMAX_DELAY);
        printf("MonitorTask: Queue is full, processing data...\n");
    }
}

void app_main(void)
{
    // สร้าง Queue และ Event Group
    dataQueue = xQueueCreate(5, sizeof(uint16_t));
    eventGroup = xEventGroupCreate();

    // สร้าง Task สำหรับ Producer, Consumer และ Monitor
    xTaskCreate(ProducerTask, "ProducerTask", 2048, NULL, 10, NULL);
    xTaskCreate(ConsumerTask, "ConsumerTask", 2048, NULL, 10, NULL);
    xTaskCreate(MonitorTask, "MonitorTask", 2048, NULL, 10, NULL);
}
