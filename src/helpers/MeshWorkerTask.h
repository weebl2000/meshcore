#pragma once

#if defined(NRF52_PLATFORM)
#include <FreeRTOS.h>
#include <task.h>

// The Adafruit_nRF52_Arduino framework runs Arduino loop() on a 4KB task stack
// (LOOP_STACK_SZ = 256*4, an unconditional #define that build flags can't override).
// Heavy LittleFS work from loop() (e.g. ClientACL::saveSessionKeys) overflows it and
// corrupts adjacent heap. Run the app loop on a dedicated task with an 8KB stack.
static TaskHandle_t _meshTaskHandle = nullptr;
static void (*_meshLoopBody)() = nullptr;

static void _mesh_worker_task(void*) {
  for (;;) {
    _meshLoopBody();
    vTaskDelay(pdMS_TO_TICKS(1));   // yield
  }
}

// 2048 words = 8KB stack. TASK_PRIO_LOW matches the framework's own loop_task.
static inline void startMeshWorker(void (*body)()) {
  _meshLoopBody = body;
  xTaskCreate(_mesh_worker_task, "mesh", 2048, NULL, TASK_PRIO_LOW, &_meshTaskHandle);
}
#endif
