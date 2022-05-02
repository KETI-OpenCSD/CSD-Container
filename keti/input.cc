#include "input.h"

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define PORT 8080
#define BUFF_SIZE 4096

void Input::InputSnippet(){
	cout << "<-----------  Input Layer Running...  ----------->\n";

	int server_fd;
	int client_fd;
	int opt = 1;
	struct sockaddr_in serv_addr; // 소켓주소
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT); // port

	if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("bind");
		exit(EXIT_FAILURE);
	} // 소켓을 지정 주소와 포트에 바인딩

	if (listen(server_fd, 3) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	} // 리스닝

	while(1){
		if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen)) < 0){
			perror("accept");
        	exit(EXIT_FAILURE);
		}
		printf("Reading from client\n");

		std::string json;//크기?
		char buffer[BUFF_SIZE] = {0};

		//int read_size = read(client_fd , json, 4096);
		
		size_t length;
		read( client_fd , &length, sizeof(length));

		int numread;
		while(1) {
			if ((numread = read( client_fd , buffer, BUFF_SIZE - 1)) == -1) {
				perror("read");
				exit(1);
			}
			length -= numread;
		    buffer[numread] = '\0';
			json += buffer;

		    if (length == 0)
				break;
		}

		cout << "*******************Snippet JSON*****************" << endl;
		cout << json << endl;
		cout << "#json len: " << strlen(json.c_str()) << endl;
		cout << "#read_size: " << length << endl;
		cout << "************************************************" << endl;
		
		Snippet parsedSnippet(json.c_str());
		
		EnQueueScan(parsedSnippet);

		close(client_fd);
	}

	close(server_fd);
	
}

// void Input::InputSnippet(){
// 	//http로 json 받는 부분 해야함
// 	cout << "\n<-----------  Input Layer Running...  ----------->\n" << endl;
	
// 	char json[8000];
// 	int i=0;
	
// 	memset(json,0,sizeof(json));
// 	int json_fd = open("snippet.json",O_RDONLY);
	
// 	while(1){
// 		int res = read(json_fd,&json[i++],1);
// 		if(res == 0){
// 			break;
// 		}
// 	}
// 	close(json_fd);

// 	cout << "*******************Snippet JSON*****************" << endl;
//   	cout << json << endl;
//   	cout << "************************************************" << endl;

// 	Snippet parsedSnippet(json);
	
// 	EnQueueScan(parsedSnippet);
// }

void Input::EnQueueScan(Snippet parsedSnippet_){
	cout << "[Put Snippet to Scan Queue]" << endl;
	//sleep(5); // 1
	ScanQueue.push_work(parsedSnippet_);
}
