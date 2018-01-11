//
// Created by shiyunjie on 17/3/28.
//

#ifndef QBIM_SENSITIVEMGR_H
#define QBIM_SENSITIVEMGR_H
#include "base0/platform.h"
#include <map>
#include "common/Debug_log.h"

#define DEF_SENSITIVE_CONFIG "./config/sensitive_words.json"
#define DEF_REPLACE_WORD "*"

class S_T_NODE
{
public:
    S_T_NODE();
    ~S_T_NODE();
    S_T_NODE* find(const string &key_word);
    S_T_NODE* insert(const string &key_word);
    void set_end();
    bool is_end();
    int size();
    typedef u9_hash_map<string, S_T_NODE*> S_MAP;
    typedef u9_hash_map<string, S_T_NODE*>::iterator S_MAP_ITER;
//private:
    S_MAP s_map;
    bool m_end;
};

class SensitiveMgr
{
public:
    SensitiveMgr();
    ~SensitiveMgr();

public:
    bool Exsists(const string &text);
    int Replace(const string &in_text, string &out_text);
private:
    void init();
    int check_utf_8(char word);
    bool check(S_T_NODE* s_t_node, const string& text, int &offset);
    S_T_NODE   m_parent_node;
};


#endif //QBIM_SENSITIVEMGR_H
