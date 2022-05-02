// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>
#include <queue>

#include "rocksdb/sst_file_reader.h"
#include "rocksdb/slice.h"
#include "rocksdb/iterator.h"
#include "rocksdb/cache.h"
#include "rocksdb/options.h"
#include "rocksdb/status.h"
#include "rocksdb/slice.h"

#include "filter.h"

using namespace ROCKSDB_NAMESPACE;
using namespace std;

constexpr uint64_t kNumInternalBytes = 8;

inline Slice ExtractUserKey(const Slice& internal_key) {
  assert(internal_key.size() >= kNumInternalBytes);
  return Slice(internal_key.data(), internal_key.size() - kNumInternalBytes);
}

class InternalKey {
 private:
  string rep_;

 public:
  InternalKey() {}  // Leave rep_ as empty to indicate it is invalid

  void DecodeFrom(const Slice& s) { rep_.assign(s.data(), s.size()); }

  Slice user_key() const { return ExtractUserKey(rep_); }
};


int main() {

    //iterator 만드는데 필요한 인자--------------------------
    /*table rep*/
    int level_ = 0;
    bool blocks_maybe_compressed_ = true;
    bool blocks_definitely_zstd_compressed_ = false;
    const bool immortal_table_ = false;
    Slice cf_name_for_tracing_ = nullptr;
    uint64_t sst_number_for_tracing_ = 0;
    shared_ptr<Cache> to_block_cache_ = nullptr;//to_ = table_options
    uint32_t to_read_amp_bytes_per_bit = 0;
    //std::shared_ptr<const FilterPolicy> to_filter_policy = nullptr;
    //std::shared_ptr<Cache> to_block_cache_compressed = nullptr;
    bool to_cache_index_and_filter_blocks_ = false;
    //ioptions
    uint32_t footer_format_version_ = 5;//footer
    int footer_checksum_type_ = 1;
    uint8_t footer_block_trailer_size_ = 5;
    
    //std::string dev_name = "/usr/local/rocksdb/000051.sst";
    /*block info*/

    std::string dev_name = "/dev/sda";
    // const uint64_t handle_offset = 43673280512;
    // const uint64_t block_size = 3995; //블록 사이즈 틀리지 않게!!
    //-----------------------------------------------------
    
    //Block Read를 위한 SstBlockReader 인스턴스 생성
    Options options;
     SstBlockReader sstBlockReader(options, blocks_maybe_compressed_,
                blocks_definitely_zstd_compressed_, immortal_table_, 
                to_read_amp_bytes_per_bit, dev_name);
    //open -> blockread -> datablock_iterator 생성
    //BlockInfo blockinfo(handle_offset, block_size);
    
      BlockInfo blockinfo = BlockInfo(1,43673284512,4059);

      Status s = sstBlockReader.Open(&blockinfo);
      if(!s.ok()){
          cout << "open error" << endl;
      }

      std::cout << "success" << std::endl;

      //datablock_iterator 획득 -> 버퍼저장
      char scan_buf[4096];
      int rows = 0;
      size_t scan_size = 0;
      const char* row_data;
      size_t row_size;

      Iterator* datablock_iter = sstBlockReader.NewIterator(ReadOptions());
      
      for (datablock_iter->SeekToFirst(); datablock_iter->Valid(); datablock_iter->Next()) {

        Status s = datablock_iter->status();
        if (!s.ok()) {
          cout << "Error reading the block - Skipped \n";
          break;
        }
        
        const Slice& key = datablock_iter->key();
        const Slice& value = datablock_iter->value();

        InternalKey ikey;
        ikey.DecodeFrom(key);

        std::cout << "[Row(HEX)] KEY: " << ikey.user_key().ToString(true) << " | VALUE: "
            << value.ToString(true) << "\n";

        row_data = value.data();
        row_size = value.size();

        memcpy(scan_buf + scan_size, row_data, row_size); 
        
        scan_size += row_size;
        rows += 1;

        std::cout << "  ------\n";

      }

      cout << "*************Send Data To Filtering*******************************************************" << endl;
      cout << "@@scan_size: " << scan_size << " @@rows:" << rows << endl;
      cout << "******************************************************************************************" << endl;

      Filtering(scan_buf, rows);

    
    

    return 0;
}

// int Scan(){

// }

