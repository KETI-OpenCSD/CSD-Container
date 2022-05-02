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
#include "csd_table_manager.h"

extern WorkQueue<Snippet> ScanQueue;
extern WorkQueue<ScanResult> FilterQueue;

class Scan{
    public:
        Scan(TableManager table_m){
            CSDTableManager_ = table_m;
        }

        void Scanning();
        void BlockScan(SstBlockReader& sstBlockReader_, BlockInfo* blockInfo, Snippet &snippet_, int bc);
        void SendData(char* scan_buf, char* ptr_, int rows, int block_id_, 
                      int total_block_count_, Snippet snippet_, vector<int> row_offset_);
        void EnQueueFilter(ScanResult scanResult_);

    private:
        TableManager CSDTableManager_;
        int dev_fd;

};