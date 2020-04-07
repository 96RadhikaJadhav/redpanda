#pragma once
#include "bytes/iobuf.h"
#include "static_deleter_fn.h"

#include <memory>
#include <zstd.h>

namespace compression {
class stream_zstd {
public:
    using zstd_compress_ctx = std::unique_ptr<
      ZSTD_CCtx,
      // wrap ZSTD C API
      static_sized_deleter_fn<ZSTD_CCtx, &ZSTD_freeCCtx>>;
    using zstd_decompress_ctx = std::unique_ptr<
      ZSTD_DCtx,
      // wrap ZSTD C API
      static_sized_deleter_fn<ZSTD_DCtx, &ZSTD_freeDCtx>>;

    iobuf compress(iobuf);
    iobuf uncompress(iobuf);

private:
    void reset_compressor();
    void reset_decompressor();
    zstd_compress_ctx& compressor();
    zstd_decompress_ctx& decompressor();

    zstd_compress_ctx _compress{nullptr};
    zstd_decompress_ctx _decompress{nullptr};
};

} // namespace compression