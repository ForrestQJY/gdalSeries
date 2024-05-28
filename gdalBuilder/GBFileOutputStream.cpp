

#include "GBFileOutputStream.hpp"

using namespace gb;

/**
 * @details 
 * Writes a sequence of memory pointed by ptr into the FILE*.
 */
uint32_t
gb::GBFileOutputStream::write(const void *ptr, uint32_t size) {
  return (uint32_t)fwrite(ptr, size, 1, fp);
}

/**
 * @details 
 * Writes a sequence of memory pointed by ptr into the ostream. 
 */
uint32_t
gb::GBStdOutputStream::write(const void *ptr, uint32_t size) {
  mstream.write((const char *)ptr, size);
  return size;
}
