/* 
 * CustomLFS_QSPIFlash.cpp - QSPI Flash support for CustomLFS
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

#ifdef NRF52840_XXAA

#include "CustomLFS_QSPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include "Arduino.h"

// Check if QSPI is enabled
#if !defined(NRFX_QSPI_ENABLED) || (NRFX_QSPI_ENABLED != 1)
#error "QSPI must be enabled. Add -DNRFX_QSPI_ENABLED=1 to build flags"
#endif

// Simple QSPI Flash chip database - minimal for compatibility
const QSPIFlashChip qspiFlashChips[] = {
  // Winbond W25Q16JV - Very common
  {
    .jedec_id = {0xEF, 0x40, 0x15},
    .total_size = 2097152,              // 2MB
    .sector_size = 4096,
    .page_size = 256,
    .address_bits = 24,
    .read_opcode = 0xEB,
    .program_opcode = 0x32,
    .erase_opcode = 0x20,
    .status_opcode = 0x05,
    .supports_quad_read = true,
    .supports_quad_write = true,
    .quad_enable_register = 2,
    .quad_enable_bit = 1,
    .quad_enable_volatile = false,
    .max_clock_hz = 80000000,
    .write_timeout_ms = 5,
    .erase_timeout_ms = 400,
    .startup_delay_us = 10000,
    .name = "W25Q16JV"
  },
  
  // PUYA P25Q16H - Nordic boards
  {
    .jedec_id = {0x85, 0x60, 0x15},
    .total_size = 2097152,
    .sector_size = 4096,
    .page_size = 256,
    .address_bits = 24,
    .read_opcode = 0xEB,
    .program_opcode = 0x32,
    .erase_opcode = 0x20,
    .status_opcode = 0x05,
    .supports_quad_read = true,
    .supports_quad_write = true,
    .quad_enable_register = 2,
    .quad_enable_bit = 1,
    .quad_enable_volatile = false,
    .max_clock_hz = 55000000,
    .write_timeout_ms = 5,
    .erase_timeout_ms = 300,
    .startup_delay_us = 10000,
    .name = "P25Q16H"
  },
  // Macronix MX25R1635F - Thinknode M1 / Lilygo T-Echo
  {
  .jedec_id = {0xC2, 0x28, 0x15},          // Manufacturer ID (Macronix), Memory Type, Capacity
  .total_size = 2097152,                  // 2MB
  .sector_size = 4096,                    // 4KB sectors
  .page_size = 256,                       // 256-byte pages
  .address_bits = 24,                     // 3-byte addressing
  .read_opcode = 0xEB,                    // Fast Read Quad I/O (4-4-4)
  .program_opcode = 0x38,                 // Quad Page Program (1-1-4)
  .erase_opcode = 0x20,                   // Sector Erase (4KB)
  .status_opcode = 0x05,                  // Read Status Register
  .supports_quad_read = true,
  .supports_quad_write = true,
  .quad_enable_register = 1,             // QE bit is in Status Register-2 (C2h = Volatile SR, but we use SR2)
  .quad_enable_bit = 6,                  // QE bit is bit 6 in Status Register 2
  .quad_enable_volatile = false,         // QE is non-volatile
  .max_clock_hz = 80000000,              // Max QSPI clock rate
  .write_timeout_ms = 5,
  .erase_timeout_ms = 400,
  .startup_delay_us = 10000,
  .name = "MX25R1635F"
},
// Zetta ZD25WQ16BUIGR - Ultra Low Power 16Mb Flash
{
  .jedec_id = {0xBA, 0x60, 0x15},
  .total_size = 2097152,              // 2MB (16Mbit)
  .sector_size = 4096,
  .page_size = 256,
  .address_bits = 24,
  .read_opcode = 0xEB,                // 4x I/O Read (QREAD)
  .program_opcode = 0x32,             // Quad Page Program (QPP)
  .erase_opcode = 0x20,               // Sector Erase (SE)
  .status_opcode = 0x05,              // Read Status Register (RDSR)
  .supports_quad_read = true,
  .supports_quad_write = true,
  .quad_enable_register = 2,          // Status Register S9 (QE bit)
  .quad_enable_bit = 1,               // QE is bit 1 in register 2
  .quad_enable_volatile = false,
  .max_clock_hz = 104000000,          // 104MHz max frequency
  .write_timeout_ms = 3,              // Page program time max
  .erase_timeout_ms = 12,             // Sector erase time max
  .startup_delay_us = 10000,          // Conservative startup delay
  .name = "ZD25WQ16BUIGR"
}
};

const uint32_t qspiFlashChipCount = sizeof(qspiFlashChips) / sizeof(qspiFlashChips[0]);

// Global instance
CustomLFS_QSPIFlash QSPIFlash;

//--------------------------------------------------------------------+
// Event Handler - Simplified for compatibility
//--------------------------------------------------------------------+

// Modified event handler
void CustomLFS_QSPIFlash::qspi_event_handler(nrfx_qspi_evt_t event, void *p_context)
{
  CustomLFS_QSPIFlash* fs = (CustomLFS_QSPIFlash*)p_context;
  
  if (fs) {
    fs->_operation_pending = false;
    fs->_operation_complete = true;
    fs->_operation_result = (event == NRFX_QSPI_EVENT_DONE) ? NRFX_SUCCESS : NRFX_ERROR_INTERNAL;
    fs->_current_operation = QSPI_OP_NONE;
  }
}

//--------------------------------------------------------------------+
// LittleFS Callbacks
//--------------------------------------------------------------------+

int CustomLFS_QSPIFlash::_qspi_read(const struct lfs_config *c, lfs_block_t block, 
                               lfs_off_t off, void *buffer, lfs_size_t size)
{
  CustomLFS_QSPIFlash* fs = (CustomLFS_QSPIFlash*)c->context;
  uint32_t addr = block * c->block_size + off;
  
  bool result = fs->qspiRead(addr, buffer, size);
  if (!result) {
    Serial.print("LFS READ FAILED: block=");
    Serial.print(block);
    Serial.print(", addr=0x");
    Serial.println(addr, HEX);
  }
  
  return result ? 0 : -1;
}

int CustomLFS_QSPIFlash::_qspi_erase(const struct lfs_config *c, lfs_block_t block)
{
  CustomLFS_QSPIFlash* fs = (CustomLFS_QSPIFlash*)c->context;
  uint32_t addr = block * c->block_size;
  
  bool result = fs->qspiErase(addr);
  if (!result) {
    Serial.print("LFS ERASE FAILED: block=");
    Serial.print(block);
    Serial.print(", addr=0x");
    Serial.println(addr, HEX);
  }
  
  return result ? 0 : -1;
}

int CustomLFS_QSPIFlash::_qspi_prog(const struct lfs_config *c, lfs_block_t block, 
                               lfs_off_t off, const void *buffer, lfs_size_t size)
{
  CustomLFS_QSPIFlash* fs = (CustomLFS_QSPIFlash*)c->context;
  uint32_t addr = block * c->block_size + off;
  
  bool result = fs->qspiWrite(addr, buffer, size);
  if (!result) {
    Serial.print("LFS WRITE FAILED: block=");
    Serial.print(block);
    Serial.print(", addr=0x");
    Serial.println(addr, HEX);
  }
  
  return result ? 0 : -1;
}


int CustomLFS_QSPIFlash::_qspi_sync(const struct lfs_config *c)
{
  CustomLFS_QSPIFlash* fs = (CustomLFS_QSPIFlash*)c->context;
  return fs->qspiWaitReady(5000) ? 0 : -1;  // 5 second timeout
}

//--------------------------------------------------------------------+
// CustomLFS_QSPIFlash Implementation
//--------------------------------------------------------------------+

CustomLFS_QSPIFlash::CustomLFS_QSPIFlash()
  : CustomLFS(false)  // Don't auto-configure
  , _qspi_initialized(false)
  , _quad_mode_enabled(false)
  , _chip(nullptr)
  , _total_size(1048576)    // Default 1MB
  , _sector_size(4096)      // Default 4KB sectors
  , _page_size(256)         // Default 256B pages
  , _is_4byte_addr(false)
  , _use_quad_read(false)
  , _use_quad_write(false)
  , _clock_frequency(16000000)  // Default 16MHz
  , _operation_pending(false)
  , _last_error(NRFX_SUCCESS)
{
  memset(&_qspi_config, 0, sizeof(_qspi_config));
}

CustomLFS_QSPIFlash::~CustomLFS_QSPIFlash()
{
  if (_qspi_initialized) {
    nrfx_qspi_uninit();
    _qspi_initialized = false;
  }
}

bool CustomLFS_QSPIFlash::begin(uint8_t sck_pin, uint8_t csn_pin, uint8_t io0_pin, 
                           uint8_t io1_pin, uint8_t io2_pin, uint8_t io3_pin)
{
  if (_qspi_initialized) {
    return false;
  }
  Serial.println("Starting QSPI initialization...");
  
  // Initialize operation state
  _operation_pending = false;
  _operation_complete = false;
  _current_operation = QSPI_OP_NONE;
  _operation_result = NRFX_SUCCESS;
  _quad_mode_enabled = false;
  _quad_read_enabled = false;
  _quad_write_enabled = false;
  
  // Configure using conservative settings initially
  memset(&_qspi_config, 0, sizeof(_qspi_config));
  
  // Pin configuration
  _qspi_config.pins.sck_pin = g_ADigitalPinMap[sck_pin];
  _qspi_config.pins.csn_pin = g_ADigitalPinMap[csn_pin];
  _qspi_config.pins.io0_pin = g_ADigitalPinMap[io0_pin];
  _qspi_config.pins.io1_pin = g_ADigitalPinMap[io1_pin];
  _qspi_config.pins.io2_pin = g_ADigitalPinMap[io2_pin];
  _qspi_config.pins.io3_pin = g_ADigitalPinMap[io3_pin];
  
  // Protocol configuration (start conservative)
  _qspi_config.prot_if.readoc = NRF_QSPI_READOC_FASTREAD;
  _qspi_config.prot_if.writeoc = NRF_QSPI_WRITEOC_PP;
  _qspi_config.prot_if.addrmode = NRF_QSPI_ADDRMODE_24BIT;
  _qspi_config.prot_if.dpmconfig = false;
  
  // Physical interface (start with conservative speed)
  _qspi_config.phy_if.sck_freq = NRF_QSPI_FREQ_32MDIV2;
  _qspi_config.phy_if.spi_mode = NRF_QSPI_MODE_0;
  _qspi_config.phy_if.dpmen = false;
  
  _clock_frequency = 16000000;
  
  
  // Initialize QSPI with event handler
  nrfx_err_t err = nrfx_qspi_init(&_qspi_config, qspi_event_handler, this);
  if (err != NRFX_SUCCESS) {
    Serial.print("QSPI init failed: 0x");
    Serial.println(err, HEX);
    return false;
  }
  NVIC_SetPriority(QSPI_IRQn, 2);  // Set QSPI IRQ priority lower to avoid conflict.

  
  Serial.println("QSPI driver initialized");
  
  // Activate QSPI tasks
  Serial.println("Activating QSPI...");
  NRF_QSPI->TASKS_ACTIVATE = 1;
  
  // Wait for QSPI to be ready
  Serial.println("Waiting for QSPI ready...");
  if (!waitForQSPIReady()) {
    Serial.println("QSPI ready timeout!");
    return false;
  }
  
  Serial.println("QSPI is ready");
  _qspi_initialized = true;
  
  // Detect flash chip
  if (!detectChip()) {
    Serial.println("Chip detection failed");
    return false;
  }
  
  // // Run comprehensive tests
  // testFlash();
  
  _configure_lfs();
  // testLFSCallbacks();

  // Mount filesystem
  if (!Adafruit_LittleFS::begin(&_lfs_config)) {
    Serial.println("Mount failed, formatting...");
    if (!format()) {
      Serial.println("Format failed!");
      return false;
    }
    if (!Adafruit_LittleFS::begin(&_lfs_config)) {
      Serial.println("Mount failed after format!");
      return false;
    }
    Serial.println("Formatted and mounted");
  } else {
    Serial.println("Filesystem mounted");
  }
  
  return true;
}

// handy for debugging but can probably be removed now.
bool CustomLFS_QSPIFlash::testLFSCallbacks()
{
  Serial.println("Testing LittleFS callbacks directly...");
  
  resetOperationState();
  
  uint8_t testBuffer[256];
  memset(testBuffer, 0xAA, sizeof(testBuffer));
  
  Serial.println("Testing erase callback...");
  int result = _qspi_erase(&_lfs_config, 0);
  if (result != 0) {
    Serial.print("Erase callback failed: ");
    Serial.println(result);
    resetOperationState();
    return false;
  }
  
  Serial.println("Testing write callback...");
  result = _qspi_prog(&_lfs_config, 0, 0, testBuffer, sizeof(testBuffer));
  if (result != 0) {
    Serial.print("Write callback failed: ");
    Serial.println(result);
    resetOperationState();
    return false;
  }
  
  Serial.println("Testing read callback...");
  uint8_t readBuffer[256];
  result = _qspi_read(&_lfs_config, 0, 0, readBuffer, sizeof(readBuffer));
  if (result != 0) {
    Serial.print("Read callback failed: ");
    Serial.println(result);
    resetOperationState();
    return false;
  }
  
  bool dataOk = true;
  for (int i = 0; i < 256; i++) {
    if (readBuffer[i] != 0xAA) {
      dataOk = false;
      break;
    }
  }
  
  Serial.print("Callback test: ");
  Serial.println(dataOk ? "SUCCESS" : "FAILED");
  
  resetOperationState();
  
  return dataOk;
}

// Callback-compatible qspiRead()
bool CustomLFS_QSPIFlash::qspiRead(uint32_t addr, void *buffer, uint32_t size)
{
  if (!_qspi_initialized || !buffer || size == 0) {
    return false;
  }
  
  if (_operation_pending) {
    return false;
  }
  
  _operation_pending = true;
  _operation_complete = false;
  _current_operation = QSPI_OP_READ;
  _operation_result = NRFX_ERROR_BUSY;
  
  nrfx_err_t err = nrfx_qspi_read(buffer, size, addr);
  if (err != NRFX_SUCCESS) {
    resetOperationState();
    return false;
  }
  
  bool success = waitForOperation(5000);
  
  if (_operation_pending) {
    resetOperationState();
  }
  
  return success;
}

// Callback-compatible qspiWrite()
bool CustomLFS_QSPIFlash::qspiWrite(uint32_t addr, const void *buffer, uint32_t size)
{
  if (!_qspi_initialized || !buffer || size == 0) {
    return false;
  }
  
  if (_operation_pending) {
    return false;
  }
  
  _operation_pending = true;
  _operation_complete = false;
  _current_operation = QSPI_OP_WRITE;
  _operation_result = NRFX_ERROR_BUSY;
  
  nrfx_err_t err = nrfx_qspi_write(buffer, size, addr);
  if (err != NRFX_SUCCESS) {
    resetOperationState();
    return false;
  }
  
  bool success = waitForOperation(10000); // Longer timeout for writes
  
  if (_operation_pending) {
    resetOperationState();
  }
  
  return success;
}

// Callback-compatible qspiErase()
bool CustomLFS_QSPIFlash::qspiErase(uint32_t addr, uint32_t size)
{
  if (!_qspi_initialized) {
    return false;
  }
  
  if (_operation_pending) {
    return false;
  }
  
  _operation_pending = true;
  _operation_complete = false;
  _current_operation = QSPI_OP_ERASE;
  _operation_result = NRFX_ERROR_BUSY;
  
  nrfx_err_t err = nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, addr);
  if (err != NRFX_SUCCESS) {
    resetOperationState();
    return false;
  }
  
  bool success = waitForOperation(30000); // Longest timeout for erase
  
  if (_operation_pending) {
    resetOperationState();
  }
  
  return success;
}

void CustomLFS_QSPIFlash::resetOperationState()
{
  _operation_pending = false;
  _operation_complete = false;
  _current_operation = QSPI_OP_NONE;
  _operation_result = NRFX_SUCCESS;
}

// Helper function - waitForOperation()
bool CustomLFS_QSPIFlash::waitForOperation(uint32_t timeout_ms)
{
  uint32_t start = millis();
  
  while (_operation_pending && (millis() - start) < timeout_ms) {
    delay(1);
    
    if (_operation_complete) {
      _operation_pending = false;
      return (_operation_result == NRFX_SUCCESS);
    }
  }
  
  if (_operation_complete) {
    _operation_pending = false;
    return (_operation_result == NRFX_SUCCESS);
  }
  
  if (_operation_pending) {
    resetOperationState();
    return false;
  }
  
  return (_operation_result == NRFX_SUCCESS);
}


uint8_t CustomLFS_QSPIFlash::readStatus(uint8_t register_num)
{
  if (!_qspi_initialized) {
    return 0xFF;
  }
  
  uint8_t opcode;
  switch (register_num) {
    case 1:
      opcode = 0x05;  // Read Status Register 1
      break;
    case 2:
      opcode = 0x35;  // Read Status Register 2
      break;
    case 3:
      opcode = 0x15;  // Read Status Register 3
      break;
    default:
      return 0xFF;
  }
  
  uint8_t status = 0xFF;
  nrf_qspi_cinstr_conf_t cfg = {
    .opcode = opcode,
    .length = NRF_QSPI_CINSTR_LEN_2B,
    .io2_level = true,
    .io3_level = true,
    .wipwait = false,
    .wren = false
  };
  
  if (nrfx_qspi_cinstr_xfer(&cfg, NULL, &status) == NRFX_SUCCESS) {
    return status;
  }
  
  return 0xFF;
}

bool CustomLFS_QSPIFlash::writeStatus(uint8_t value, uint8_t register_num)
{
  if (!_qspi_initialized) {
    return false;
  }
  
  // Write Enable first
  nrf_qspi_cinstr_conf_t cfg = {
    .opcode = 0x06,  // WREN
    .length = NRF_QSPI_CINSTR_LEN_1B,
    .io2_level = true,
    .io3_level = true,
    .wipwait = false,
    .wren = false
  };
  
  if (nrfx_qspi_cinstr_xfer(&cfg, NULL, NULL) != NRFX_SUCCESS) {
    return false;
  }
  
  // Write status register
  uint8_t opcode;
  switch (register_num) {
    case 1:
      opcode = 0x01;  // Write Status Register 1
      break;
    case 2:
      opcode = 0x31;  // Write Status Register 2 (some chips)
      break;
    case 3:
      opcode = 0x11;  // Write Status Register 3 (some chips)
      break;
    default:
      return false;
  }
  
  cfg.opcode = opcode;
  cfg.length = NRF_QSPI_CINSTR_LEN_2B;
  cfg.wipwait = true;  // Wait for operation to complete
  
  return (nrfx_qspi_cinstr_xfer(&cfg, &value, NULL) == NRFX_SUCCESS);
}



bool CustomLFS_QSPIFlash::waitForQSPIReady()
{
  uint32_t timeout = millis() + 5000;
  
  while (millis() < timeout) {
    // Use the same check as the working example
    uint32_t status = NRF_QSPI->STATUS;
    if ((status & 0x08) == 0x08 && (status & 0x01000000) == 0) {
      return true;
    }
    delay(1);
  }
  
  return false;
}

bool CustomLFS_QSPIFlash::detectChip()
{
  uint8_t jedec_id[3] = {0, 0, 0};
  
  // Try to read JEDEC ID
  nrf_qspi_cinstr_conf_t cinstr_cfg = {
    .opcode = 0x9F,
    .length = NRF_QSPI_CINSTR_LEN_4B,
    .io2_level = true,
    .io3_level = true,
    .wipwait = false,
    .wren = false
  };
  
  nrfx_err_t err = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, jedec_id);
  if (err != NRFX_SUCCESS) {
    Serial.print("JEDEC read failed: 0x");
    Serial.println(err, HEX);
    return false;
  }
  
  Serial.print("JEDEC ID: ");
  for (int i = 0; i < 3; i++) {
    if (jedec_id[i] < 0x10) Serial.print("0");
    Serial.print(jedec_id[i], HEX);
    if (i < 2) Serial.print(" ");
  }
  Serial.println();
  
  // Check for valid ID
  if ((jedec_id[0] == 0x00 && jedec_id[1] == 0x00 && jedec_id[2] == 0x00) ||
      (jedec_id[0] == 0xFF && jedec_id[1] == 0xFF && jedec_id[2] == 0xFF)) {
    Serial.println("Invalid JEDEC ID - check connections");
    return false;
  }
  
  // Look for known chip
  for (uint32_t i = 0; i < qspiFlashChipCount; i++) {
    if (jedec_id[0] == qspiFlashChips[i].jedec_id[0] && 
        jedec_id[1] == qspiFlashChips[i].jedec_id[1] && 
        jedec_id[2] == qspiFlashChips[i].jedec_id[2]) {
      
      _chip = &qspiFlashChips[i];
      _total_size = _chip->total_size;
      _sector_size = _chip->sector_size;
      _page_size = _chip->page_size;
      
      Serial.print("Known chip: ");
      Serial.println(_chip->name);
      return true;
    }
  }
  
  Serial.println("Unknown chip - using defaults");
  return true;  // Continue with defaults
}



bool CustomLFS_QSPIFlash::qspiWaitReady(uint32_t timeout_ms)
{
  uint32_t start = millis();
  
  while ((millis() - start) < timeout_ms) {
    // Simple status check using custom instruction
    uint8_t status = 0xFF;
    nrf_qspi_cinstr_conf_t cfg = {
      .opcode = 0x05,  // Read status
      .length = NRF_QSPI_CINSTR_LEN_2B,
      .io2_level = true,
      .io3_level = true,
      .wipwait = false,
      .wren = false
    };
    
    if (nrfx_qspi_cinstr_xfer(&cfg, NULL, &status) == NRFX_SUCCESS) {
      if ((status & 0x01) == 0) {  // WIP bit clear
        return true;
      }
    }
    
    delay(1);
  }
  
  return false;
}

void CustomLFS_QSPIFlash::_configure_lfs()
{
  memset(&_lfs_config, 0, sizeof(_lfs_config));
  
  _lfs_config.context = this;
    // DEBUG: Verify context pointer immediately
  Serial.print("_configure_lfs: Setting context to: 0x");
  Serial.println((uintptr_t)this, HEX);
  Serial.print("_configure_lfs: Context verification - _operation_pending = ");
  Serial.println(_operation_pending);
  Serial.print("_configure_lfs: Context verification - _qspi_initialized = ");
  Serial.println(_qspi_initialized);
  
  // Test the context pointer immediately
  CustomLFS_QSPIFlash* test_ctx = (CustomLFS_QSPIFlash*)_lfs_config.context;
  Serial.print("_configure_lfs: Retrieved context: 0x");
  Serial.println((uintptr_t)test_ctx, HEX);
  Serial.print("_configure_lfs: Retrieved context _operation_pending = ");
  Serial.println(test_ctx->_operation_pending);
  

  // Callbacks
  _lfs_config.read = _qspi_read;
  _lfs_config.prog = _qspi_prog;
  _lfs_config.erase = _qspi_erase;
  _lfs_config.sync = _qspi_sync;
  
  // CRITICAL: Adjust these parameters for QSPI flash
  _lfs_config.read_size = 256;           // Match page size
  _lfs_config.prog_size = 256;           // Match page size
  _lfs_config.block_size = _sector_size; // 4096 bytes
  _lfs_config.block_count = _total_size / _sector_size;
  
  // Reduce lookahead for smaller flash
  _lfs_config.lookahead = 32;            // Smaller lookahead for 2MB flash
  
  // Allocate static buffers instead of dynamic (more reliable)
  static uint8_t read_buffer[256];
  static uint8_t prog_buffer[256];
  static uint8_t lookahead_buffer[32];
  
  _lfs_config.read_buffer = read_buffer;
  _lfs_config.prog_buffer = prog_buffer;
  _lfs_config.lookahead_buffer = lookahead_buffer;
  _lfs_config.file_buffer = NULL;        // Keep this dynamic
  
  // Update parent class members
  _flash_addr = 0;
  _flash_total_size = _total_size;
  _block_size = _sector_size;
  
  // // Debug the configuration
  // Serial.println("=== LittleFS Configuration ===");
  // Serial.print("Block size: "); Serial.println(_lfs_config.block_size);
  // Serial.print("Block count: "); Serial.println(_lfs_config.block_count);
  // Serial.print("Read size: "); Serial.println(_lfs_config.read_size);
  // Serial.print("Prog size: "); Serial.println(_lfs_config.prog_size);
  // Serial.print("Lookahead: "); Serial.println(_lfs_config.lookahead);
  // Serial.print("Total size: "); 
  // Serial.print(_lfs_config.block_count * _lfs_config.block_size / 1024);
  // Serial.println(" KB");
}

bool CustomLFS_QSPIFlash::testFlash()
{
  if (!_qspi_initialized) {
    return false;
  }
  
  Serial.println("Testing QSPI flash...");
  
  uint8_t testData[256];
  uint8_t readData[256];
  
  // Create test pattern
  for (int i = 0; i < 256; i++) {
    testData[i] = i ^ 0xAA;
  }
  
  // Use last sector
  uint32_t testAddr = _total_size - _sector_size;
  
  // Erase
  if (!qspiErase(testAddr)) {
    Serial.println("Erase failed");
    return false;
  }
  
  // Write
  if (!qspiWrite(testAddr, testData, sizeof(testData))) {
    Serial.println("Write failed");
    return false;
  }
  
  // Read
  if (!qspiRead(testAddr, readData, sizeof(readData))) {
    Serial.println("Read failed");
    return false;
  }
  
  // Compare
  for (int i = 0; i < 256; i++) {
    if (testData[i] != readData[i]) {
      Serial.print("Verify failed at ");
      Serial.println(i);
      return false;
    }
  }
  
  Serial.println("Flash test passed");
  return true;
}

bool CustomLFS_QSPIFlash::lowLevelFormat()
{
  Serial.println("WARNING: This will erase all data!");
  Serial.println("Formatting flash...");
  
  uint32_t sectors = _total_size / _sector_size;
  for (uint32_t i = 0; i < sectors; i++) {
    if (!qspiErase(i * _sector_size)) {
      Serial.print("Format failed at sector ");
      Serial.println(i);
      return false;
    }
    
    if ((i % 32) == 0) {
      Serial.print("Progress: ");
      Serial.print((i * 100) / sectors);
      Serial.println("%");
    }
  }
  
  Serial.println("Format complete");
  return true;
}

bool CustomLFS_QSPIFlash::isQSPIReady()
{
  if (!_qspi_initialized) {
    return false;
  }
  
  // Check if QSPI is enabled and ready
  bool enabled = (NRF_QSPI->ENABLE == 1);
  bool ready = (NRF_QSPI->STATUS & QSPI_STATUS_READY_Msk) != 0;
  
  Serial.print("QSPI enabled: ");
  Serial.print(enabled);
  Serial.print(", ready: ");
  Serial.println(ready);
  
  return enabled && ready;
}


// // placeholders for now
// bool CustomLFS_QSPIFlash::setClockFrequency(uint32_t freq) { return false; }
// bool CustomLFS_QSPIFlash::enableMemoryMapping() { return _qspi_initialized; }
// bool CustomLFS_QSPIFlash::disableMemoryMapping() { return true; }

#endif // NRF52840_XXAA