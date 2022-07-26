#include <cstdio>
#include <cstdlib>

extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
}

static xQueueHandle queue;

static void txThread(void *args) {
  float x = 0;
  for (;;) {
    printf("sending %f...\n", x);
    xQueueSend(queue, &x, portMAX_DELAY);
    x += 0.1;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

static void rxThread(void *args) {
  for (;;) {
    float received;
    if (xQueueReceive(queue, &received, portMAX_DELAY) == pdTRUE) {
      printf("received: %f\n", received);
    }
  }
  vTaskDelete(NULL);
}

static void taskMonitorThread(void *arg) {

  static const TickType_t delay = 500 / portTICK_PERIOD_MS;

  for (;;) {
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();

    uint32_t totalRuntime;
    TaskStatus_t *buffer =
        (TaskStatus_t *)malloc(taskCount * sizeof(TaskStatus_t));
    TaskStatus_t *tasksStatusArray =
        (TaskStatus_t *)malloc(taskCount * sizeof(TaskStatus_t));

    taskCount = uxTaskGetSystemState(buffer, taskCount, &totalRuntime);

    for (int task = 0; task < taskCount; task++)
      tasksStatusArray[buffer[task].xTaskNumber - 1] = buffer[task];

    free(buffer);

    for (int task = 0; task < taskCount; task++) {
      char state;
      switch (tasksStatusArray[task].eCurrentState) {
      case eRunning:
        state = '*';
        break;
      case eReady:
        state = 'R';
        break;
      case eBlocked:
        state = 'B';
        break;
      case eSuspended:
        state = 'S';
        break;
      case eDeleted:
        state = 'D';
        break;
      default:
        state = 0x00;
        break;
      }

      printf("[DEBG] %20s: %c, %lu, %6u, %u\n",
             tasksStatusArray[task].pcTaskName, state,
             tasksStatusArray[task].uxCurrentPriority,
             tasksStatusArray[task].usStackHighWaterMark,
             tasksStatusArray[task].ulRunTimeCounter);
    }
    free(tasksStatusArray);
    printf("[DEBG] Current Heap Free Size: %lu\n", xPortGetFreeHeapSize());

    printf("[DEBG] Minimal Heap Free Size: %lu\n",
           xPortGetMinimumEverFreeHeapSize());

    printf("[DEBG] Total RunTime:  %u ms\n", totalRuntime);

    printf("[DEBG] System Uptime:  %lu ms\n",
           xTaskGetTickCount() * portTICK_PERIOD_MS);

    vTaskDelay(delay);
  }
}

int main() {
  queue = xQueueCreate(1000, sizeof(float));
  xTaskCreate(rxThread, "Rx", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(txThread, "Tx", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(taskMonitorThread, "MONITOR", configMINIMAL_STACK_SIZE, NULL, 0,
              NULL);
  vTaskStartScheduler();
  exit(EXIT_FAILURE);
}

extern "C" {
void vAssertCalled(const char *const pcFileName, unsigned long ulLine) {
  portENTER_CRITICAL();
  printf("Assertion: %s | %lu\n", pcFileName, ulLine);
  exit(EXIT_FAILURE);
  portEXIT_CRITICAL();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  portENTER_CRITICAL();
  printf("Error: statck overflow | %s \n", pcTaskName);
  exit(EXIT_FAILURE);
  portEXIT_CRITICAL();
}

void vApplicationMallocFailedHook() {
  portENTER_CRITICAL();
  printf("Error: malloc failed\n");
  exit(EXIT_FAILURE);
  portEXIT_CRITICAL();
}
}
