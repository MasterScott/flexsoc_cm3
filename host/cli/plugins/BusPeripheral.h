/**
 *  This is the abstract base class for a simulated bus peripheral. Each bus peripheral
 *  has a memory interface and if needed can spawn an async thread to toggle the host
 *  processor IRQ line.
 *
 *  All rights reserved.
 *  Tiny Labs Inc
 *  2019
 */
#ifndef BUSPERIPHERAL_H
#define BUSPERIPHERAL_H

#include <stdint.h>
#include "PluginTarget.h"
#include "pluginhelper.h"
#include "log.h"

class BusPeripheral {
 private:
  uint32_t base;
  bool log = false;
  
 protected:
  PluginTarget *target;
  uint32_t size;
  
 public:
  BusPeripheral (const char *args) {
    plugin_parse_bool (args, "log=", &log);
  }
  virtual ~BusPeripheral () {}

  // Memory interface
  virtual void Init (void) {};
  uint32_t ReadWLog (uint32_t addr, uint32_t data) {
    if (log)
      target->Log (LOG_NORMAL, "[R] %s[%X] = %08X", Name(), addr, data);
    return 0;
  }
  virtual uint32_t ReadW (uint32_t addr) {
    return 0;
  }
  virtual uint16_t ReadH (uint32_t addr) {
    return ReadW (addr) >> ((addr & 3) * 8) & 0xffff;
  }
  virtual uint8_t  ReadB (uint32_t addr) {
    return ReadW (addr) >> ((addr & 3) * 8) & 0xff;
  }
  virtual void WriteW (uint32_t addr, uint32_t data, uint32_t mask) { }
  virtual void WriteWLog (uint32_t addr, uint32_t data, uint32_t mask) {
    if (log)
      target->Log (LOG_NORMAL, "[W] %s[%X] = %X",
                   Name(),
                   target->MaskAddr (addr, mask),
                   target->MaskData (data, mask));
  }
  virtual const char *Name (void) { return "???"; }
  
  // Access base
  uint32_t Base (void) { return base; }
  void SetBase (uint32_t base) { this->base = base; }
  void SetTarget (PluginTarget *targ) { this->target = targ; }
  
  // Return size of busperipheral
  uint32_t Size (void) {
    // Return nearest power of two
    return 1 << (32 - __builtin_clz (size - 1));
  };
};

#endif /* BUSPERIPHERAL_H */

