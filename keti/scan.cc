// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory)
#include "scan.h"

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

void Scan::Scanning(){
  // dev_fd = open("/dev/ngd-blk", O_RDONLY);
  cout << "<-----------  Scan Layer Running...  ----------->\n";

    while (1){
        Snippet snippet = ScanQueue.wait_and_pop();

        cout << "\n[Get Parsed Snippet] \n #Snippet Work ID: " << snippet.work_id << "" << endl;	  

        //sleep(1); // 2

        TableRep table_rep = CSDTableManager_.GetTableRep(snippet.table_name);
		                
        Options options;
        SstBlockReader sstBlockReader(
            options, table_rep.blocks_maybe_compressed, table_rep.blocks_definitely_zstd_compressed, 
            table_rep.immortal_table, table_rep.read_amp_bytes_per_bit, table_rep.dev_name);

        list<BlockInfo> &bl = snippet.block_info_list;
        list<BlockInfo>::iterator it;
        int block_count = 1;
        for(it = bl.begin(); it != bl.end(); it++){//블록
            BlockInfo b = *it;
            BlockScan(sstBlockReader, &b, snippet, block_count);
            block_count += 1;
        }
    }
}

void Scan::BlockScan(SstBlockReader& sstBlockReader_, BlockInfo* blockInfo, Snippet &snippet_, int bc){
  //cout << "\n[1.Block Scan Start]" << endl;
  // cout << "- WorkID: " << snippet_.work_id << endl;
  // cout << "- Block( " << bc << " / " << snippet_.block_info_list.size() << " ) ID: " << blockInfo->block_id << endl;  
  // cout << "- Block Size: " << blockInfo->block_size << endl;

  // char scan_buf[4096];

  
  // lseek(dev_fd,blockInfo->block_offset,SEEK_SET);
  // int read_size = read(dev_fd, scan_buf, blockInfo->block_size);

  // std::cout << "#buffer_read_size : " << read_size << std::endl;
  
  // close(dev_fd);

  Status s  = sstBlockReader_.Open(blockInfo);
  if(!s.ok()){
      cout << "open error" << endl;
  }

  char scan_buf[4096];
  char *ptr = scan_buf;
  int rows = 0;
  size_t scan_size = 0;
  const char* row_data;
  size_t row_size;
  vector<int> row_offset;

  Iterator* datablock_iter = sstBlockReader_.NewIterator(ReadOptions());
    
  //cout << "\n[2.Read Block Rows...]\n" << endl;

  for (datablock_iter->SeekToFirst(); datablock_iter->Valid(); datablock_iter->Next()) {
      row_offset.push_back(scan_size);

      Status s = datablock_iter->status();
      if (!s.ok()) {
      cout << "Error reading the block - Skipped \n";
      break;
      }
      
      const Slice& key = datablock_iter->key();
      const Slice& value = datablock_iter->value();

      InternalKey ikey;
      ikey.DecodeFrom(key);

      //std::cout << "[Row(HEX)] KEY: " << ikey.user_key().ToString(true) << " | VALUE: ";

      row_data = value.data();
      row_size = value.size();

      // for(int i=0; i<row_size; i++){
      //   printf("%02X",(u_char)row_data[i]);
      // }
      // cout << endl;

      for(int i=0; i<row_size; i++){
          *ptr++ = row_data[i];
      }
      
      scan_size += row_size;
      rows += 1;
  }

  //sleep(1); // 5

  // cout << "-------------------------------Scan Result-----------------------------------------"<<endl;
  // cout << "| WorkID: " << snippet_.work_id <<
  //         " | BlockID( " << bc << " / " << snippet_.block_info_list.size() << " ) : " << blockInfo->block_id <<  
  //         " | scan size: " << scan_size << " | rows: " << rows
  // << " | block size: " << blockInfo->block_size << " | " << endl; 
  // printf("#offset: [ ");
  // for(int j=0; j<rows+1; j++){
  //     printf("%d ,", temp_offset[j]);
  // }
  // cout<< "]" << endl;
  // cout << "----------------------------------------------------------------------------------"<<endl;
  // for(int i=0; i<scan_size; i++){
  //     printf("%02X",(u_char)scan_buf[i]);
  // }
  // cout << "\n----------------------------------------------------------------------------------"<<endl;
  
  SendData(scan_buf, ptr, rows, blockInfo->block_id, snippet_.total_block_count, snippet_, row_offset); 

}

void Scan::SendData(char *scan_buf_, char* ptr_, int rows, int block_id_, 
                    int total_block_count_, Snippet snippet_, vector<int> row_offset_){
    ScanResult scanResult(
        snippet_.work_id, block_id_, total_block_count_, scan_buf_, ptr_, rows, 
        snippet_.table_col, snippet_.table_filter, snippet_.table_offset,
        snippet_.table_offlen, snippet_.table_datatype, snippet_.csd_name, row_offset_
    );

    EnQueueFilter(scanResult);
}


void Scan::EnQueueFilter(ScanResult scanResult_){
  //cout << "\n[3.Insert Scan Result Into FilterQueue]" << endl;
  // while(FilterQueue.length() > 50){
	//   sleep(1);
  // }
  FilterQueue.push_work(scanResult_);
}