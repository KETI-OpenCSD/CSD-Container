#include <queue>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rocksdb/sst_file_reader.h"

using namespace rapidjson;
using namespace std;
using namespace ROCKSDB_NAMESPACE;

struct Snippet;
template <typename T>
class WorkQueue;

extern WorkQueue<Snippet> ScanQueue;

class Input{
    public:
        Input(){}
        void InputSnippet();
        void EnQueueScan(Snippet snippet_);
};

struct Snippet{
    int work_id;
    string csd_name;
    string table_name;
    list<BlockInfo> block_info_list;
    vector<string> table_col;
    string table_filter;
    vector<int> table_offset;
    vector<int> table_offlen;
    vector<int> table_datatype;
    int total_block_count;
    
    Snippet(const char* json_){
        cout << "\n[Parsing Snippet]" << endl;	
    
        Document document;
        document.Parse(json_);
        
        work_id = document["WorkID"].GetInt();
        csd_name = document["CSD Name"].GetString();
        table_name = document["table_name"].GetString();

        Value &blockList = document["BlockList"];
        total_block_count = blockList.Size();
        for(int i = 0; i<total_block_count; i++){
            int block_id_ = blockList[i]["BlockID"].GetInt();
            uint64_t block_offset_ = blockList[i]["Offset"].GetUint64();
            uint64_t block_size_ = blockList[i]["Length"].GetUint64();
            BlockInfo newBlock(block_id_, block_offset_, block_size_);
            block_info_list.push_back(newBlock);
        }

        Value &table_col_ = document["table_col"];
        for(int j=0; j<table_col_.Size(); j++){
            string col = table_col_[j].GetString();
            int startoff = document["table_offset"][j].GetInt();
            int offlen = document["table_offlen"][j].GetInt();
            int datatype = document["table_datatype"][j].GetInt();
            table_col.push_back(col);
            table_offset.push_back(startoff);
            table_offlen.push_back(offlen);
            table_datatype.push_back(datatype);
        }

        Value &table_filter_ = document["table_filter"];
        Document small_document;
        small_document.SetObject();
        rapidjson::Document::AllocatorType& allocator = small_document.GetAllocator();
        small_document.AddMember("table_filter",table_filter_,allocator);
        StringBuffer strbuf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
	      small_document.Accept(writer);
        table_filter = strbuf.GetString();

    }
};

template <typename T>
class WorkQueue
{
  condition_variable work_available;
  mutex work_mutex;
  queue<T> work;

public:
  void push_work(T item){
    unique_lock<mutex> lock(work_mutex);

    bool was_empty = work.empty();
    work.push(item);

    lock.unlock();

    if (was_empty){
      work_available.notify_one();
    }    
  }

  T wait_and_pop(){
    unique_lock<mutex> lock(work_mutex);
    while (work.empty()){
      work_available.wait(lock);
    }

    T tmp = work.front();
    work.pop();
    return tmp;
  }

  int length(){
	  int ret;
	  unique_lock<mutex> lock(work_mutex);

    ret = work.size();

    lock.unlock();
	  return ret;
  }
};

    // int level_ = 0;
    // bool blocks_maybe_compressed_ = true;
    // bool blocks_definitely_zstd_compressed_ = false;
    // const bool immortal_table_ = false;
    // Slice cf_name_for_tracing_ = nullptr;
    // uint64_t sst_number_for_tracing_ = 0;
    // shared_ptr<Cache> to_block_cache_ = nullptr;//to_ = table_options
    // uint32_t to_read_amp_bytes_per_bit = 0;
    // //std::shared_ptr<const FilterPolicy> to_filter_policy = nullptr;
    // //std::shared_ptr<Cache> to_block_cache_compressed = nullptr;
    // bool to_cache_index_and_filter_blocks_ = false;
    // //ioptions
    // uint32_t footer_format_version_ = 5;//footer
    // int footer_checksum_type_ = 1;
    // uint8_t footer_block_trailer_size_ = 5;
    
    // //std::string dev_name = "/usr/local/rocksdb/000051.sst";
    // /*block info*/

    // std::string dev_name = "/dev/sda";
    // // const uint64_t handle_offset = 43673280512;
    // // const uint64_t block_size = 3995; //블록 사이즈 틀리지 않게!!