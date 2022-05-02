#include "merge_manager.h"

#define PACKET_SIZE 36

void MergeManager::Merging(){
    // key_t key1=54321;
    // int msqid;
    // message msg;
    // msg.msg_type=1;
    // if((msqid=msgget(key1,IPC_CREAT|0666))==-1){
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    // printf("~MergeBlock~ # workid: %d, blockid: %d, rows: %d, length: %d, offset_len: %ld\n",result.work_id, result.block_id, result.rows, result.totallength, result.offset.size());
    // string rowfilter = "<-----------  Merge Manager Running...  ----------->";
    // strcpy(msg.msg,rowfilter.c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }

    while (1){
        FilterResult filterResult = MergeQueue.wait_and_pop();
        // string rowfilter = "<------------Merge Block------------>";
        // strcpy(msg.msg,rowfilter.c_str());
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }
                
        MergeBlock(filterResult);
    }
}

void MergeManager::MergeBlock(FilterResult &result){
    // cout << result.data << endl;
    // printf("~MergeBlock~ # workid: %d, blockid: %d, rows: %d, length: %d, offset_len: %ld\n",result.work_id, result.block_id, result.rows, result.totallength, result.offset.size());

    // key_t key1=54321;
    // int msqid;
    // message msg;
    // msg.msg_type=1;
    // if((msqid=msgget(key1,IPC_CREAT|0666))==-1){
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    // string rowfilter = "[Get Block Filter Result]\n";
    // rowfilter += "-------------------------------Filtered Block Info----------------------------------\n";
    // rowfilter += "| work id: " + to_string(result.work_id)+ " | block id: " + to_string(result.block_id) 
    //           + " | rows: " + to_string(result.rows) + " | filtered block length: " + to_string(result.totallength)
    //           + " | is last block: " + to_string(result.is_last_block) + " | \n";
    // rowfilter += "------------------------------------------------------------------------------------\n";
    
    // strcpy(msg.msg,rowfilter.c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
    
    // ostringstream oss;
    // for(int i = 0; i < result.totallength; i++){
    //     cout << hex << (int)result.data[i];
    // }

    // strcpy(msg.msg,oss.str().c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }

    // rowfilter = "------------------------------------------------------------------------------------\n";

    // strcpy(msg.msg,rowfilter.c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
	    
    int row_len = 0;
    int row_num = 0;
    int block_size = 0;
    int key = result.work_id;

    if(m_MergeManager.find(key)==m_MergeManager.end()){//Key에 해당하는 블록버퍼가 없다면 생성
        BlockBuffer block;
        block.length = 0;
        block.rows = 0;
        block.block_count = 0;
        block.work_id = key;
        block.csd_name = result.csd_name;
        block.total_block_count = result.total_block_count;
        m_MergeManager.insert(make_pair(key,block));
    } 

    int block_start_offset = m_MergeManager[key].length;
    
    if(result.rows == 0){//현재 블록 데이터가 모두 필터되었다면
        // string rowfilter = "**current filtered block {" + to_string(result.block_id) + "} length is 0\n";
        // strcpy(msg.msg,rowfilter.c_str());
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }
        m_MergeManager[key].block_id_list.push_back(make_tuple(result.block_id,-1,0,true));
                
        //return;
    }

    else{// 블록에 데이터가 있다면
        // rowfilter = "\n[Merge Filtered Block Rows...]\n";
        // strcpy(msg.msg,rowfilter.c_str());
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }

        //sleep(1); //10
        
        vector<int> temp_offset;
        temp_offset.assign( result.offset.begin(), result.offset.end() );
        temp_offset.push_back(result.totallength);

        //새 블록 넣기 전 확인
        row_len = temp_offset[1] - temp_offset[0];

        if(m_MergeManager[key].length + row_len > MAX_SIZE){//row추가시 데이터 크기 넘으면
            // rowfilter = "\n[Send Merged Block Data To Buffer Manager]";
            // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
            //     printf("msgsnd failed\n");
            //     exit(0);
            // }        
            SendDataToBufferManager(m_MergeManager[key]);//데이터 전송
            m_MergeManager[key].InitBlockBuffer();
        }

        //새 블록 넣기
        for(int i=0; i<result.rows; i++){
            row_len = temp_offset[i+1] - temp_offset[i];
            if(m_MergeManager[key].length + row_len > MAX_SIZE){//row추가시 데이터 크기 넘으면
                block_size = m_MergeManager[key].length - block_start_offset;
                m_MergeManager[key].block_id_list.push_back(make_tuple(result.block_id,block_start_offset,block_size,false));
                // rowfilter = "\n[Send Merged Block Data To Buffer Manager]";
                // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
                //     printf("msgsnd failed\n");
                //     exit(0);
                // }            
                SendDataToBufferManager(m_MergeManager[key]);//데이터 전송
                m_MergeManager[key].InitBlockBuffer();
            }

            m_MergeManager[key].row_offset.push_back(m_MergeManager[key].length); //현재 row 시작 offset
            m_MergeManager[key].rows += 1;
            int current_offset = m_MergeManager[key].length;
            for(int j = temp_offset[i]; j<temp_offset[i+1]; j++){
                m_MergeManager[key].data[current_offset] =  result.data[j]; // 현재 row데이터 복사
                current_offset += 1;
            }
            m_MergeManager[key].length += row_len;// 데이터 길이 = row 전체 길이
        }

        block_size = m_MergeManager[key].length - block_start_offset;
        m_MergeManager[key].block_id_list.push_back(make_tuple(result.block_id,block_start_offset,block_size,true));
        // printf("~~After One Block~~ # workid: %d, rows: %d, length: %d, offset_len: %ld, block_list_len: %ld\n",result.work_id, m_MergeManager[key].rows, m_MergeManager[key].length, m_MergeManager[key].row_offset.size(), m_MergeManager[key].block_id_list.size());

        // rowfilter =  "[Check Merge Buffer]\n";
        // rowfilter += "-------------------------------Merging Block Info----------------------------------\n";
        // rowfilter += "| work id: " + to_string(m_MergeManager[key].work_id)+ " | length: " + to_string(m_MergeManager[key].length) 
        //         + " | rows: " + to_string(m_MergeManager[key].rows) + " | \n";
        // for(int i=0; i<m_MergeManager[key].block_id_list.size(); i++){
        //     rowfilter += "block list[" + to_string(i) + "] : { " + to_string(get<0>(m_MergeManager[key].block_id_list[i])) + " : " 
        //     +  to_string(get<1>(m_MergeManager[key].block_id_list[i])) + " : " +  to_string(get<2>(m_MergeManager[key].block_id_list[i])) 
        //     + " : " +  to_string(get<3>(m_MergeManager[key].block_id_list[i])) + " }\n";
        // }
        // rowfilter += "------------------------------------------------------------------------------------\n";
        
        // strcpy(msg.msg,rowfilter.c_str());
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }
        
        // ostringstream osss;
        // for(int i = 0; i < m_MergeManager[key].length; i++){
        //     osss << hex << (int)m_MergeManager[key].data[i];
        // }

        // strcpy(msg.msg,osss.str().c_str());
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }

        // rowfilter = "------------------------------------------------------------------------------------\n";
        

        // strcpy(msg.msg,rowfilter.c_str());
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }
    }

    m_MergeManager[key].block_count += 1;

    if(m_MergeManager[key].total_block_count == m_MergeManager[key].block_count){//블록 병합이 끝나면
        // rowfilter = "\n[Send Merged Block Data To Buffer Manager]";
        // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }
        SendDataToBufferManager(m_MergeManager[key]);//데이터 전송
        m_MergeManager.erase(key);//key에 해당하는거 삭제
    }
    
}

void MergeManager::SendDataToBufferManager(BlockBuffer &mergedBlock){
    //mergeBlock을 json ?
    // printf("~~Send Data To BufferManager~~ # workid: %d, rows: %d, length: %d, offset_len: %ld, block_list_len: %ld\n",mergedBlock.work_id, mergedBlock.rows, mergedBlock.length, mergedBlock.row_offset.size(), mergedBlock.block_id_list.size());
    // key_t key1=54321;
    // int msqid;
    // message msg;
    // msg.msg_type=1;
    // if((msqid=msgget(key1,IPC_CREAT|0666))==-1){
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    // string rowfilter =  "[Send Merged Block Data To Buffer Manager]\n";
    // rowfilter += "-------------------------------Merged Block Info----------------------------------\n";
    // rowfilter += "| work id: " + to_string(mergedBlock.work_id)+ " | length: " + to_string(mergedBlock.length) 
    //           + " | rows: " + to_string(mergedBlock.rows) + " | \n";
    // for(int i=0; i<mergedBlock.block_id_list.size(); i++){
    //     rowfilter += "block list[" + to_string(i) + "] : { " + to_string(get<0>(mergedBlock.block_id_list[i])) + " : " 
    //     +  to_string(get<1>(mergedBlock.block_id_list[i])) + " : " +  to_string(get<2>(mergedBlock.block_id_list[i])) 
    //     + " : " +  to_string(get<3>(mergedBlock.block_id_list[i])) + " }\n";
    // }
    // rowfilter += "------------------------------------------------------------------------------------\n";
    
    // cout << "---------------Merged Block Info-----------------" << endl;
    // cout << "| work id: " << mergedBlock.work_id << " | length: " << mergedBlock.length
    //      << " | rows: " << mergedBlock.rows << " | " << "block list : ";
    // for(int i=0; i<mergedBlock.block_id_list.size(); i++){
    //     cout << "{" << get<0>(mergedBlock.block_id_list[i]) << " : " << get<3>(mergedBlock.block_id_list[i]) + "},";
    // }
    // cout << "\n----------------------------------------------\n";
    // for(int i = 0; i < mergedBlock.length; i++){
    //     printf("%02X",(u_char)mergedBlock.data[i]);
    // }
    // cout << "------------------------------------------------\n";

    // strcpy(msg.msg,rowfilter.c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
    
    // ostringstream osss;
    // for(int i = 0; i < mergedBlock.length; i++){
    //     osss << hex << (int)mergedBlock.data[i];
    // }

    // strcpy(msg.msg,osss.str().c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }

    // rowfilter = "------------------------------------------------------------------------------------\n";

    // strcpy(msg.msg,rowfilter.c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }

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

    int sockfd;
    struct sockaddr_in serv_addr;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("10.0.5.33");
    serv_addr.sin_port = htons(8888);

    connect(sockfd,(const sockaddr*)&serv_addr,sizeof(serv_addr));
	
	size_t len = strlen(block_buf_.c_str());
	send(sockfd, &len, sizeof(len), 0);
    send(sockfd, (char*)block_buf_.c_str(), len, 0);

    static char cBuffer[PACKET_SIZE];
    if (recv(sockfd, cBuffer, PACKET_SIZE, 0) == 0){
        cout << "client recv Error" << endl;
        return;
    };

    len = mergedBlock.length;
    send(sockfd,&len,sizeof(len),0);
    send(sockfd, mergedBlock.data, 4096, 0);
    // free(mergedBlock.data);

    // printf("~MergeBlock~ # workid: %d, blockid: %d, rows: %d, length: %d, offset_len: %ld\n",result.work_id, result.block_id, result.rows, result.totallength, result.offset.size());
    // rowfilter = "[Json Send To Buffer Manager]\n";
    // rowfilter += block_buf_;
    
    // strcpy(msg.msg,rowfilter.c_str());
    // if(msgsnd(msqid,&msg,sizeof(msg.msg),0)==-1){
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
    
    close(sockfd);
}