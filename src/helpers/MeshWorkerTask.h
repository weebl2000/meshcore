#pragma once

#if defined(NRF52_PLATFORM)
// Run an application loop body on a dedicated FreeRTOS task with an 8KB stack,
// instead of the Adafruit_nRF52_Arduino framework's Arduino loop_task (whose 4KB
// stack -- LOOP_STACK_SZ = 256*4, an unconditional #define that build flags can't
// override -- is too small for LittleFS file opens done from loop(), e.g.
// ClientACL::saveSessionKeys, which overflow it and corrupt adjacent heap).
//
// Definitions live in MeshWorkerTask.cpp so there is a single shared instance.
// Returns true if the task was created, false on FreeRTOS heap exhaustion.
bool startMeshWorker(void (*loopBody)());
#endif
