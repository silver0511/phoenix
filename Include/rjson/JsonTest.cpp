//
// Created by shiyunjie on 2017/6/2.
//

#include "rjson/JsonParser.h"
#include <sys/time.h>
#include <pthread.h>

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
    //根对象
    RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
    string str;
    str = "i am parent str";
    JsonParse::add(l_document, "parent_str", str, l_allocator);
    JsonParse::add<int8_t>(l_document, "parent_int8", (int8_t)8, l_allocator);
    JsonParse::add<uint8_t>(l_document, "parent_uint8", (uint8_t)-1, l_allocator);
    JsonParse::add<int16_t>(l_document, "parent_int16", (int16_t)16, l_allocator);
    JsonParse::add<uint16_t>(l_document, "parent_uint16", (uint16_t)-2, l_allocator);
    JsonParse::add<int>(l_document, "parent_int", 111, l_allocator);
    JsonParse::add<unsigned>(l_document, "parent_unsigned", (unsigned)222, l_allocator);
    JsonParse::add<bool>(l_document, "parent_bool", true, l_allocator);
    JsonParse::add<double>(l_document, "parent_double", 333.33f, l_allocator);
    JsonParse::add<int64_t>(l_document, "parent_int64", (int64_t)131239129381290, l_allocator);
    JsonParse::add<uint64_t>(l_document, "parent_uint64_t", (uint64_t)-2, l_allocator);

    //内嵌对象1
    RJ_CREATE_OBJECT(child_object)
    str = "i am child str";
    JsonParse::add(child_object, "child_str", str, l_allocator);
    JsonParse::add<int64_t>(child_object, "child_int64", (int64_t)7342342777, l_allocator);

    //内嵌数组2->内嵌对象1
    RJ_CREATE_ARRAY(child_array1)
    for(int i = 0; i < 100; i++)
    {
        JsonParse::push<int>(child_array1, i, l_allocator);
    }
    JsonParse::add(child_object, "child_array1", child_array1, l_allocator);

    //内嵌数组1
    RJ_CREATE_ARRAY(child_array)
    str = "i am child array str";
    JsonParse::push(child_array, str, l_allocator);
    JsonParse::push<uint64_t>(child_array, (uint64_t)349234283, l_allocator);

    //内嵌数组2->内嵌数组1
    RJ_CREATE_ARRAY(child_array2)
    str = "i am child array2 str";
    JsonParse::push(child_array2, str, l_allocator);
    JsonParse::push<uint64_t>(child_array2, (uint64_t)88888888, l_allocator);
    JsonParse::push(child_array, child_array2, l_allocator);

    //内嵌对象2->内嵌数组1
    RJ_CREATE_OBJECT(child_obj2)
    str = "i am child4 str";
    JsonParse::add(child_obj2, "child4_str", str, l_allocator);
    JsonParse::add<int64_t>(child_obj2, "child4_int64", (int64_t)4234121211313, l_allocator);
    JsonParse::push(child_array, child_obj2, l_allocator);
    JsonParse::push<uint8_t>(child_array, (uint8_t)-10, l_allocator);
    JsonParse::push<int16_t>(child_array, (int16_t)27, l_allocator);

    //内嵌数组1->根对象
    JsonParse::add(l_document, "child_array", child_array, l_allocator);
    //内嵌对象1->根对象
    JsonParse::add(l_document, "child_obj", child_object, l_allocator);

    std::string out_json = "";
    JsonParse::to_string(l_document, out_json);
    return out_json;
}

void json_decode(RJDocument &doc, const char* data)
{
    JsonParse::parse(doc, data);
    //根对象解析
    string str_val;
    JsonParse::get(doc, "parent_str", str_val);
    assert(str_val == "i am parent str");
    int int_val;
    JsonParse::get<int>(doc, "parent_int", int_val);
    assert(int_val == 111);
    unsigned unsigned_val;
    JsonParse::get<unsigned>(doc, "parent_unsigned", unsigned_val);
    assert((unsigned_val == (unsigned)222));
    bool b_val;
    JsonParse::get<bool>(doc, "parent_bool", b_val);
    assert(b_val == true);
    double double_val;
    JsonParse::get<double>(doc, "parent_double", double_val);
    assert(double_val == 333.33f);
    int64_t  i64_val;
    JsonParse::get<int64_t>(doc, "parent_int64", i64_val);
    assert(i64_val == (int64_t)131239129381290);
    uint64_t  ui64_val;
    JsonParse::get<uint64_t>(doc, "parent_uint64_t", ui64_val);
    assert(ui64_val == (uint64_t)-2);
    int8_t int8_val;
    JsonParse::get<int8_t>(doc, "parent_int8", int8_val);
    assert(int8_val == (int8_t)8);
    uint8_t uint8_val;
    JsonParse::get<uint8_t>(doc, "parent_uint8", uint8_val);
    assert(uint8_val == (uint8_t)-1);
    int16_t int16_val;
    JsonParse::get<int16_t>(doc, "parent_int16", int16_val);
    assert(int16_val == (int16_t)16);
    uint16_t uint16_val;
    JsonParse::get<uint16_t>(doc, "parent_uint16", uint16_val);
    assert(uint16_val == (uint16_t)-2);

    //内嵌对象1
    RJsonValue &child_object = JsonParse::get(doc, "child_obj");
    assert(!child_object.IsNull());
    JsonParse::get(child_object, "child_str", str_val);
    assert(str_val == "i am child str");
    JsonParse::get<int64_t>(child_object, "child_int64", i64_val);
    assert(i64_val == (int64_t)7342342777);

    //内嵌数组2->内嵌对象1
    RJsonValue &child_array1 = JsonParse::get(child_object, "child_array1");
    //解析数组
    int i = 0;
    int data_count = JsonParse::count(child_array1);
    for (; i < data_count; ++i)
    {
        JsonParse::at<int>(child_array1, i, int_val);
        assert(int_val == i);
    }

    i = 0;
    for (RJConstIterator iter = child_array1.Begin(); iter != child_array1.End(); ++iter)
    {
        assert(iter->GetInt() == i);
        i++;
    }
    //c++11 support
    i = 0;
    for (auto &v : child_array1.GetArray())
    {
        assert(v.GetInt() == i);
        i++;
    }

    //内嵌数组1
    RJsonValue &child_array = JsonParse::get(doc, "child_array");
    assert(!child_array.IsNull());
    JsonParse::at(child_array, 0, str_val);
    assert(str_val == "i am child array str");
    JsonParse::at<uint64_t>(child_array, 1, ui64_val);
    assert(ui64_val == (uint64_t)349234283);

    //内嵌数组2->内嵌数组1
    RJsonValue &child_array2 = JsonParse::at(child_array, 2);
    assert(!child_array2.IsNull());
    JsonParse::at(child_array2, 0, str_val);
    assert(str_val == "i am child array2 str");
    JsonParse::at<uint64_t>(child_array2, 1, ui64_val);
    assert(ui64_val == (uint64_t)88888888);


    //内嵌对象2->内嵌数组1
    RJsonValue &child_obj2 = JsonParse::at(child_array, 3);
    assert(!child_obj2.IsNull());
    JsonParse::get(child_obj2, "child4_str", str_val);
    assert(str_val == "i am child4 str");
    JsonParse::get<int64_t>(child_obj2, "child4_int64", i64_val);
    assert(i64_val == (int64_t)4234121211313);


    JsonParse::at(child_array, 4, uint8_val);
    assert(uint8_val == (uint8_t)-10);
    JsonParse::at(child_array, 5, int16_val);
    assert(int16_val == (int16_t)27);
}

void *aaa(void* p)
{
    while(true)
    {
        string out_json = json_encode();
        RJ_CREATE_EMPTY_DOCUMENT(r_document)
        json_decode(r_document, out_json.c_str());
    }
}

int main()
{
    unsigned long start_time = get_time();
    string out_json = json_encode();
    unsigned long end_time = get_time();
    cout << "delta time encode: " << (end_time - start_time) << endl;
    cout << out_json.c_str() << endl;



    start_time = get_time();
    RJ_CREATE_EMPTY_DOCUMENT(r_document)
    json_decode(r_document, out_json.c_str());
    end_time = get_time();
    cout << "delta time decode: " << (end_time - start_time) << endl;

//    for(int i = 0; i < 1000; i++)
//    {
//        pthread_t lthread;
//        pthread_create(&lthread, NULL, aaa, 0);
//    }
    getchar();
    return 1;
}