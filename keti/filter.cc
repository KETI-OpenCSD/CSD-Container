#include "filter.h"
#include <unistd.h>
#include <time.h>
// int count1 = 0;

void Filter::Filtering()
{
    // cout << "<-----------  Filter Layer Running...  ----------->\n";
    // key_t key = 12345;
    // int msqid;
    // message msg;
    // msg.msg_type = 1;
    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
    // {
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    while (1)
    {
        ScanResult scanResult = FilterQueue.wait_and_pop();
        // string rowfilter = "Filtering Using Filter Queue : Work ID " + to_string(scanResult.work_id) + " Block ID " + to_string(scanResult.block_id) + " Row Num " + to_string(scanResult.rows) + " Filter Json " + scanResult.table_filter;
        // strcpy(msg.msg, rowfilter.c_str());
        // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
        // {
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }

        // cout << " <------------Filter Block------------>" << endl;
        //  cout << "Filtering Using Filter Queue : Work ID " << scanResult.work_id << " Block ID " << scanResult.block_id <<" Row Num "<< scanResult.rows << " Filter Json " << scanResult.table_filter << endl;
        BlockFilter(scanResult);
    }
}

int Filter::BlockFilter(ScanResult &scanResult)
{
    cout << "\n----------------------------------------------\n";
    for(int i = 0; i < scanResult.buf_size; i++){
        printf("%02X",(u_char)scanResult.scan_buf[i]);
    }
    cout << "------------------------------------------------\n";
    // clock_t start = clock();
    // key_t key = 12345;
    // int msqid;
    // message msg;
    // msg.msg_type = 1;
    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
    // {
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    // string rowfilter = "---------- Block num : " + to_string(scanResult.block_id) + " ----------";
    // strcpy(msg.msg, rowfilter.c_str());
    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
    // {
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
	// cout << "filter data -----------------------------------------------------" << endl;
	// for(int i=0;i<4096;i++){
	// 	printf("%02X",scanResult.scan_buf[i]);
	// }

    // //strcpy(msg.msg, rowfilter.c_str());
    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
    // {
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
    // rowfilter = "Success Recived Scanning";
    // strcpy(msg.msg, rowfilter.c_str());
    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
    // {
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
    // if (count1 == 0){        
    //     count1++;
    // }

    char *rowbuf = scanResult.scan_buf;
    row_offset = 0;

    // for(int i = 0; i < 2000; i ++){
    //     cout << hex << (int)rowbuf[i] << endl;
    // }
    // string test(rowbuf);
    // cout << test << endl;
    // cout << scanResult.table_filter << endl;
    unordered_map<string, int> startptr;
    unordered_map<string, int> lengthRaw;
    unordered_map<string, int> typedata;
    unordered_map<string, int> newstartptr;
    unordered_map<string, int> newlengthraw;

    FilterResult filterresult(scanResult.work_id, scanResult.block_id, scanResult.total_block_count, 0, 0, scanResult.csd_name);

    int ColNum = scanResult.table_col.size(); //컬럼 넘버로 컬럼의 수를 의미(스니펫을 통해 받은 컬럼의 수)
    int RowNum = scanResult.rows;             //로우 넘버로 로우의 수를 의미(스캔에서 받은 로우의 수)

    vector<int> startoff = scanResult.table_offset;
    vector<int> offlen = scanResult.table_offlen;
    vector<int> datatype = scanResult.table_datatype;
    vector<string> ColName = scanResult.table_col; //스니펫을 통해 받은 각 컬럼의 이름이 저장되는 배열

    string str = scanResult.table_filter;
    Document document;
    document.Parse(str.c_str());
    // cout << scanResult.table_filter << endl;
    Value &filterarray = document["table_filter"];

    bool CV, TmpV;          // CV는 현재 연산의 결과, TmpV는 이전 연산 까지의 결과
    bool Passed;            // and조건 이전이 f일 경우 연산을 생략하는 함수
    bool isSaved, canSaved; // or을 통해 저장이 되었는지, and 또는 or에서 저장이 가능한지 를 나타내는 변수
    bool isnot;             //이전 not operator를 만낫는지에 대한 변수
    bool isvarchar = 0;     // varchar 형을 포함한 컬럼인지에 대한 변수

    isvarchar = isvarc(datatype, ColNum);
    makedefaultmap(ColName, startoff, offlen, datatype, ColNum, startptr, lengthRaw, typedata);

    int iter = 0; //각 row의 시작점
    for (int i = 0; i < RowNum; i++)
    {
        // clock_t start1 = clock();
        makenewmap(isvarchar, ColNum, newstartptr, newlengthraw, datatype, lengthRaw, ColName, iter, startoff, offlen, rowbuf);
        // clock_t finish1 = clock();
        // cout << (double)(finish1 - start1) / CLOCKS_PER_SEC << endl;
        TmpV = true;
        Passed = false;
        isSaved = false;
        canSaved = false;
        isnot = false;

        for (int j = 0; j < filterarray.Size(); j++)
        {
            switch (filterarray[j]["OPERATOR"].GetInt())
            {
            case GE:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    // string rowfilter = "*Pass Compare*";
                    // strcpy(msg.msg, rowfilter.c_str());
                    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                    // {
                    //     printf("msgget failed\n");
                    //     exit(0);
                    // }
                    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                    // {
                    //     printf("msgsnd failed\n");
                    //     exit(0);
                    // }
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {
                        // cout << filterarray[j]["LV"].GetString() << endl;                                                                                                // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            // cout << typedata[filterarray[j]["LV"].GetString()] << endl;
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetInt();
                            }
                            // string rowfilter = "LV : " + to_string(LV) + " >= RV : " + to_string(RV);
                            // strcpy(msg.msg, rowfilter.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                            compareGE(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                                {
                                    RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { // string과 int의 비교 시
                            }

                            compareGE(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 246)
                                {
                                    RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { //여기가 int를 데시멀로 바꾸는 부분
                                int tmpint;
                                tmpint = filterarray[j]["RV"].GetInt();
                                RV = ItoDec(tmpint);
                            }
                            compareGE(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type decimal" << endl;
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            string RV;
                            if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            {
                                RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            compareGE(LV, RV, CV, TmpV, canSaved, isnot);
                            // string lebetween = "LV : " + LV + " >=" + " RV : " + RV;
                            // // char tempstr = lebetween.c_str();
                            // strcpy(msg.msg, lebetween.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        int RV;
                        RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        compareGE(LV, RV, CV, TmpV, canSaved, isnot);
                        // cout << LV << " " << RV << endl;
                        // string lebetween = "LV : " + to_string(LV) + " >=" + " RV : " + to_string(RV);
                        // strcpy(msg.msg, lebetween.c_str());
                        // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                        // {
                        //     printf("msgget failed\n");
                        //     exit(0);
                        // }
                        // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                        // {
                        //     printf("msgsnd failed\n");
                        //     exit(0);
                        // }
                    }
                }
                /* code */
                break;
            case LE:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    // string rowfilter = "*Pass Compare*";
                    // strcpy(msg.msg, rowfilter.c_str());
                    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                    // {
                    //     printf("msgget failed\n");
                    //     exit(0);
                    // }
                    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                    // {
                    //     printf("msgsnd failed\n");
                    //     exit(0);
                    // }
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {                                                                                                            // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetInt();
                            }
                            compareLE(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetString();
                                RV = RV.substr(1);
                            }
                            compareLE(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 246)
                                {
                                    RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { //여기가 int를 데시멀로 바꾸는 부분
                                int tmpint;
                                tmpint = filterarray[j]["RV"].GetInt();
                                RV = ItoDec(tmpint);
                            }
                            compareLE(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type decimal" << endl;
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            string RV;
                            if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            {
                                RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            compareLE(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        int RV;
                        RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        compareLE(LV, RV, CV, TmpV, canSaved, isnot);
                        // string lebetween = "LV : " + to_string(LV) + " <=" + " RV : " + to_string(RV);
                        // strcpy(msg.msg, lebetween.c_str());
                        // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                        // {
                        //     printf("msgget failed\n");
                        //     exit(0);
                        // }
                        // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                        // {
                        //     printf("msgsnd failed\n");
                        //     exit(0);
                        // }
                    }
                }
                /* code */
                break;
            case GT:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    // string rowfilter = "*Pass Compare*";
                    // strcpy(msg.msg, rowfilter.c_str());
                    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                    // {
                    //     printf("msgget failed\n");
                    //     exit(0);
                    // }
                    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                    // {
                    //     printf("msgsnd failed\n");
                    //     exit(0);
                    // }
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {                                                                                                            // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetInt();
                            }
                            compareGT(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetString();
                                RV = RV.substr(1);
                            }
                            compareGT(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 246)
                                {
                                    RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { //여기가 int를 데시멀로 바꾸는 부분
                                int tmpint;
                                tmpint = filterarray[j]["RV"].GetInt();
                                RV = ItoDec(tmpint);
                            }
                            // string rowfilter = "LV : " + LV + " >" + " RV : " + RV;
                            // strcpy(msg.msg, rowfilter.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                            compareGT(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type decimal" << endl;
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            string RV;
                            if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            {
                                RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            compareGT(LV, RV, CV, TmpV, canSaved, isnot); //여기
                            // string lebetween = "LV : " + LV + " >" + " RV : " + RV;
                            // strcpy(msg.msg, lebetween.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        int RV;
                        RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        compareGT(LV, RV, CV, TmpV, canSaved, isnot);
                        // string rowfilter = "LV : " + to_string(LV) + " >" + " RV : " + to_string(RV);
                        // strcpy(msg.msg, rowfilter.c_str());
                        // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                        // {
                        //     printf("msgget failed\n");
                        //     exit(0);
                        // }
                        // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                        // {
                        //     printf("msgsnd failed\n");
                        //     exit(0);
                        // }
                    }
                }
                /* code */
                break;
            case LT:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    // string rowfilter = "*Pass Compare*";
                    // strcpy(msg.msg, rowfilter.c_str());
                    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                    // {
                    //     printf("msgget failed\n");
                    //     exit(0);
                    // }
                    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                    // {
                    //     printf("msgsnd failed\n");
                    //     exit(0);
                    // }
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {                                                                                                            // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            // cout << typedata[filterarray[j]["LV"].GetString()] << endl;
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetInt();
                            }
                            // cout << LV << " " << RV << endl;
                            // string rowfilter = "LV : " + to_string(LV) + " < RV : " + to_string(RV);
                            // strcpy(msg.msg, rowfilter.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                            compareLT(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetString();
                                RV = RV.substr(1);
                            }
                            compareLT(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 246)
                                {
                                    RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { //여기가 int를 데시멀로 바꾸는 부분
                                int tmpint;
                                tmpint = filterarray[j]["RV"].GetInt();
                                RV = ItoDec(tmpint);
                            }
                            // string rowfilter = "LV : " + LV + " < RV : " + RV;
                            // strcpy(msg.msg, rowfilter.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                            compareLT(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type decimal" << endl;
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            string RV;
                            if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            {
                                RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            // string rowfilter = "LV : " + LV + " <" + " RV : " + RV;
                            // strcpy(msg.msg, rowfilter.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                            compareLT(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        int RV;
                        RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        compareLT(LV, RV, CV, TmpV, canSaved, isnot);
                        // string lebetween = "LV : " + to_string(LV) + "<" + " RV : " + to_string(RV);
                        // strcpy(msg.msg, lebetween.c_str());
                        // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                        // {
                        //     printf("msgget failed\n");
                        //     exit(0);
                        // }
                        // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                        // {
                        //     printf("msgsnd failed\n");
                        //     exit(0);
                        // }
                    }
                }
                /* code */
                break;
            case ET:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    string rowfilter = "*Pass Compare*";
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {                                                                                                            // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetInt();
                            }
                            compareET(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetString();
                                RV = RV.substr(1);
                            }
                            compareET(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 246)
                                {
                                    RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { //여기가 int를 데시멀로 바꾸는 부분
                                int tmpint;
                                tmpint = filterarray[j]["RV"].GetInt();
                                RV = ItoDec(tmpint);
                            }
                            compareET(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type decimal" << endl;
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            string RV;
                            if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            {
                                RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            compareET(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        int RV;
                        RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        compareET(LV, RV, CV, TmpV, canSaved, isnot);
                    }
                }
                /* code */
                break;
            case NE:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {                                                                                                            // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetInt();
                            }
                            compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else
                            {
                                RV = filterarray[j]["RV"].GetString();
                                RV = RV.substr(1);
                            }
                            compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            if (filterarray[j]["RV"].GetType() == 5)
                            {
                                if (typedata[filterarray[j]["RV"].GetString()] == 246)
                                {
                                    RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                                }
                                else
                                {
                                    RV = filterarray[j]["RV"].GetString();
                                    RV = RV.substr(1);
                                }
                            }
                            else
                            { //여기가 int를 데시멀로 바꾸는 부분
                                int tmpint;
                                tmpint = filterarray[j]["RV"].GetInt();
                                RV = ItoDec(tmpint);
                            }
                            compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                            // cout << "type decimal" << endl;
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            string RV;
                            if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            {
                                RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            {
                                RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                            }
                            compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        int RV;
                        RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                    }
                }
                /* code */
                break;
            case LIKE:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    //  cout << "isnot print :" << isnot << " value : " << LV << endl;
                }
                else
                {
                    string RV;
                    string LV;
                    if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                    {
                        int tmplv;
                        tmplv = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        LV = to_string(tmplv);
                        // cout << "type little" << endl;
                        //나중 다른 데이트 처리를 위한 구분
                    }

                    else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                    {
                        LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                        // cout << "type big" << endl;
                    }
                    else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                    {
                        LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                        // cout << "type decimal" << endl;
                    }
                    else
                    {
                        LV = filterarray[j]["LV"].GetString();
                        LV = LV.substr(1);
                    }
                    if (typedata[filterarray[j]["RV"].GetString()] == 3 || typedata[filterarray[j]["RV"].GetString()] == 14) //리틀에디안
                    {
                        int tmprv;
                        tmprv = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf);
                        RV = to_string(tmprv);
                        // cout << "type little" << endl;
                        //나중 다른 데이트 처리를 위한 구분
                    }

                    else if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15) //빅에디안
                    {
                        RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                        // cout << "type big" << endl;
                    }
                    else if (typedata[filterarray[j]["RV"].GetString()] == 246) //예외 Decimal일때
                    {
                        RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf);
                        // cout << "type decimal" << endl;
                    }
                    else
                    {
                        RV = filterarray[j]["RV"].GetString();
                        RV = RV.substr(1);
                    }
                    CV = LikeSubString_v2(LV, RV);
                    // cout << CV << endl;
                }
                // cout << "isnot print :" << isnot << " value : " << LV << endl;
                // cout << isnot << endl;
                if (isnot)
                {
                    if (CV)
                    {
                        CV = false;
                        canSaved = false;
                    }
                    else
                    {
                        CV = true;
                        canSaved = true;
                    }
                }
                /* code */
                break;
            case BETWEEN:
                if (Passed)
                {
                    // cout << "*Row Filtered*" << endl;
                    // string rowfilter = "*Pass Compare*";
                    // strcpy(msg.msg, rowfilter.c_str());
                    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                    // {
                    //     printf("msgget failed\n");
                    //     exit(0);
                    // }
                    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                    // {
                    //     printf("msgsnd failed\n");
                    //     exit(0);
                    // }
                }
                else
                {
                    // cout << typedata[filterarray[j]["LV"].GetString()] << endl;
                    // cout << filterarray[j]["EXTRA"][0].GetString() << endl;
                    //  cout << filterarray[j]["LV"].GetString() << endl;
                    // cout << j << endl;
                    //  cout << filterarray[j]["LV"].GetType() << endl;
                    if (filterarray[j]["LV"].GetType() == 5)
                    {
                        // cout << "type : 5" << endl;
                        // string filtersring = filterarray[j]["LV"].GetString();
                        // cout << typedata[filtersring] << endl;
                        // cout << typedata[filterarray[j]["LV"].GetString()] << endl;                                                                                                  // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            int RV;
                            int RV1;
                            for (int k = 0; k < filterarray[j]["EXTRA"].Size(); k++)
                            {
                                if (filterarray[j]["EXTRA"][k].GetType() == 5)
                                { //컬럼명 또는 스트링이다. --> 스트링을 int로 변경, 만약 변경 불가한 문자의 경우 ex. 'asd' 예외처리해서 걍 f로 반환
                                    if (typedata[filterarray[j]["EXTRA"][k].GetString()] == 3 || typedata[filterarray[j]["EXTRA"][k].GetString()] == 14)
                                    {
                                        if (k == 0)
                                        {
                                            RV = typeLittle(typedata, filterarray[j]["EXTRA"][k].GetString(), newlengthraw, newstartptr, rowbuf);
                                        }
                                        else
                                        {
                                            RV1 = typeLittle(typedata, filterarray[j]["EXTRA"][k].GetString(), newlengthraw, newstartptr, rowbuf);
                                        }
                                    }
                                    else
                                    { //스트링이다 --> 변환 가능한가
                                        if (k == 0)
                                        {
                                            try
                                            {
                                                RV = stoi(filterarray[j]["EXTRA"][k].GetString());
                                            }
                                            catch (...)
                                            {
                                                CV = true; //수정 필요
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            try
                                            {
                                                RV1 = stoi(filterarray[j]["EXTRA"][k].GetString());
                                            }
                                            catch (...)
                                            {
                                                CV = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                                else if (filterarray[j]["EXTRA"][k].GetType() == 6) // int,float 타입
                                {                                                   // int, float, double
                                    if (filterarray[j]["EXTRA"][k].IsInt())
                                    {
                                        if (k == 0)
                                        {
                                            RV = filterarray[j]["EXTRA"][k].GetInt();
                                        }
                                        else
                                        {
                                            RV1 = filterarray[j]["EXTRA"][k].GetInt();
                                        }
                                    }
                                    else
                                    { // float일 경우는 없음 --> 스트링으로 들어오기 때문에
                                        float RV = filterarray[j]["EXTRA"][k].GetFloat();
                                    }
                                }
                            }
                            CV = BetweenOperator(LV, RV, RV1);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {

                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            string RV1;
                            for (int k = 0; k < filterarray[j]["EXTRA"].Size(); k++)
                            {
                                if (filterarray[j]["EXTRA"][k].GetType() == 5)
                                { //컬럼명 또는 스트링이다. --> 스트링이다 == float가 decimal로 800000000으로 들어온다
                                    if (typedata[filterarray[j]["EXTRA"][k].GetString()] == 3 || typedata[filterarray[j]["EXTRA"][k].GetString()] == 14)
                                    {
                                        if (k == 0)
                                        {
                                            RV = typeBig(newlengthraw, newstartptr, filterarray[j]["EXTRA"][k].GetString(), rowbuf);
                                        }
                                        else
                                        {
                                            RV1 = typeBig(newlengthraw, newstartptr, filterarray[j]["EXTRA"][k].GetString(), rowbuf);
                                        }
                                    }
                                    else
                                    { //스트링이다 --> 변환 가능한가
                                        if (k == 0)
                                        {
                                            try
                                            {
                                                RV = filterarray[j]["EXTRA"][k].GetString();
                                                RV = RV.substr(1);
                                            }
                                            catch (...)
                                            {
                                                CV = true; //수정 필요
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            try
                                            {
                                                RV1 = filterarray[j]["EXTRA"][k].GetString();
                                                RV1 = RV.substr(1);
                                            }
                                            catch (...)
                                            {
                                                CV = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                                else if (filterarray[j]["EXTRA"][k].GetType() == 6) // int,float 타입 이 부분도 수정필요 string과 int의 비교
                                {                                                   // int, float, double
                                    int tmpint;
                                    if (filterarray[j]["EXTRA"][k].IsInt())
                                    {
                                        if (k == 0)
                                        {
                                            tmpint = filterarray[j]["EXTRA"][k].GetInt();
                                            RV = to_string(tmpint);
                                        }
                                        else
                                        {
                                            tmpint = filterarray[j]["EXTRA"][k].GetInt();
                                            RV1 = to_string(tmpint);
                                        }
                                    }
                                    else
                                    { // float일 경우는 없음 --> 스트링으로 들어오기 때문에
                                      // float RV = filterarray[j]["EXTRA"][k].GetFloat();
                                    }
                                }
                            }
                            CV = BetweenOperator(LV, RV, RV1);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            // cout << "type : 246" << endl;
                            // cout << typedata[filterarray[j]["LV"].GetString()] << endl;
                            // cout << "j : " << j << endl;
                            // cout << "246" << j << endl;
                            // cout << filtersring << endl;
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            string RV;
                            string RV1;
                            // cout << LV << endl;
                            for (int k = 0; k < filterarray[j]["EXTRA"].Size(); k++)
                            {
                                // cout << filterarray[j]["EXTRA"][k].GetType() << endl;
                                if (filterarray[j]["EXTRA"][k].GetType() == 5)
                                { //컬럼명 또는 스트링이다. --> 스트링이다 == float가 decimal로 800000000으로 들어온다
                                    if (typedata[filterarray[j]["EXTRA"][k].GetString()] == 3 || typedata[filterarray[j]["EXTRA"][k].GetString()] == 14)
                                    {
                                        if (k == 0)
                                        {
                                            RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["EXTRA"][k].GetString(), rowbuf);
                                        }
                                        else
                                        {
                                            RV1 = typeDecimal(newlengthraw, newstartptr, filterarray[j]["EXTRA"][k].GetString(), rowbuf);
                                        }
                                    }
                                    else
                                    { //스트링이다 --> 변환 가능한가
                                        if (k == 0)
                                        {
                                            try
                                            {
                                                RV = filterarray[j]["EXTRA"][k].GetString();
                                                RV = RV.substr(1);
                                            }
                                            catch (...)
                                            {
                                                CV = true; //수정 필요
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            try
                                            {
                                                RV1 = filterarray[j]["EXTRA"][k].GetString();
                                                RV1 = RV1.substr(1);
                                            }
                                            catch (...)
                                            {
                                                CV = true;
                                                break;
                                            }
                                        }
                                    }
                                    // cout << "LV : " << LV << "RV : " << RV << "RV1 : " << RV1 << endl;
                                }
                                else if (filterarray[j]["EXTRA"][k].GetType() == 6) // int,float 타입 이 부분도 수정필요 string과 int의 비교
                                {                                                   // int, float, double
                                    int tmpint;
                                    if (filterarray[j]["EXTRA"][k].IsInt())
                                    {
                                        if (k == 0)
                                        {
                                            tmpint = filterarray[j]["EXTRA"][k].GetInt();
                                            RV = to_string(tmpint);
                                        }
                                        else
                                        {
                                            tmpint = filterarray[j]["EXTRA"][k].GetInt();
                                            RV1 = to_string(tmpint);
                                        }
                                    }
                                    else
                                    { // float일 경우는 없음 --> 스트링으로 들어오기 때문에
                                      // float RV = filterarray[j]["EXTRA"][k].GetFloat();
                                    }
                                }
                            }
                            // string betweenret = "LV : " + LV + " BETWEEN " + "RV : " + RV + " RV1 : " + RV1;
                            // strcpy(msg.msg, betweenret.c_str());
                            // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
                            // {
                            //     printf("msgget failed\n");
                            //     exit(0);
                            // }
                            // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
                            // {
                            //     printf("msgsnd failed\n");
                            //     exit(0);
                            // }
                            CV = BetweenOperator(LV, RV, RV1);
                        }
                        else
                        { // lv가 데시멀일때
                            // cout << filterarray[j]["LV"].GetString() << endl;
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            // string RV;
                            // if (typedata[filterarray[j]["RV"].GetString()] == 254 || typedata[filterarray[j]["RV"].GetString()] == 15)
                            // {
                            //     RV = typeBig(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf, lvtype);
                            // }
                            // else if (typedata[filterarray[j]["RV"].GetString()] == 246)
                            // {
                            //     RV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["RV"].GetString(), rowbuf, lvtype);
                            // }
                            // compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        // int RV;
                        // RV = typeLittle(typedata, filterarray[j]["RV"].GetString(), newlengthraw, newstartptr, rowbuf, lvtype);
                        // compareNE(LV, RV, CV, TmpV, canSaved, isnot);
                    }
                }
                if (isnot)
                {
                    if (CV)
                    {
                        CV = false;
                        canSaved = false;
                    }
                    else
                    {
                        CV = true;
                        canSaved = true;
                    }
                }
                /* code */
                break;
            case IN: //고민이 좀 필요한 부분 만약 데이터타입이 다 맞춰서 들어온다면?
                /* code */
                if (Passed)
                {
                    cout << "*Row Filtered*" << endl;
                    break;
                }
                else
                {
                    if (filterarray[j]["LV"].GetType() == 5)
                    {                                                                                                            // 6은 스트링 --> 스트링이다는 컬럼이름이거나 char이거나 decimal이다
                        if (typedata[filterarray[j]["LV"].GetString()] == 3 || typedata[filterarray[j]["LV"].GetString()] == 14) //리틀에디안
                        {
                            int LV = typeLittle(typedata, filterarray[j]["LV"].GetString(), newlengthraw, newstartptr, rowbuf);
                            Value &Extra = filterarray[j]["EXTRA"];
                            CV = InOperator(LV, Extra, typedata, newlengthraw, newstartptr, rowbuf);
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 254 || typedata[filterarray[j]["LV"].GetString()] == 15) //빅에디안
                        {
                            string LV = typeBig(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            Value &Extra = filterarray[j]["EXTRA"];
                            // Extra = filterarray[j]["EXTRA"].GetArray();
                            CV = InOperator(LV, Extra, typedata, newlengthraw, newstartptr, rowbuf);
                            // cout << "type big" << endl;
                        }
                        else if (typedata[filterarray[j]["LV"].GetString()] == 246) //예외 Decimal일때
                        {
                            string LV = typeDecimal(newlengthraw, newstartptr, filterarray[j]["LV"].GetString(), rowbuf);
                            Value &Extra = filterarray[j]["EXTRA"];
                            CV = InOperator(LV, Extra, typedata, newlengthraw, newstartptr, rowbuf);
                        }
                        else
                        {
                            string LV = filterarray[j]["LV"].GetString();
                            LV = LV.substr(1);
                            Value &Extra = filterarray[j]["EXTRA"];
                            CV = InOperator(LV, Extra, typedata, newlengthraw, newstartptr, rowbuf);
                        }
                    }
                    else
                    { // lv는 인트타입의 상수
                        int LV = filterarray[j]["LV"].GetInt();
                        Value &Extra = filterarray[j]["EXTRA"];
                        CV = InOperator(LV, Extra, typedata, newlengthraw, newstartptr, rowbuf);
                    }
                }
                break;
            case IS: // NULL형식에 대한 확인 필요
                /* code */
                break;
            case ISNOT:
                /* code */
                break;
            case NOT:
                if (Passed)
                {
                    break;
                }
                else if (isnot)
                {
                    isnot = false;
                    // j++;
                }
                else
                {
                    isnot = true;
                    // j++;
                }
                /* code */
                break;
            case AND:
                isnot = false;
                if (CV == false)
                { // f and t or t and t
                    Passed = true;
                }
                else
                {
                    TmpV = CV;
                    // PrevOper = 1;
                }
                /* code */
                break;
            case OR:
                isnot = false;
                if (CV == true)
                {
                    isSaved = true;
                    // cout << "Saved or" << endl;
                    // SavedRow(Rawrowdata[i]);
                }
                else
                {
                    TmpV = true;
                    // PrevOper = 0;
                    Passed = false;
                }
                /* code */
                break;
            default:
                cout << "error this is no default" << endl;
                break;
            }
            // cout << CV << endl;
            if (isSaved == true)
            { // or을 통해 저장되었다면
                char *ptr = rowbuf;
                char *tmpsave = new char[newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2];
                memcpy(tmpsave, ptr + newstartptr[ColName[0]] - 2, newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2);
                SavedRow(tmpsave, row_offset, filterresult, newstartptr[ColName[0]] - 2, newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2);
                row_offset += newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2;
                // delete[] tmpsave;
				free(tmpsave);
                break;
            }
        }

        // }
        if (canSaved == true && isSaved == false && Passed != true && CV == true)
        { // and를 통해 저장된다면
            // cout << "*Save Row*" << endl;
            char *ptr = rowbuf;
            char *tmpsave = new char[newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2];
            memcpy(tmpsave, ptr + newstartptr[ColName[0]] - 2, newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2);
            SavedRow(tmpsave, row_offset, filterresult, newstartptr[ColName[0]] - 2, newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2);
            row_offset += newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]] - newstartptr[ColName[0]] + 2;
			free(tmpsave);
            // delete[] tmpsave;
            // cout << tmpsave[0] << endl;
            //  cout << Rawrowdata[i] << endl;
        }
        // string lebetween = "*Row End";
        // strcpy(msg.msg, lebetween.c_str());
        // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
        // {
        //     printf("msgget failed\n");
        //     exit(0);
        // }
        // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
        // {
        //     printf("msgsnd failed\n");
        //     exit(0);
        // }
    }
    // std::cout << "  ------" << std::endl;
    // clock_t finish = clock();
    // cout << "BLOCK FILTER TIME" << endl;
    // cout << (double)(finish - start) / CLOCKS_PER_SEC << endl;
    // cout << "-------------------------------------------------------" << endl;

    sendfilterresult(filterresult);
    return 0;
}

void sendfilterresult(FilterResult &filterresult_)
{
    // string abc(filterresult_.data);
    // cout << abc << endl;
    // printf("~~sendfilterresult~~ length: %d",filterresult_.totallength);
    // key_t key = 12345;
    // int msqid;
    // message msg;
    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
    // {
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    // cout << "filter" << endl;
    // ostringstream oss;
    // for (int i = 0; filterresult_.data + i < filterresult_.ptr; i++)
    // {
    //     cout << hex << (int)filterresult_.data[i];
    // }
    // for (int i = 0; i < 50; i ++){
    //     cout << hex << (int)filterresult_.data[i];
    // }
    // cout << filterresult_.data << endl;
    // string lebetween = "After Filter Result : \nRows : " + to_string(filterresult_.rows) + "\nRow Data : \n" + oss.str();
    // strcpy(msg.msg, lebetween.c_str());
    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
    // {
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }

    // cout << "\n----------------------------------------------\n";
    // for(int i = 0; i < filterresult_.totallength; i++){
    //     printf("%02X",(u_char)filterresult_.data[i]);
    // }
    // cout << "------------------------------------------------\n";
    
    MergeQueue.push_work(filterresult_);
}

bool LikeSubString(string lv, string rv)
{ // case 0, 1, 2, 3, 4 --> %sub%(문자열 전체) or %sub(맨 뒤 문자열) or sub%(맨 앞 문자열) or sub(똑같은지) or %s%u%b%(생각 필요)
    // 해당 문자열 포함 검색 * 또는 % 존재 a like 'asd'
    int len = rv.length();
    int LvLen = lv.length();
    std::string val;
    if (rv[0] == '%' && rv[len - 1] == '%')
    {
        // case 0
        val = rv.substr(1, len - 2);
        for (int i = 0; i < LvLen - len + 1; i++)
        {
            if (lv.substr(i, val.length()) == val)
            {
                return true;
            }
        }
    }
    else if (rv[0] == '%')
    {
        // case 1
        val = rv.substr(1, len - 1);
        if (lv.substr(lv.length() - val.length() - 1, val.length()) == val)
        {
            return true;
        }
    }
    else if (rv[len - 1] == '%')
    {
        // case 2
        val = rv.substr(0, len - 1);
        if (lv.substr(0, val.length()) == val)
        {
            return true;
        }
    }
    else
    {
        // case 3
        if (rv == lv)
        {
            return true;
        }
    }
    return false;
}

bool LikeSubString_v2(string lv, string rv)
{ // % 위치 찾기
    // 해당 문자열 포함 검색 * 또는 % 존재 a like 'asd'
    int len = rv.length();
    int LvLen = lv.length();
    int i = 0, j = 0;
    int substringsize = 0;
    bool isfirst = false, islast = false; // %가 맨 앞 또는 맨 뒤에 있는지에 대한 변수
    // cout << rv[0] << endl;
    if (rv[0] == '%')
    {
        isfirst = true;
    }
    if (rv[len - 1] == '%')
    {
        islast = true;
    }
    vector<string> val = split(rv, '%');
    // for (int k = 0; k < val.size(); k++){
    //     cout << val[k] << endl;
    // }
    // for(int k = 0; k < val.size(); k ++){
    //     cout << val[k] << endl;
    // }
    if (isfirst)
    {
        i = 1;
    }
    // cout << LvLen << " " << val[val.size() - 1].length() << endl;
    // cout << LvLen - val[val.size() - 1].length() << endl;
    for (i; i < val.size(); i++)
    {
        // cout << "print i : " << i << endl;

        for (j; j < LvLen - val[val.size() - 1].length() + 1; j++)
        { // 17까지 돌아야함 lvlen = 19 = 17
            // cout << "print j : " << j << endl;
            substringsize = val[i].length();
            if (!isfirst)
            {

                if (lv.substr(0, substringsize) != val[i])
                {
                    // cout << "111111" << endl;
                    return false;
                }
            }
            if (!islast)
            {

                if (lv.substr(LvLen - val[val.size() - 1].length(), val[val.size() - 1].length()) != val[val.size() - 1])
                {
                    // cout << lv.substr(LvLen - val[val.size()-1].length() + 1, val[val.size()-1].length()) << " " << val[val.size()-1] << endl;
                    // cout << "222222" << endl;
                    return false;
                }
            }
            if (lv.substr(j, val[i].length()) == val[i])
            {
                // cout << lv.substr(j,val[i].length()) << endl;
                if (i == val.size() - 1)
                {
                    // cout << lv.substr(j, val[i].length()) << " " << val[i] << endl;
                    return true;
                }
                else
                {
                    j = j + val[i].length();
                    i++;
                    continue;
                }
            }
        }
        return false;
    }

    return false;
}

bool InOperator(string lv, Value &rv, unordered_map<string, int> typedata, unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, char *rowbuf)
{
    // 여러 상수 or 연산 ex) a IN (50,60) == a = 50 or a = 60
    for (int i = 0; i < rv.Size(); i++)
    {
        string RV = "";
        if (rv[i].IsString())
        {
            if (typedata[rv[i].GetString()] == 3 || typedata[rv[i].GetString()] == 14) //리틀에디안
            {
                int tmp = typeLittle(typedata, rv[i].GetString(), newlengthraw, newstartptr, rowbuf);
                // cout << "type little" << endl;
                //나중 다른 데이트 처리를 위한 구분
                RV = ItoDec(tmp);
            }

            else if (typedata[rv[i].GetString()] == 254 || typedata[rv[i].GetString()] == 15) //빅에디안
            {
                RV = typeBig(newlengthraw, newstartptr, rv[i].GetString(), rowbuf);
                // cout << "type big" << endl;
            }
            else if (typedata[rv[i].GetString()] == 246) //예외 Decimal일때
            {
                RV = typeDecimal(newlengthraw, newstartptr, rv[i].GetString(), rowbuf);
                // cout << "type decimal" << endl;
            }
            else
            {
                string tmps;
                tmps = rv[i].GetString();
                RV = tmps.substr(1);
            }
        }
        else //걍 int면?
        {
            RV = ItoDec(rv[i].GetInt());
        }
        if (lv == RV)
        {
            return true;
        }
    }
    return false;
}
bool InOperator(int lv, Value &rv, unordered_map<string, int> typedata, unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, char *rowbuf)
{
    for (int i = 0; i < rv.Size(); i++)
    {
        if (rv[i].IsString())
        {
            if (typedata[rv[i].GetString()] == 3 || typedata[rv[i].GetString()] == 14) //리틀에디안
            {
                int RV = typeLittle(typedata, rv[i].GetString(), newlengthraw, newstartptr, rowbuf);
                // cout << "type little" << endl;
                //나중 다른 데이트 처리를 위한 구분
                if (lv == RV)
                {
                    return true;
                }
            }

            else if (typedata[rv[i].GetString()] == 254 || typedata[rv[i].GetString()] == 15) //빅에디안
            {
                string RV = typeBig(newlengthraw, newstartptr, rv[i].GetString(), rowbuf);
                // cout << "type big" << endl;
                try
                {
                    if (lv == atoi(RV.c_str()))
                    {
                        return true;
                    }
                }
                catch (...)
                {
                    continue;
                }
            }
            else if (typedata[rv[i].GetString()] == 246) //예외 Decimal일때
            {
                string RV = typeDecimal(newlengthraw, newstartptr, rv[i].GetString(), rowbuf);
                // cout << "type decimal" << endl;

                if (ItoDec(lv) == RV)
                {
                    return true;
                }
            }
            else
            {
                string tmps;
                int RV;
                tmps = rv[i].GetString();
                try
                {
                    RV = atoi(tmps.substr(1).c_str());
                }
                catch (...)
                {
                    continue;
                }
            }
        }
        else // int or string
        {
            int RV = rv[i].GetInt();
            if (lv == RV)
            {
                return true;
            }
        }
    }
    return false;
}

bool BetweenOperator(int lv, int rv1, int rv2)
{
    // a between 10 and 20 == a >= 10 and a <= 20
    if (lv >= rv1 && lv <= rv2)
    {
        return true;
    }
    return false;
}

bool BetweenOperator(string lv, string rv1, string rv2)
{
    // a between 10 and 20 == a >= 10 and a <= 20
    if (lv >= rv1 && lv <= rv2)
    {
        return true;
    }
    return false;
}

bool IsOperator(string lv, string rv, int isnot)
{
    // a is null or a is not null
    if (lv.empty())
    {
        if (isnot == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    if (isnot == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void SavedRow(char *row, int length, FilterResult &filterresult, int startlength, int nowlength)
{
    // cout << "[Saved Row(HEX)] VALUE: ";
    //  for (int k = 0; k < length; k++)
    //  {
    //      cout << hex << (int)row[k];
    //  }
    //  cout << endl;
    cout << "saved row" << endl;
    filterresult.rows++;
    filterresult.offset.push_back(length);
    filterresult.totallength += nowlength;
    for (int i = 0; i < nowlength; i++)
    {
        *filterresult.ptr++ = row[i];
    }
    // key_t key = 12345;
    // int msqid;
    // message msg;
    // msg.msg_type = 1;
    // if ((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
    // {
    //     printf("msgget failed\n");
    //     exit(0);
    // }
    // string saveret = "*Saved Row";
    // strcpy(msg.msg, saveret.c_str());
    // if (msgsnd(msqid, &msg, sizeof(msg.msg), 0) == -1)
    // {
    //     printf("msgsnd failed\n");
    //     exit(0);
    // }
    // writefile.write(saveret.c_str(),saveret.size());

    // printf("~~SavedRow~~ # blockid: %d, rows: %d, length: %d, offset_len: %ld, data_len: %ld",filterresult.block_id, filterresult.rows, filterresult.totallength, filterresult.offset.size(), filterresult.data.size());

    // cout << endl;
    //  sendrow.push_back(row[testsmall_line_col[0]] + "," + row[testsmall_line_col[1]] + "," + row[testsmall_line_col[3]] + "," + row[testsmall_line_col[3]] + "," + row[testsmall_line_col[4]] + "," + row[testsmall_line_col[5]] + "," + row[testsmall_line_col[6]] + "," + row[testsmall_line_col[7]] + "," + row[testsmall_line_col[8]] + "," + row[testsmall_line_col[9]] + "," + row[testsmall_line_col[10]] + "," + row[testsmall_line_col[11]] + "," + row[testsmall_line_col[12]] + "," + row[testsmall_line_col[13]] + "," + row[testsmall_line_col[14]] + "," + row[testsmall_line_col[15]] );
}

vector<string> split(string str, char Delimiter)
{
    istringstream iss(str); // istringstream에 str을 담는다.
    string buffer;          // 구분자를 기준으로 절삭된 문자열이 담겨지는 버퍼

    vector<string> result;

    // istringstream은 istream을 상속받으므로 getline을 사용할 수 있다.
    while (getline(iss, buffer, Delimiter))
    {
        result.push_back(buffer); // 절삭된 문자열을 vector에 저장
    }

    return result;
}

bool isvarc(vector<int> datatype, int ColNum)
{
    int isvarchar = 0;
    for (int i = 0; i < ColNum; i++) // varchar 확인
    {
        if (datatype[i] == 15)
        {
            isvarchar = 1;
        }
    }
    return isvarchar;
}

void makedefaultmap(vector<string> ColName, vector<int> startoff, vector<int> offlen, vector<int> datatype, int ColNum, unordered_map<string, int> &startptr, unordered_map<string, int> &lengthRaw, unordered_map<string, int> &typedata)
{
    for (int i = 0; i < ColNum; i++)
    {
        startptr.insert(make_pair(ColName[i], startoff[i]));
        lengthRaw.insert(make_pair(ColName[i], offlen[i]));
        typedata.insert(make_pair(ColName[i], datatype[i]));
    }
}

void makenewmap(int isvarchar, int ColNum, unordered_map<string, int> &newstartptr, unordered_map<string, int> &newlengthraw, vector<int> datatype, unordered_map<string, int> lengthRaw, vector<string> ColName, int &iter, vector<int> startoff, vector<int> offlen, char *rowbuf)
{
    bool aftervarchar = 0;
    int rowlength = 0;
    if (isvarchar == 1)
    {
        newstartptr.clear();
        newlengthraw.clear();
        for (int j = 0; j < ColNum; j++)
        {
            int newofflen = 0;
            if (datatype[j] == 15 || aftervarchar == 1)
            {
                aftervarchar = 1;
                if (datatype[j] == 15)
                { //맨 앞 컬럼 타입이 varchar일 경우 길이를 새로 구하고 그 길이로 시작 인덱스를 구하는데, 그 이후의 모든 시작인덱스를 구해줘야함
                    if (lengthRaw[ColName[j]] < 256)
                    { // varchar 길이 1바이트
                        if (j == 0)
                        {
                            newofflen = (int)rowbuf[iter + startoff[j]];
                            newstartptr.insert(make_pair(ColName[j], iter + startoff[j] + 1));
                            newlengthraw.insert(make_pair(ColName[j], newofflen));
                        }
                        else
                        {
                            // cout << newstartptr[testsmall_line_col[j-1]] + newlengthraw[testsmall_line_col[j-1]] << endl;
                            newofflen = (int)rowbuf[newstartptr[ColName[j - 1]] + newlengthraw[ColName[j - 1]]];
                            newstartptr.insert(make_pair(ColName[j], newstartptr[ColName[j - 1]] + newlengthraw[ColName[j - 1]] + 1));
                            newlengthraw.insert(make_pair(ColName[j], newofflen));
                            // cout << "newofflen = " << (int)rowbuf[iter + newstartptr[testsmall_line_col[j-1]] + newlengthraw[testsmall_line_col[j-1]]] << endl;
                        }
                    }
                    else if (lengthRaw[ColName[j]] >= 256)
                    {
                        // varchar 길이 2바이트
                        char lenbuf[4];
                        lenbuf[2] = 0x00;
                        lenbuf[3] = 0x00;
                        int *lengthtmp;
                        if (j == 0)
                        {
                            for (int k = 0; k < 2; k++)
                            {
                                lenbuf[k] = rowbuf[iter + startoff[j] + k];
                                lengthtmp = (int *)lenbuf;
                            }
                            newofflen = lengthtmp[0];
                            newstartptr.insert(make_pair(ColName[j], iter + startoff[j] + 2));
                            newlengthraw.insert(make_pair(ColName[j], newofflen));
                        }
                        else
                        {
                            // cout << newstartptr[testsmall_line_col[j-1]] + newlengthraw[testsmall_line_col[j-1]] << endl;
                            for (int k = 0; k < 2; k++)
                            {
                                lenbuf[k] = rowbuf[newstartptr[ColName[j - 1]] + newlengthraw[ColName[j - 1]] + k];
                                lengthtmp = (int *)lenbuf;
                            }
                            newofflen = lengthtmp[0];
                            newstartptr.insert(make_pair(ColName[j], newstartptr[ColName[j - 1]] + newlengthraw[ColName[j - 1]] + 2));
                            newlengthraw.insert(make_pair(ColName[j], newofflen));
                            // cout << "newofflen = " << (int)rowbuf[iter + newstartptr[testsmall_line_col[j-1]] + newlengthraw[testsmall_line_col[j-1]]] << endl;
                        }
                    }
                }
                else
                {
                    newstartptr.insert(make_pair(ColName[j], newstartptr[ColName[j - 1]] + newlengthraw[ColName[j - 1]]));
                    newlengthraw.insert(make_pair(ColName[j], offlen[j]));
                }
            }
            else
            {
                newstartptr.insert(make_pair(ColName[j], iter + startoff[j]));
                // cout << iter + startoff[j] << endl;
                newlengthraw.insert(make_pair(ColName[j], offlen[j]));
            }
        }
        // cout << newstartptr[testsmall_line_col[ColNum - 1]] << " " << testsmall_line_col[ColNum - 1] << endl;
        // iter = newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]];
        // cout << iter << endl;
    }
    else
    {
        for (int j = 0; j < ColNum; j++)
        {
            newstartptr.insert(make_pair(ColName[j], iter + startoff[j]));
            newlengthraw.insert(make_pair(ColName[j], offlen[j]));
        }
        // newstartptr = startptr;
        // newlengthraw = lengthRaw;
        // rowlength = startoff[ColNum - 1] + offlen[ColNum - 1];
    }
    iter = newstartptr[ColName[ColNum - 1]] + newlengthraw[ColName[ColNum - 1]];
}

void compareGE(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{
    if (LV >= RV)
    {
        // cout << "LV is ge" << endl;
        // cout << LV << " " << OneRow[WhereClauses[j+1]] << endl;
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}

void compareGE(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{
    if (LV >= RV)
    {
        // cout << "LV is ge" << endl;
        // cout << LV << " " << OneRow[WhereClauses[j+1]] << endl;
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}

void compareLE(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{
    if (LV <= RV)
    {
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareLE(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{
    if (LV <= RV)
    {
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareGT(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{
    if (LV > RV)
    {
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareGT(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{
    if (LV > RV)
    {
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareLT(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{

    if (LV < RV)
    {
        // cout << "LV is small" << endl;
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareLT(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{

    if (LV < RV)
    {
        // cout << "LV is small" << endl;
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareET(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{

    if (LV == RV)
    {
        // cout << "same" << endl;
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareET(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{

    if (LV == RV)
    {
        // cout << "same" << endl;
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        {
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}

void compareNE(string LV, string RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{

    if (LV != RV)
    {
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        { //의미 없음 tmpv 가 false일 경우는 passed
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}
void compareNE(int LV, int RV, bool &CV, bool &TmpV, bool &canSaved, bool isnot)
{

    if (LV != RV)
    {
        if (TmpV == true)
        {
            CV = true;
            canSaved = true;
        }
        else
        { //의미 없음 tmpv 가 false일 경우는 passed
            CV = false;
            canSaved = false;
        }
    }
    else
    {
        CV = false;
        canSaved = false;
    }
    if (isnot)
    {
        if (CV)
        {
            CV = false;
            canSaved = false;
        }
        else
        {
            CV = true;
            canSaved = true;
        }
    }
}

int typeLittle(unordered_map<string, int> typedata, string colname, unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, char *rowbuf)
{
    // cout << "tttteeessssttttt  " << typedata[colname] << endl;
    if (typedata[colname] == 14)
    { // date
        // cout << 1 << endl;
        int *tmphex;
        char temphexbuf[4];
        //  = new char[4];
        int retint;
        temphexbuf[0] == 0x00;
        temphexbuf[1] == 0x00;
        temphexbuf[2] == 0x00;
        temphexbuf[3] == 0x00;
        memset(temphexbuf, 0, 4);
        // cout << "col length : " << newlengthraw[colname] << endl;
        // cout << "date size : " << newlengthraw[colname] << endl;
        for (int k = 0; k < newlengthraw[colname]; k++)
        {
            // cout << rowbuf[newstartptr[colname] + k] << endl;
            temphexbuf[k] = rowbuf[newstartptr[colname] + k];
            // cout << hex << (int)rowbuf[newstartptr[colname] + k] << endl;
        }
        tmphex = (int *)temphexbuf;
        retint = tmphex[0];
        // cout << "tmphex : " << retint << endl;
        // delete[] temphexbuf;
        // cout << "return int = "<< retint << endl;
        return retint;
    }
    else if (typedata[colname] == 3)
    { // int
        char intbuf[4];
        //  = new char[4];
        int *intbuff;
        int retint;

        intbuf[0] == 0x00;
        intbuf[1] == 0x00;
        intbuf[2] == 0x00;
        intbuf[3] == 0x00;
        memset(intbuf, 0, 4);
        // cout << "date size : " << newlengthraw[colname] << endl;
        for (int k = 0; k < newlengthraw[colname]; k++)
        {
            intbuf[k] = rowbuf[newstartptr[colname] + k];
        }
        intbuff = (int *)intbuf;
        retint = intbuff[0];
        // delete[] intbuf;
        //  cout << intbuff[0] << endl;
        return retint;
    }
    return 0;
    // else
    // {
    //     //예외 타입
    //     return NULL;
    // }
}

string typeBig(unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, string colname, char *rowbuf)
{
    string tmpstring = "";
    for (int k = 0; k < newlengthraw[colname]; k++)
    {
        tmpstring = tmpstring + (char)rowbuf[newstartptr[colname] + k];
    }
    return tmpstring;
}
string typeDecimal(unordered_map<string, int> newlengthraw, unordered_map<string, int> newstartptr, string colname, char *rowbuf)
{
    char tmpbuf[4];
    string tmpstring = "";
    for (int k = 0; k < newlengthraw[colname]; k++)
    {
        ostringstream oss;
        int *tmpdata;
        tmpbuf[0] = 0x80;
        tmpbuf[1] = 0x00;
        tmpbuf[2] = 0x00;
        tmpbuf[3] = 0x00;
        tmpbuf[0] = rowbuf[newstartptr[colname] + k];
        tmpdata = (int *)tmpbuf;
        oss << hex << tmpdata[0];
        // oss << hex << rowbuf[newstartptr[WhereClauses[j]] + k];
        if (oss.str().length() <= 1)
        {
            tmpstring = tmpstring + "0" + oss.str();
        }
        else
        {
            tmpstring = tmpstring + oss.str();
        }
        // delete[] tmpbuf;
    }
    return tmpstring;
}

string ItoDec(int inum)
{
    std::stringstream ss;
    std::string s;
    ss << hex << inum;
    s = ss.str();
    string decimal = "80";
    for (int i = 0; i < 10 - s.length(); i++)
    {
        decimal = decimal + "0";
    }
    decimal = decimal + s;
    decimal = decimal + "00";
    return decimal;
}