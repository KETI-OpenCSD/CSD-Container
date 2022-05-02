#include "return.h"

#define PACKET_SIZE 36

void Return::SendDataToBufferManager(BlockBuffer mergedBlock){
    //mergeBlock을 json ?
    // printf("~~Send Data To BufferManager~~ # workid: %d, rows: %d, length: %d, offset_len: %ld, block_list_len: %ld\n",mergedBlock.work_id, mergedBlock.rows, mergedBlock.length, mergedBlock.row_offset.size(), mergedBlock.block_id_list.size());
    key_t key1=54321;
    int msqid;
    message msg;
    msg.msg_type=1;
    if((msqid=msgget(key1,IPC_CREAT|0666))==-1){
        printf("msgget failed\n");
        exit(0);
    }
    string rowfilter =  "[Send Merged Block Data To Buffer Manager]\n";
    rowfilter += "-------------------------------Merged Block Info----------------------------------\n";
    rowfilter += "| work id: " + to_string(mergedBlock.work_id)+ " | length: " + to_string(mergedBlock.length) 
              + " | rows: " + to_string(mergedBlock.rows) + " | \n";
    for(int i=0; i<mergedBlock.block_id_list.size(); i++){
        rowfilter += "block list[" + to_string(i) + "] : { " + to_string(get<0>(mergedBlock.block_id_list[i])) + " : " 
        +  to_string(get<1>(mergedBlock.block_id_list[i])) + " : " +  to_string(get<2>(mergedBlock.block_id_list[i])) 
        + " : " +  to_string(get<3>(mergedBlock.block_id_list[i])) + " }\n";
    }
    rowfilter += "------------------------------------------------------------------------------------\n";
    
    strcpy(msg.msg,rowfilter.c_str());
    if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        printf("msgsnd failed\n");
        exit(0);
    }
    
    ostringstream osss;
    for(int i = 0; i < mergedBlock.length; i++){
        osss << hex << (int)mergedBlock.data[i];
    }

    strcpy(msg.msg,osss.str().c_str());
    if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        printf("msgsnd failed\n");
        exit(0);
    }

    rowfilter = "------------------------------------------------------------------------------------\n";

    strcpy(msg.msg,rowfilter.c_str());
    if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        printf("msgsnd failed\n");
        exit(0);
    }

    StringBuffer block_buf;
    PrettyWriter<StringBuffer> writer(block_buf);

    writer.StartObject();
    writer.Key("Work ID");
    writer.Int(mergedBlock.work_id);
    writer.Key("Block ID");
    writer.StartArray();

    for (int i = 0; i < mergedBlock.block_id_list.size(); i ++){
        writer.StartArray();
        writer.Int(get<0>(mergedBlock.block_id_list[i]));
        writer.Int(get<1>(mergedBlock.block_id_list[i]));
        writer.Int(get<2>(mergedBlock.block_id_list[i]));
        writer.Bool(get<3>(mergedBlock.block_id_list[i]));
        writer.EndArray();
    }
    
    writer.EndArray();
    writer.Key("nrows");
    writer.Int(mergedBlock.rows);
    writer.Key("Row Offset");
    writer.StartArray();
    for (int i = 0; i < mergedBlock.row_offset.size(); i ++){
        writer.Int(mergedBlock.row_offset[i]);
    }
    writer.EndArray();
    writer.Key("Length");
    writer.Int(mergedBlock.length);

    writer.Key("CSD Name");
    writer.String(mergedBlock.csd_name.c_str());

    writer.EndObject();

    string block_buf_ = block_buf.GetString();

    int sock;
    struct sockaddr_in serv_addr;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("10.0.5.33");
    serv_addr.sin_port = htons(8888);

    int len = sizeof(mergedBlock);
    connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    send(sock, (char*)block_buf_.c_str(), strlen(block_buf_.c_str()),0);

    static char cBuffer[PACKET_SIZE];
    if (recv(sock, cBuffer, PACKET_SIZE, 0) == 0){
        cout << "client recv Error" << endl;
        return;
    };

    // printf("~MergeBlock~ # workid: %d, blockid: %d, rows: %d, length: %d, offset_len: %ld\n",result.work_id, result.block_id, result.rows, result.totallength, result.offset.size());
    rowfilter = "[Json Send To Buffer Manager]\n";
    rowfilter += block_buf_;
    
    strcpy(msg.msg,rowfilter.c_str());
    if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        printf("msgsnd failed\n");
        exit(0);
    }
	    
    send(sock, mergedBlock.data, mergedBlock.length,0);

    close(sock);
}