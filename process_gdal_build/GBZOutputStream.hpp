#ifndef GBZOUTPUTSTREAM_HPP
#define GBZOUTPUTSTREAM_HPP

#include "zlib.h"
#include "GBOutputStream.hpp"

namespace gb {
  class GBZFileOutputStream;
  class GBZOutputStream;
}

class GB_DLL gb::GBZOutputStream : public gb::GBOutputStream {
public:
  GBZOutputStream(gzFile gzptr): fp(gzptr) {}

  /// Writes a sequence of memory pointed by ptr into the stream
  virtual uint32_t write(const void *ptr, uint32_t size);

protected:
  /// The underlying GZFILE*
  gzFile fp;
};

/// Implements CTBOutputStream for gzipped files
class GB_DLL gb::GBZFileOutputStream : public gb::GBZOutputStream {
public:
  GBZFileOutputStream(const char *fileName);
 ~GBZFileOutputStream();

  void close();
};

#endif
