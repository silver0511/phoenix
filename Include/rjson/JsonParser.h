//
// Created by shiyunjie on 2017/5/24.
//

#ifndef __JSONPARSER_H__
#define __JSONPARSER_H__

#include <string>
#include <cstdio>
#include <type_traits>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"

#ifndef OUT
    #define OUT
#endif

typedef rapidjson::Value RJsonValue;
typedef rapidjson::Document RJDocument;
typedef rapidjson::Document::AllocatorType RJAllocType;
typedef rapidjson::SizeType RJ_SIZE_TYPE;
typedef rapidjson::Value::ConstValueIterator RJConstIterator;
//创建空的根对象
#define RJ_CREATE_EMPTY_DOCUMENT(root_name) \
    RJDocument root_name; \

//创建根(对象)
#define RJ_CREATE_O_DOCUMENT(root_name, alloc_name) \
    RJDocument root_name(rapidjson::kObjectType); \
    RJAllocType& alloc_name = root_name.GetAllocator(); \

//创建根(数组)
#define RJ_CREATE_A_DOCUMENT(root_name, alloc_name) \
    RJDocument root_name(rapidjson::kArrayType); \
    RJAllocType& alloc_name = root_name.GetAllocator(); \

//创建子(对象)
#define RJ_CREATE_OBJECT(value_name) \
    RJsonValue value_name(rapidjson::kObjectType); \

//创建子(数组)
#define RJ_CREATE_ARRAY(value_name) \
    RJsonValue value_name(rapidjson::kArrayType); \

class JsonParse
{
public:
    JsonParse(){};
    ~JsonParse(){};

    /*
     * suport K_ID:[int, unsigned, bool, double, int64_t, uint64_t, int8_t, uint8_t, int16_t, uint16_t]
    */
    template<typename K_ID>
    static bool add(RJsonValue &json_value, const char* key, const K_ID &value, RJAllocType& allocator_type)
    {
        if(!json_value.IsObject())
        {
            return false;
        }

        if(has(json_value, key))
        {
            json_value.RemoveMember(key);
        }

        json_value.AddMember<K_ID>(RJsonValue(key, allocator_type).Move(),
                                   value, allocator_type);
        return true;
    }

    static bool add(RJsonValue &json_value, const char* key, const std::string &value, RJAllocType& allocator_type)
    {
        if(!json_value.IsObject())
        {
            return false;
        }

        if(has(json_value, key))
        {
            json_value.RemoveMember(key);
        }

        rapidjson::Value p_name(rapidjson::kStringType);
        p_name.SetString(key, allocator_type);
        rapidjson::Value p_value(rapidjson::kStringType);
        p_value.SetString(value.c_str(), (int)value.size(), allocator_type);
        json_value.AddMember(p_name, p_value, allocator_type);
        return true;
    }

    static bool add(RJsonValue &json_value, const char* key, RJsonValue &value, RJAllocType& allocator_type)
    {
        if (!json_value.IsObject())
        {
            return false;
        }

        if (has(json_value, key))
        {
            json_value.RemoveMember(key);
        }

        json_value.AddMember(RJsonValue(key, allocator_type).Move(), value, allocator_type);
        return true;
    }

    /*
    * suport K_ID:[string, int, unsigned, bool, double, int64_t, uint64_t, int8_t, uint8_t, int16_t, uint16_t]
    */
    template<typename K_ID>
    static bool get(RJsonValue &json_value, const char* key, OUT K_ID &out_value)
    {
        if(!json_value.IsObject())
        {
            return false;
        }

        auto iter = json_value.FindMember(key);
        if(iter == json_value.MemberEnd())
        {
            return false;
        }

        return get_value<K_ID>(iter->value, out_value);
    }

    //support string
    static bool get(RJsonValue &json_value, const char* key, OUT std::string &out_value)
    {
        if(!json_value.IsObject())
        {
            return false;
        }

        auto iter = json_value.FindMember(key);
        if(iter == json_value.MemberEnd())
        {
            return false;
        }

        if(!iter->value.IsString())
        {
            return false;
        }

        out_value = iter->value.GetString();
        return true;
    }

    //ps:外部必须通过引用去接，否则会swap原本的dom结构
    //RJsonValue &p = get(...);
    static RJsonValue& get(RJsonValue &json_value, const char* key)
    {
        assert(json_value.IsObject());
        auto iter = json_value.FindMember(key);
        if(iter == json_value.MemberEnd())
        {
            return get_null();
        }
        return iter->value;
    }



    static bool remove(RJsonValue &json_value, const char* key)
    {
        if(!json_value.IsObject())
        {
            return false;
        }

        json_value.EraseMember(key);
    }

    /*
    * suport K_ID:[int, unsigned, bool, double, int64_t, uint64_t, int8_t, uint8_t, int16_t, uint16_t]
    */
    template<typename K_ID>
    static bool push(RJsonValue &json_value, const K_ID &value, RJAllocType& allocator_type)
    {
        if(!json_value.IsArray())
        {
            return false;
        }

        json_value.PushBack<K_ID>(value, allocator_type);

        return true;
    }

    static bool push(RJsonValue &json_value, const std::string &value, RJAllocType& allocator_type)
    {
        if(!json_value.IsArray())
        {
            return false;
        }

        rapidjson::Value p_value(rapidjson::kStringType);
        p_value.SetString(value.c_str(), value.size(), allocator_type);
        json_value.PushBack(p_value, allocator_type);

        return true;
    }

    static bool push(RJsonValue &json_value, RJsonValue &value, RJAllocType& allocator_type)
    {
        if(!json_value.IsArray())
        {
            return false;
        }

        json_value.PushBack(value, allocator_type);
        return true;
    }

    static RJ_SIZE_TYPE count(RJsonValue &json_value)
    {
        if(!json_value.IsArray())
        {
            return -1;
        }

        return json_value.Size();
    }

    /*
    * suport K_ID:[string, int, unsigned, bool, double, int64_t, uint64_t, int8_t, uint8_t, int16_t, uint16_t]
    */
    template<typename K_ID>
    static bool at(RJsonValue &json_value, int index, OUT K_ID &out_value)
    {
        if(!json_value.IsArray())
        {
            return false;
        }

        if(index < 0 || index >= json_value.Size())
        {
            return false;
        }

        return get_value<K_ID>(json_value[index], out_value);
    }

    //support string
    static bool at(RJsonValue &json_value, int index, OUT std::string &out_value)
    {
        if(!json_value.IsArray())
        {
            return false;
        }

        if(index < 0 || index >= json_value.Size())
        {
            return false;
        }

        if(!json_value[index].IsString())
        {
            return false;
        }

        out_value = json_value[index].GetString();
        return true;
    }

    //ps:外部必须通过引用去接，否则会swap原本的dom结构
    //RJsonValue &p = at(...);
    static RJsonValue& at(RJsonValue &json_value, int index)
    {
        assert(json_value.IsArray());
        if(index < 0 || index >= json_value.Size())
        {
            return get_null();
        }

        return json_value[index];
    }

    static bool erase(RJsonValue &json_value, int index)
    {
        if(!json_value.IsArray())
        {
            return false;
        }

        if(index < 0 || index >= json_value.Size())
        {
            return false;
        }

        RJsonValue::ConstValueIterator beg = json_value.Begin();
        RJsonValue::ConstValueIterator end = json_value.End();
        json_value.Erase(beg + index);
    }

    static bool parse(RJDocument &json_document, const char* data)
    {
        if(NULL == data)
        {
            return false;
        }

        json_document.Parse<0>(data);
        bool has_error = json_document.HasParseError();
        if(has_error)
        {
            fprintf(stderr, "\nparse Error(offset %u): %s, data:%s\n",
                    (unsigned)json_document.GetErrorOffset(),
                    rapidjson::GetParseError_En(json_document.GetParseError()), data);
            return false;
        }

        if(json_document.IsNull())
        {
            has_error = true;
        }

        return !has_error;
    }

    static bool parse_file(RJDocument &json_document, const char* file_name)
    {
        if(NULL == file_name)
        {
            return false;
        }

        FILE* fp = fopen(file_name, "rb");
        if(!fp)
        {
            return false;
        }

        fseek(fp,0,SEEK_END);
        int buf_len = ftell(fp);
        fseek(fp,0,SEEK_SET);

        printf("file_name = %s, read size = %d \n", file_name, buf_len);

        char *buf = (char*)malloc(sizeof(char) * buf_len);
        rapidjson::FileReadStream is(fp, buf, buf_len);
        json_document.ParseStream(is);
        fclose(fp);
        free(buf);

        bool has_error = json_document.HasParseError();
        if(has_error)
        {
            fprintf(stderr, "\nparse_file Error(offset %u): %s\n",
                    (unsigned)json_document.GetErrorOffset(),
                    rapidjson::GetParseError_En(json_document.GetParseError()));
        }

        if(json_document.IsNull())
        {
            has_error = true;
        }

        return !has_error;
    }

    static bool to_string(const RJsonValue& value, OUT std::string &out_value)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        out_value = buffer.GetString();
        return true;
    }

    static bool is_null(const RJsonValue &value)
    {
        return value.IsNull();
    }

private:
    /*
     * support K_ID:[int, unsigned, bool, double, int64_t, uint64_t, int8_t, uint8_t, int16_t, uint16_t]
    */
    template<typename K_ID>
    static bool get_value(RJsonValue &json_value, OUT K_ID &out_value)
    {
        bool result = true;
        if(std::is_same<K_ID, int>::value && json_value.IsInt())
        {
            out_value = json_value.GetInt();
        }
        else if(std::is_same<K_ID, unsigned>::value && json_value.IsUint())
        {
            out_value = json_value.GetUint();
        }
        else if(std::is_same<K_ID, bool>::value && json_value.IsBool())
        {
            out_value = json_value.GetBool();
        }
        else if(std::is_same<K_ID, double>::value && json_value.IsDouble())
        {
            out_value = json_value.GetDouble();
        }
        else if(std::is_same<K_ID, int64_t>::value && json_value.IsInt64())
        {
            out_value = json_value.GetInt64();
        }
        else if(std::is_same<K_ID, uint64_t>::value && json_value.IsUint64())
        {
            out_value = json_value.GetUint64();
        }
        else if(std::is_same<K_ID, int8_t>::value && json_value.IsInt())
        {
            out_value = (int8_t)json_value.GetInt();
        }
        else if(std::is_same<K_ID, uint8_t>::value && json_value.IsInt())
        {
            out_value = (uint8_t)json_value.GetInt();
        }
        else if(std::is_same<K_ID, int16_t>::value && json_value.IsInt())
        {
            out_value = (int16_t)json_value.GetInt();
        }
        else if(std::is_same<K_ID, uint16_t>::value && json_value.IsInt())
        {
            out_value = (uint16_t)json_value.GetInt();
        }
        else
        {
            result = false;
        }

        return result;
    }

    static bool has(RJsonValue &json_value, const char* key)
    {
        return json_value.HasMember(key);
    }

    static RJsonValue& get_null()
    {
        static RJsonValue null_value(rapidjson::kNullType);
        return null_value;
    }
};

#endif //__JSONPARSER_H__
