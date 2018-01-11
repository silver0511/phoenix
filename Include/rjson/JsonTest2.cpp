//
// Created by shiyunjie on 2017/6/5.
//
#include <sys/time.h>
#include <iostream>
#include "jsoncmdpacket/JsonCmdPacket.h"
using namespace std;

unsigned long get_time()
{
    struct timeval now;
    gettimeofday(&now,NULL);
    unsigned long tv_sec = now.tv_sec;
    unsigned long tv_usec = now.tv_usec;
    return tv_sec * 1000000 + now.tv_usec;
}

string json_encode()
{
    CJsonCmdPacket out_json;
    string str;
    str = "fdsfsdfsdfsdfief2@$*(*$@($+@([党风建设来得及发顺丰加上逻辑积分水电费商家粉丝雷锋精神连接方式了就的发送到发送到】";
    out_json.PutAttrib("parent_str", str);
    out_json.PutAttribUN("parent_int", 111);
    out_json.PutAttribUN("parent_unsigned", 222);
    out_json.PutAttribUN("parent_bool", 1);
    out_json.PutAttribUN("parent_double", 333.33f);
    out_json.PutAttribUN("parent_int64", 1313);
    out_json.PutAttribUN("parent_uint64_t", -1);

    Json::Value child_object;
    str = "miehaadkfj的咖啡机收代理费（）&*）&）&***（";
    child_object["child_str"] = str;
    child_object["child_int64"] = 121
    child_object["child_obj"] = child_object;

    return out_json.GetString();
}

void json_decode(const char* data, int len)
{
    CJsonCmdPacket p_json;
    p_json.SetData(data, len);
}

int main()
{
    unsigned long start_time = get_time();
    string out_json = json_encode();
    unsigned long end_time = get_time();
    cout << "delta time encode: " << (end_time - start_time) << endl;
    cout << out_json.c_str() << endl;



    start_time = get_time();
    json_decode(out_json.c_str(), out_json.length());
    end_time = get_time();
    cout << "delta time decode: " << (end_time - start_time) << endl;
    return 1;
}
