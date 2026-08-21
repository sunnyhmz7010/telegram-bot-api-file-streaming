//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2025
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "server/FileStreamCore.h"

namespace telegram_bot_api {

td::Status FileStreamCursor::update_progress(td::int64 download_offset, td::int64 downloaded_prefix_size,
                                             bool is_completed) {
  if (total_size < 0 || download_offset < 0 || downloaded_prefix_size < 0 || download_offset > total_size ||
      downloaded_prefix_size > total_size - download_offset) {
    return td::Status::Error(500, "Inconsistent file download progress");
  }
  if (end_offset < start_offset && total_size > 0 && start_offset == 0) {
    end_offset = total_size - 1;
  }
  if (download_offset == 0 && downloaded_prefix_size > contiguous_end) {
    contiguous_end = downloaded_prefix_size;
  }
  if (is_completed && download_offset == 0 && downloaded_prefix_size == total_size) {
    contiguous_end = total_size;
  }
  return td::Status::OK();
}

td::int64 FileStreamCursor::next_read_size(td::int64 chunk_size) const {
  if (chunk_size <= 0 || total_size < 0 || end_offset < start_offset || next_offset > end_offset ||
      next_offset >= contiguous_end) {
    return 0;
  }
  return td::min(chunk_size, td::min(contiguous_end - next_offset, end_offset + 1 - next_offset));
}

td::Status FileStreamCursor::commit(td::int64 offset, td::int64 size) {
  if (offset != next_offset || size <= 0 || size > contiguous_end - next_offset ||
      size > end_offset + 1 - next_offset) {
    return td::Status::Error(500, "Out-of-order, duplicate, or incomplete file stream block");
  }
  next_offset += size;
  return td::Status::OK();
}

}  // namespace telegram_bot_api
