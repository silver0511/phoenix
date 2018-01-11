//
// Created by shiyunjie on 17/3/28.
//

#include "SensitiveMgr.h"
#include "ServerUtilMgr.h"
#include "rjson/JsonParser.h"

S_T_NODE::S_T_NODE()
{
    m_end = false;
}

S_T_NODE::~S_T_NODE()
{
    S_MAP_ITER iter;
    for(iter == s_map.begin(); iter != s_map.end(); iter++)
    {
        SAFE_DELETE(iter->second);
    }

    s_map.clear();
}

void S_T_NODE::set_end()
{
    m_end = true;
}

bool S_T_NODE::is_end()
{
    return m_end;
}

S_T_NODE* S_T_NODE::find(const string &key_word)
{
    S_MAP_ITER iter = s_map.find(key_word);
    if(iter == s_map.end())
    {
        return NULL;
    }

    return iter->second;
}

S_T_NODE* S_T_NODE::insert(const string &key_word)
{
    S_MAP_ITER iter = s_map.find(key_word);
    if(iter != s_map.end())
    {
//        LOG_TRACE(LOG_INFO, false, __FUNCTION__, "key_word = "
//                << key_word <<" is exsist: " << iter->second);
        return iter->second;
    }

    S_T_NODE *new_node = new S_T_NODE();
    s_map.insert(S_MAP::value_type(key_word, new_node));
    return new_node;
}

int S_T_NODE::size()
{
    return s_map.size();
}


SensitiveMgr::SensitiveMgr()
{
    init();
}

SensitiveMgr::~SensitiveMgr()
{

}

void SensitiveMgr::init()
{
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "parse json start: " << CSystem::GetMicroSystemTime());
    RJ_CREATE_EMPTY_DOCUMENT(l_document)
    bool result = JsonParse::parse_file(l_document, DEF_SENSITIVE_CONFIG);
    if(!result || l_document.IsNull())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "read json file invalid");
        return;
    }

    RJsonValue &json_data = JsonParse::at(l_document, 0);
    if(json_data.IsNull())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "read json data not array");
        return;
    }

    RJsonValue &json_data_list = JsonParse::get(json_data, "data");
    RJ_SIZE_TYPE data_count = JsonParse::count(json_data_list);
    if(json_data.IsNull() || data_count <= 0)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "read json data not exist");
        return;
    }

    for (RJ_SIZE_TYPE index = 0; index < data_count; ++index)
    {
        string s_text = "";
        RJsonValue& json_value_list = JsonParse::at(json_data_list, index);
        RJsonValue& json_value = JsonParse::at(json_value_list, 0);
        if(json_value.IsInt())
        {
            s_text = std::to_string(json_value.GetInt());
        }
        else if(json_value.IsString())
        {
            s_text = json_value.GetString();
        }

        int w_index = 0;
        S_T_NODE *p_cur_node = &m_parent_node;
        string single_word;
        while(w_index < s_text.size())
        {
            int f_word_size = check_utf_8(s_text[w_index]);
            single_word = s_text.substr(w_index, f_word_size);
            w_index += f_word_size;
            p_cur_node = p_cur_node->insert(single_word);
        }
        if(s_text.size() > 0 && p_cur_node != &m_parent_node)
        {
            p_cur_node->set_end();
        }
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "parse json end: " << CSystem::GetMicroSystemTime()
    << " parent_node size: " << m_parent_node.size());
}

int SensitiveMgr::check_utf_8(char word)
{
    int size = 1;
    if(word & 0x80)
    {
        word <<= 1;
        do{
            word <<= 1;
            ++size;
        }while(word & 0x80);
    }
    return size;
}

bool SensitiveMgr::check(S_T_NODE* s_t_node, const string& text, int &offset)
{
    if(s_t_node->is_end())
    {
        //敏感词
        return true;
    }

    int word_size = check_utf_8(text[offset]);
    string single_word = text.substr(offset, word_size);
    offset += word_size;

    S_T_NODE *child_node = s_t_node->find(single_word);
    if(NULL == child_node)
    {
        //非敏感词
        return false;
    }

    return check(child_node, text, offset);
}

bool SensitiveMgr::Exsists(const string &text)
{
    int cur_index = 0;
    while(cur_index < text.size())
    {
        int offset = cur_index;
        int f_word_size = check_utf_8(text[offset]);
        if(!check(&m_parent_node, text, offset))
        {
            offset = f_word_size;
        }
        else
        {
            return true;
        }

        cur_index = offset;
    }

    return false;
}

int SensitiveMgr::Replace(const string &in_text, string &out_text)
{
    int c_count = 0;
    int cur_index = 0;
    bool is_sensitive = false;
    bool is_last_sensitive = false;
    while(cur_index < in_text.size())
    {
        int offset = cur_index;
        int f_word_size = check_utf_8(in_text[offset]);
        if(!check(&m_parent_node, in_text, offset))
        {
            out_text += in_text.substr(cur_index, f_word_size);
            cur_index += f_word_size;
            is_last_sensitive = false;
            c_count++;
        }
        else
        {
            is_sensitive = true;
            if(!is_last_sensitive)
            {
                out_text += DEF_REPLACE_WORD;
                c_count++;
            }

            if(cur_index == offset)
            {
                LOG_TRACE(LOG_ERR, false, __FUNCTION__, "invalid offset: " << offset
                                                                           << "text: " << in_text);
                cur_index += f_word_size;
            }
            else
            {
                cur_index = offset;
            }
            is_last_sensitive = true;
        }
    }

    return c_count;
}