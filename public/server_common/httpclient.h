#ifndef __HTTP_CLIENT_H_
#define __HTTP_CLIENT_H_
#include <string>
#include "curl/curl.h"
#include "curl/types.h"
#include "curl/easy.h"

class CHttpClient  
{  
public:  
    CHttpClient(void);  
    ~CHttpClient(void);  
  
public:
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);  
    int Get(const std::string & strUrl, std::string & strResponse);  

    int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);  
    int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);  
  
public:
    void SetDebug(bool bDebug);  
	void SetCookie(const std::string & szCookie) {m_szCookie = szCookie;}
  
private:  
    bool m_bDebug;
	std::string m_szCookie;
	CURL* curl;
};
#endif
