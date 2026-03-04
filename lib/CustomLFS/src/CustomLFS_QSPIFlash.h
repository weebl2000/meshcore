/* 
 * CustomLFS_QSPIFlash.h - QSPI Flash support for CustomLFS
 *
 * Copyright (c) 2025 oltaco <taco@sly.nu>
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CUSTOMLFS_QSPI_H_
#define CUSTOMLFS_QSPI_H_

// Only compile for nRF52840 which has QSPI hardware
#ifdef NRF52840_XXAA

#include "CustomLFS.h"
#include "nrfx_qspi.h"
#include "nrf_qspi.h"
#include "variant.h"

#include "Adafruit_LittleFS.h"
using namespace Adafruit_LittleFS_Namespace;

// if the pins aren't defined we can't compile on boards without QSPI.
#ifndef PIN_QSPI_SCK
#define PIN_QSPI_SCK -1
#endif

#ifndef PIN_QSPI_CS
#define PIN_QSPI_CS -1
#endif

#ifndef PIN_QSPI_IO0
#define PIN_QSPI_IO0 -1
#endif

#ifndef PIN_QSPI_IO1
#define PIN_QSPI_IO1 -1
#endif

#ifndef PIN_QSPI_IO2
#define PIN_QSPI_IO2 -1
#endif

#ifndef PIN_QSPI_IO3
#define PIN_QSPI_IO3 -1
#endif


// QSPI Flash chip configuration structure
struct QSPIFlashChip {
  uint8_t jedec_id[3];             // JEDEC ID bytes [manufacturer, memory_type, capacity]
  uint32_t total_size;             // Total flash size in bytes
  uint32_t sector_size;            // Sector erase size (typically 4KB)
  uint32_t page_size;              // Page program size (typically 256B)
  uint8_t address_bits;            // 24 or 32 bit addressing
  
  // QSPI-specific configuration
  uint8_t read_opcode;             // Fast read opcode (0x0B, 0xEB for quad)
  uint8_t program_opcode;          // Page program opcode (0x02, 0x32 for quad)
  uint8_t erase_opcode;            // Sector erase opcode (0x20, 0xD8)
  uint8_t status_opcode;           // Read status register opcode (0x05)
  
  // Quad mode configuration
  bool supports_quad_read;         // Supports quad read operations
  bool supports_quad_write;        // Supports quad write operations
  uint8_t quad_enable_register;    // Status register for quad enable (1 or 2)
  uint8_t quad_enable_bit;         // Bit position for quad enable
  bool quad_enable_volatile;       // True if quad enable is volatile
  
  // Timing specifications
  uint32_t max_clock_hz;           // Maximum QSPI clock frequency
  uint16_t write_timeout_ms;       // Write operation timeout
  uint16_t erase_timeout_ms;       // Erase operation timeout
  uint16_t startup_delay_us;       // Power-up delay
  
  const char *name;                // Chip name
};

class CustomLFS_QSPIFlash : public CustomLFS
{
private:
  // QSPI hardware configuration
  nrfx_qspi_config_t _qspi_config;

  bool _quad_mode_enabled;
  bool _quad_read_enabled;
  bool _quad_write_enabled;

  // Flash chip information
  const QSPIFlashChip *_chip;
  uint32_t _total_size;
  uint32_t _sector_size;
  uint32_t _page_size;
  bool _is_4byte_addr;
  
  // Performance optimization
  bool _use_quad_read;
  bool _use_quad_write;
  uint32_t _clock_frequency;
  
  // Internal state
  volatile bool _operation_pending;
  nrfx_err_t _last_error;

  // Operation state tracking
  enum QSPIOperation {
    QSPI_OP_NONE,
    QSPI_OP_READ,
    QSPI_OP_WRITE,
    QSPI_OP_ERASE
  };

  volatile QSPIOperation _current_operation;
  volatile bool _operation_complete;
  volatile nrfx_err_t _operation_result;
  
  bool waitForOperation(uint32_t timeout_ms);
  void resetOperationState();

  // Flash detection and configuration
  bool detectChip();
  
  // Low-level flash operations
  bool qspiRead(uint32_t addr, void *buffer, uint32_t size);
  bool qspiWrite(uint32_t addr, const void *buffer, uint32_t size);
  bool qspiErase(uint32_t addr, uint32_t size = 0);  // 0 = sector erase
  bool qspiWaitReady(uint32_t timeout_ms = 10000);
  
  // handy for debugging but maybe remove?
  bool testLFSCallbacks();

  // Status and control operations
  uint8_t readStatus(uint8_t register_num = 1);
  bool writeStatus(uint8_t value, uint8_t register_num = 1);
  
  // Static callback functions for LittleFS operations
  static int _qspi_read(const struct lfs_config *c, lfs_block_t block, 
                        lfs_off_t off, void *buffer, lfs_size_t size);
  static int _qspi_prog(const struct lfs_config *c, lfs_block_t block, 
                        lfs_off_t off, const void *buffer, lfs_size_t size);
  static int _qspi_erase(const struct lfs_config *c, lfs_block_t block);
  static int _qspi_sync(const struct lfs_config *c);
  
  // QSPI event handler
  static void qspi_event_handler(nrfx_qspi_evt_t event, void *p_context);
  
  // Configure LFS for QSPI flash
  virtual void _configure_lfs() override;


public:
  // Constructor
  CustomLFS_QSPIFlash();
  
  bool _qspi_initialized;
  // Initialization with pin configuration
  bool begin(uint8_t sck_pin = PIN_QSPI_SCK, uint8_t csn_pin = PIN_QSPI_CS,
             uint8_t io0_pin = PIN_QSPI_IO0, uint8_t io1_pin = PIN_QSPI_IO1,
             uint8_t io2_pin = PIN_QSPI_IO2, uint8_t io3_pin = PIN_QSPI_IO3);
  
  // Configuration methods
  // bool setClockFrequency(uint32_t frequency_hz);
  
  // Information methods
  uint32_t getFlashSize() const { return _total_size; }
  uint32_t getSectorSize() const { return _sector_size; }
  uint32_t getPageSize() const { return _page_size; }
  uint32_t getClockFrequency() const { return _clock_frequency; }
  const char* getChipName() const { return _chip ? _chip->name : "Unknown"; }

  // remove?
  bool isReady() const { return _qspi_initialized && !_operation_pending; }
  bool isQuadModeEnabled() const { return _quad_mode_enabled; }
  bool supportsQuadRead() const { return _chip ? _chip->supports_quad_read : false; }
  bool supportsQuadWrite() const { return _chip ? _chip->supports_quad_write : false; }

  // Advanced operations
  bool lowLevelFormat();
  bool testFlash();
  

  bool isQSPIReady();
  bool waitForQSPIReady();
   
  // Direct flash access for advanced users
  bool rawRead(uint32_t addr, void *buffer, uint32_t size) { return qspiRead(addr, buffer, size); }
  bool rawWrite(uint32_t addr, const void *buffer, uint32_t size) { return qspiWrite(addr, buffer, size); }
  bool rawErase(uint32_t addr, uint32_t size = 0) { return qspiErase(addr, size); }
  
  // // Memory mapping support (XIP - Execute in Place)
  // void* getMemoryMappedAddress() const { return (void*)0x12000000; }  // nRF52840 QSPI XIP base
  // bool enableMemoryMapping();
  // bool disableMemoryMapping();
  
  // Override parent class methods that don't apply to QSPI
  bool setFlashRegion(uint32_t flash_addr, uint32_t flash_size, 
                      uint32_t block_size = 0) = delete; // Not applicable for external QSPI flash
  
  // Destructor
  ~CustomLFS_QSPIFlash();
};

// Known QSPI flash chips database
extern const QSPIFlashChip qspiFlashChips[];
extern const uint32_t qspiFlashChipCount;

// Global instance for convenience
extern CustomLFS_QSPIFlash QSPIFlash;

// Type alias for easier usage
typedef CustomLFS_QSPIFlash LittleFS_QSPI;

#else
#error "QSPI is only supported on nRF52840"
#endif // NRF52840_XXAA

#endif /* CUSTOMLFS_QSPI_H_ */