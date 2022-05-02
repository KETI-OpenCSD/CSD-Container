#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <vector>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <atomic>

#include "rapidjson/document.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" 

#include "TableManager.h"
#include "CSDScheduler.h"
#include "keti_type.h"

#define PORT 8080
#define MAXLINE 256

using namespace rapidjson;
using namespace std;

void accept_connection(int server_fd);
int my_LBA2PBA(char* req,char* res);

TableManager tblManager;
Scheduler csdscheduler;

atomic<int> WorkID;

int main(int argc, char const *argv[])
{
	//init table manager	
	tblManager.init_TableManager();
	//init WorkID
	WorkID=0;

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
	
	thread(accept_connection, server_fd).detach();

	while (1);
	
	close(server_fd);

    //send(new_socket , test_buf , 1024 , 0 );
    //printf("Hello message sent\n");
    return 0;
}
void accept_connection(int server_fd){
	while (1) {
		int new_socket;
		struct sockaddr_in address;
		int addrlen = sizeof(address);
		char buffer[4096] = {0};

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
			(socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		
		read( new_socket , buffer, 4096);
		std::cout << buffer << std::endl;
		
		//parse json	
		Document document;
		document.Parse(buffer);
		
		Value &type = document["type"];

		switch(type.GetInt()){
			case KETI_WORK_TYPE::SCAN :{
				std::cout << "Do SCAN Pushdown" << std::endl;
				char req_json[4096];
				char res_json[4096];

				Value &table_name = document["table_name"];
				//gen req_json
				if( tblManager.generate_req_json(table_name.GetString(),req_json) == -1 ){
					int ret = -1;
					send(new_socket,&ret,sizeof(ret),0);
					break;
				}
				std::cout << req_json << std::endl;
				
				//do LBA2PBA
				my_LBA2PBA(req_json,res_json);
				std::cout << res_json << std::endl;
				
				//get table schema
				vector<TableManager::ColumnSchema> schema;
				tblManager.get_table_schema(table_name.GetString(),schema);
				
				vector<TableManager::ColumnSchema>::iterator itor = schema.begin();
				for(; itor != schema.end(); itor++){
					std::cout << "column_name : " << (*itor).column_name << " " << (*itor).type << " " << (*itor).length << " " << (*itor).offset << std::endl;
				}

				//after sched
				int tmp = WorkID.load();
				send(new_socket,&tmp,sizeof(tmp),0);
				std::cout << "WorkID : " << WorkID << std::endl;
				WorkID++;

				break;
			}
			case KETI_WORK_TYPE::SCAN_N_FILTER :{
				std::cout << "Do SCAN_N_FILTER Pushdown" << std::endl;
				char req_json[4096];
				char res_json[4096];

				Value &table_name = document["table_name"];
				//gen req_json
				tblManager.generate_req_json(table_name.GetString(),req_json);
				std::cout << req_json << std::endl;
				
				//do LBA2PBA
				my_LBA2PBA(req_json,res_json);
				std::cout << res_json << std::endl;
				Document reqdoc;
				reqdoc.Parse(req_json);
				string sstfilename = reqdoc["REQ"]["Chunk List"][0]["filename"].GetString();
				// cout << sstfilename << endl;
				Document blockdoc;
				blockdoc.Parse(res_json);
				Value &Blcokinfo = blockdoc["RES"]["Chunk List"][0][sstfilename.c_str()];
				Value &filter = document["Extra"]["filter"];
				
				//get table schema
				vector<TableManager::ColumnSchema> schema;
				tblManager.get_table_schema(table_name.GetString(),schema);
				vector<int> offset;
				vector<int> offlen;
				vector<int> datatype;
				vector<string> colname;
				
				vector<TableManager::ColumnSchema>::iterator itor = schema.begin();
				for(; itor != schema.end(); itor++){
					std::cout << "column_name : " << (*itor).column_name << " " << (*itor).type << " " << (*itor).length << " " << (*itor).offset << std::endl;
					offset.push_back((*itor).offset);
					offlen.push_back((*itor).length);
					datatype.push_back((*itor).type);
					colname.push_back((*itor).column_name);
				}
				csdscheduler.sched(WorkID,Blcokinfo,offset,offlen,datatype,colname,filter,sstfilename);

				//after sched
				int tmp = WorkID.load();
				send(new_socket,&tmp,sizeof(tmp),0);
				std::cout << "WorkID : " << WorkID << std::endl;
				WorkID++;

				break;
			}
			case KETI_WORK_TYPE::REQ_SCANED_BLOCK : {
				std::cout << "Return Merged data" << std::endl;
				
				//check workid
				Value &work_id = document["work_id"];
				std::cout << "WorkID : " << work_id.GetInt() << std::endl;

				char test_return_buf[4096]="test_row1test_row2test_row3";
				int test_n_rows=3;
				int test_offsets[3]={0,9,18};
				int test_buf_length=strlen(test_return_buf);
				int header[5];
				
				header[0] = test_n_rows;
				header[1] = test_offsets[0];
				header[2] = test_offsets[1];
				header[3] = test_offsets[2];
				header[4] = test_buf_length;

				send(new_socket,header,sizeof(header),0);
				send(new_socket,test_return_buf,test_buf_length,0);

				break;
			}
			default: {
				break;
			}
		}
		close(new_socket);
	}
}

int my_LBA2PBA(char* req,char* res){
	off64_t offset_buffer[128][3];
	int tbl_size;
	
	int i=0;
	// input
	off64_t req_offset;
	off64_t req_length;
	
	/*
	// read json from file
	int json_fd;
	char json[4096];
		
	memset(json,0,sizeof(json));
	json_fd = open("req_json.json",O_RDONLY);

	int res;

	
	while(1){
		res = read(json_fd,&json[i++],1);
		if(res == 0){
			break;
		}
	}
	close(json_fd);
	*/
	char *json = req;
	// print read json
	//printf("%s\n",json);

	//parse json	
	Document document;
	document.Parse(json);

	Document res_document;
	res_document.SetObject();
	rapidjson::Document::AllocatorType& allocator = res_document.GetAllocator();


	if(!document.HasMember("REQ")){
		printf("No REQ\n");
		return 1;
	}
	
	Value &REQObject = document["REQ"];


	if(!REQObject.IsObject()) {
		printf("REQ is No Object\n");
		return 1;
	}

	if(!REQObject.HasMember("Ordering")) {
		printf("No Ordering\n");
		return 1;
	}
	
	Value RES(kObjectType);
	Value str_val(kObjectType);

	str_val.SetString(REQObject["Ordering"].GetString(),static_cast<SizeType>(strlen(REQObject["Ordering"].GetString())),allocator);
	RES.AddMember("Ordering",str_val,allocator); // set res_json ordering

	Value &ChunkObjects = REQObject["Chunk List"];
	
	Value Chunk_List(kArrayType);
	for(i=0;i<ChunkObjects.Size();i++){
		Value &sstObject = ChunkObjects[i];		
		Value Chunk(kObjectType); // res chunk

		//check sst filename
		Value &filenameObject = sstObject["filename"];
		//print sst filename
		//std::cout << "filename : " << filenameObject.GetString() << std::endl;

		//do hdparm
		char cmd[256];
		sprintf(cmd,"hdparm --fibmap /usr/local/rocksdb/%s 2> /dev/null",filenameObject.GetString());

		char buf[MAXLINE];
		int flag = 0;
		FILE *fp=popen(cmd,"r");
		int j=0;
		while(fgets(buf, MAXLINE, fp) != NULL)
		{
			char *tok = strtok(buf," ");
			if(flag){ //set table
				//printf("byte_offset %s ",tok); 
				offset_buffer[j][0] = atoi(tok);
				tok = strtok(NULL, " "); //begin_LBA
				//printf("begin_LBA %s\n",tok);
				offset_buffer[j][1] = (off64_t)512 * atoi(tok);
				tok = strtok(NULL, " "); //end_LBA
				tok = strtok(NULL, " "); //sectors
				offset_buffer[j][2] = (off64_t)512 * atoi(tok);
				j++;
			}
			if(strcmp(tok,"byte_offset") == 0){
				flag = 1;
			}
		}
		tbl_size = j;
		pclose(fp);
		
		Value &ChunksObject = sstObject["Chunks"];
		Value Offset_List(kArrayType); // res offset list
		
		int seq_id = 1;
		int csd_id = 1; // currently fixed
		for(j=0;j<ChunksObject.Size();j++){
			Value &offsetObject = ChunksObject[j];
			
			//std::cout << "Offset : " << offsetObject["Offset"].GetInt64() << std::endl;
			//std::cout << "Length : " << offsetObject["Length"].GetInt() << std::endl;

			flag = 0;
			req_offset = offsetObject["Offset"].GetInt64();
			req_length = offsetObject["Length"].GetInt();
			for(int k=0;k<tbl_size;k++){
				Value Offset(kObjectType); // res offset 
				if(flag || req_offset >= offset_buffer[k][0] && req_offset < offset_buffer[k][0] + offset_buffer[k][2]){
					flag = 1;
					if(req_length > offset_buffer[k][2]){
						//printf("{\n\t\"SEQ ID\" : %d,\n\t\"CSD UD\" : %d,\n\t\"Offset\" : %ld,\n\t\"Length\" : %ld\n},\n",seq_id,csd_id,offset_buffer[k][1] + req_offset - offset_buffer[k][0],offset_buffer[k][2]);
						Offset.AddMember("SEQ ID",seq_id,allocator); //fill res offset
						Offset.AddMember("CSD ID",csd_id,allocator);
						Offset.AddMember("Offset",offset_buffer[k][1] + req_offset - offset_buffer[k][0],allocator);
						Offset.AddMember("Length",offset_buffer[k][2],allocator);
						Offset_List.PushBack(Offset,allocator); //push back res offset to offset list

						req_length -= offset_buffer[k][2];
						req_offset += offset_buffer[k][2];
					} else {
						//printf("{\n\t\"SEQ ID\" : %d,\n\t\"CSD UD\" : %d,\n\t\"Offset\" : %ld,\n\t\"Length\" : %ld\n},\n",seq_id,csd_id,offset_buffer[k][1] + req_offset - offset_buffer[k][0],req_length);
						Offset.AddMember("SEQ ID",seq_id++,allocator); //fill res offset
						Offset.AddMember("CSD ID",csd_id,allocator);
						Offset.AddMember("Offset",offset_buffer[k][1] + req_offset - offset_buffer[k][0],allocator);
						Offset.AddMember("Length",req_length,allocator);
						Offset_List.PushBack(Offset,allocator); //push back res offset to offset list
						break;
					}
				}
			}
		}
		// 
		Chunk.AddMember(filenameObject,Offset_List,allocator);
		Chunk_List.PushBack(Chunk,allocator);
	}
	RES.AddMember("Chunk List",Chunk_List,allocator); // set res_json ordering
	res_document.AddMember("RES",RES,allocator);
		
	// Convert JSON document to string
	rapidjson::StringBuffer strbuf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
	res_document.Accept(writer);

	strcpy(res,strbuf.GetString());
	//std::cout << strbuf.GetString() << std::endl;

	return 0;
}