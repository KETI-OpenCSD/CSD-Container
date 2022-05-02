#include <queue>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <typeinfo>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sstream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rocksdb/sst_file_reader.h"

#include "input.h"
// #include "return.h"

using namespace std;
using namespace rapidjson;

#define MAX_SIZE 4096

// struct BlockBuffer;
struct FilterResult;

extern WorkQueue<FilterResult> MergeQueue;

struct BlockBuffer{
    int work_id;
    vector<tuple<int, int, int, bool>> block_id_list; 
    // block_id : block start offset : block size : is full block
    // block start offset이 -1이면 데이터 없음
    vector<int> row_offset; 
    int rows;
    char data[4096];//4k = 4096 찰때마다 전송
    int length;
    string csd_name;
    int block_count;
    int total_block_count;

    // BlockBuffer(int workid_, string csdname_){
    //     work_id = workid_;
    //     block_id_list.clear();
    //     row_offset.clear();
    //     rows = 0;
    //     length = 0;
    //     csd_name = csdname_;
    // }

    void InitBlockBuffer(){
        this->block_id_list.clear();
        this->row_offset.clear();
        memset(&this->data, 0, sizeof(MAX_SIZE));
        this->rows = 0;
        this->length = 0;
    }
};

struct message{
        long msg_type;
        char msg[10240];
};

class MergeManager{
public:
    // MergeManager(Return return_if){
    //     ReturnIF_ = return_if;
    // }
    MergeManager(){}
    void Merging();
    void MergeBlock(FilterResult &result);
    void SendDataToBufferManager(BlockBuffer &mergedBlock);

private:
    unordered_map<int, BlockBuffer> m_MergeManager;// key=workid
    // Return ReturnIF_;
};

struct FilterResult{
    int work_id;
    int block_id;
    int total_block_count;
    int rows;
    vector<int> offset;
    int totallength;
    char data[4096];
	char *ptr;
    string csd_name;

    FilterResult(
        int work_id, int block_id, int total_block_count_,
        int rows, int totallength, string csd_name_)
        : work_id(work_id), block_id(block_id), 
          total_block_count(total_block_count_),
          rows(rows), totallength(totallength),
          csd_name(csd_name_) {
			  ptr = data;
	}
        
};