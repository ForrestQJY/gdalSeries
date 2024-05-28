#include "GBException.hpp"
#include "GBZOutputStream.hpp"

using namespace gb;

/**
 * @details 
 * Writes a sequence of memory pointed by ptr into the GZFILE*.
 */
uint32_t
gb::GBZOutputStream::write(const void *ptr, uint32_t size) {
  if (size == 1) {
    int c = *((const char *)ptr);
    return gzputc(fp, c) == -1 ? 0 : 1;
  }
  else {
    return gzwrite(fp, ptr, size) == 0 ? 0 : size;
  }
}

gb::GBZFileOutputStream::GBZFileOutputStream(const char *fileName) : GBZOutputStream(NULL) {
  gzFile file = gzopen(fileName, "wb");

  if (file == NULL) {
    throw GBException("Failed to open file");
  }
  fp = file;
}

gb::GBZFileOutputStream::~GBZFileOutputStream() {
  close();
}

void
gb::GBZFileOutputStream::close() {

  // Try and close the file
  if (fp) {
    switch (gzclose(fp)) {
    case Z_OK:
      break;
    case Z_STREAM_ERROR:
    case Z_ERRNO:
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
    default:
      throw GBException("Failed to close file");
    }
    fp = NULL;
  }
}
