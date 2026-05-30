#include "MeshWorkerTask.h"

#if defined(NRF52_PLATFORM)
#include <Arduino.h>
#include <MeshCore.h>   // MESH_DEBUG_PRINTLN
#include <FreeRTOS.h>
#include <task.h>

static TaskHandle_t   _meshTaskHandle = nullptr;
static void (*_meshLoopBody)() = nullptr;

static void mesh_worker_task(void*) {
  for (;;) {
    if (_meshLoopBody) _meshLoopBody();
    vTaskDelay(1);   // yield at least one tick (tick-rate independent)
  }
}

bool startMeshWorker(void (*loopBody)()) {
  if (loopBody == nullptr) {
    MESH_DEBUG_PRINTLN("startMeshWorker: null loopBody");
    return false;
  }
  if (_meshTaskHandle != nullptr) {
    MESH_DEBUG_PRINTLN("startMeshWorker: already started");
    return false;
  }
  _meshLoopBody = loopBody;
  // 2048 words = 8KB stack (measured peak ~4.7KB + headroom).
  // TASK_PRIO_LOW (1) matches the framework's own loop_task priority.
  if (xTaskCreate(mesh_worker_task, "mesh", 2048, NULL, TASK_PRIO_LOW, &_meshTaskHandle) != pdPASS) {
    MESH_DEBUG_PRINTLN("startMeshWorker: xTaskCreate failed (out of FreeRTOS heap?)");
    _meshTaskHandle = nullptr;
    return false;
  }
  return true;
}
#endif
