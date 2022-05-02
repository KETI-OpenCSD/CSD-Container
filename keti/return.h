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
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

using namespace rapidjson;
using namespace std;

struct BlockBuffer;

class Return{
    public:
        Return(){}
        void SendDataToBufferManager(BlockBuffer mergedBlock);
};

struct message{
        long msg_type;
        char msg[2000];
};

struct BlockBuffer{
    int work_id;
    vector<tuple<int, int, int, bool>> block_id_list; 
    // block_id : block start offset : block size : is full block
    // block start offset이 -1이면 데이터 없음
    vector<int> row_offset; 
    int rows;
    //vector<char> data; //4k = 4096 찰때마다 전송
    char data[4096];
    int length;
    string csd_name;

    // BlockBuffer(int workid_, string csdname_){
    //     work_id = workid_;
    //     block_id_list.clear();
    //     row_offset.clear();
    //     rows = 0;
    //     length = 0;
    //     csd_name = csdname_;
    // }
};