#ifndef MED_ADDRESS_HPP
#define MED_ADDRESS_HPP

#include <string>
#include <thread>

#include "med/MedScan.hpp"

/**
 * This is the address will be saved, re-use, lock, etc.
 */
class MedAddress : public MedScan {
  friend class Med;
public:
  MedAddress();
  MedAddress(MemAddr address);
  ~MedAddress();

  string description;
  bool lock; /**< Not using mutex */

  void lockValue(string pid);
  void unlockValue();
  void setLockedValue(string value);
  string getLockedValue();

private:
  string lockedValue;
  std::thread* lockThread;
};

#endif
