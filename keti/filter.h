#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <queue>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>

#include "merge_manager.h"

using namespace std;
using namespace rapidjson;

struct ScanResult;
struct FilterResult;

extern WorkQueue<ScanResult> FilterQueue;
extern WorkQueue<FilterResult> MergeQueue;



class Filter{
public:
    Filter(){}

    int BlockFilter(ScanResult &scanResult);
    void Filtering();

    int row_offset;    
};

void SavedRow(char *row, int length, FilterResult &filterresult,int startlength, int nowlength);
vector<string> split(string str, char Delimiter);
bool LikeSubString(string lv, string rv);
bool LikeSubString_v2(string lv, string rv);
bool InOperator(string lv, Value& rv,unordered_map<string, int> typedata, unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, char *rowbuf);
bool InOperator(int lv, Value& rv,unordered_map<string, int> typedata, unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, char *rowbuf);
bool BetweenOperator(int lv, int rv1, int rv2);
bool BetweenOperator(string lv, string rv1, string rv2);
bool IsOperator(string lv, string rv, int isnot);
bool isvarc(vector<int> datatype, int ColNum);
void makedefaultmap(vector<string> ColName, vector<int> startoff, vector<int> offlen, vector<int> datatype, int ColNum, unordered_map<string, int> &startptr, unordered_map<string, int> &lengthRaw, unordered_map<string, int> &typedata);
void makenewmap(int isvarchar, int ColNum, unordered_map<string, int> &newstartptr, unordered_map<string, int> &newlengthraw, vector<int> datatype, unordered_map<string, int> lengthRaw, vector<string> ColName, int &iter, vector<int> startoff, vector<int> offlen, char *rowbuf);
void compareGE(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareGE(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareLE(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareLE(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareGT(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareGT(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareLT(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareLT(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareET(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareET(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareNE(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
void compareNE(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot);
int typeLittle(unordered_map<string, int> typedata, string colname, unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, char *rowbuf);
string ItoDec(int inum);
string typeBig(unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, string colname, char *rowbuf);
string typeDecimal(unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, string colname, char *rowbuf);
void sendfilterresult(FilterResult &filterresult);
// int Filtering(char *qwer, int row_count);

struct ScanResult
{
    int work_id;
    int block_id;
    int total_block_count;
    char scan_buf[4096];
    // char* scanptr;
	int buf_size;
    int rows;
    string table_filter;
    vector<string> table_col;
    vector<int> table_offset;
    vector<int> table_offlen;
    vector<int> table_datatype;
    string csd_name;
    vector<int> row_offset;

    ScanResult(
        int work_id_, int block_id_, int total_block_count_, char* scan_buf_, char* ptr_,
        int rows_, vector<string> table_col_, string table_filter_,
        vector<int> table_offset_, vector<int> table_offlen_,
        vector<int> table_datatype_, string csd_name_, vector<int> row_offset_)
        : work_id(work_id_), block_id(block_id_),
          total_block_count(total_block_count_), rows(rows_),
          table_col(table_col_),
          table_filter(table_filter_),
          table_offset(table_offset_),
          table_offlen(table_offlen_),
          table_datatype(table_datatype_),
          csd_name(csd_name_),
          row_offset(row_offset_) {
            int i;
            for(i=0; scan_buf_ + i < ptr_; i++){
                this->scan_buf[i] = scan_buf_[i];
            }
            buf_size=i;
	    };

    
};

enum opertype
{
    GE,      // >=
    LE,      // <=
    GT,      // >
    LT,      // <
    ET,      // ==
    NE,      // !=
    LIKE,    // RV로 스트링
    BETWEEN, // RV로 배열형식 [10,20] OR [COL1,20] --> extra
    IN,      // RV로 배열형식 [10,20,30,40] + 크기 --> extra
    IS,      // IS 와 IS NOT을 구분 RV는 무조건 NULL
    ISNOT,   // IS와 구분 필요 RV는 무조건 NULL
    NOT,     // ISNOT과 관련 없음 OPERATOR 앞에 붙는 형식 --> 혼자 들어오는 oper
    AND,     // AND --> 혼자 들어오는 oper
    OR,      // OR --> 혼자 들어오는 oper
};

