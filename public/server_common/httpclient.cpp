#include "httpclient.h"
#include "common/Debug_log.h"

CHttpClient::CHttpClient(void):m_bDebug(false),m_szCookie("")
{  
  curl = curl_easy_init();
}  
  
CHttpClient::~CHttpClient(void)  
{  
  curl_easy_cleanup(curl);
}  
  
static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)  
{  
    if(itype == CURLINFO_TEXT)  
    {        
		//LOG_TRACE(6, true, __FUNCTION__, "[TEXT]" << pData);
    }  
    else if(itype == CURLINFO_HEADER_IN)  
    {  
 		LOG_TRACE(6, true, __FUNCTION__, "[HEADER_IN]" << pData);
    }  
    else if(itype == CURLINFO_HEADER_OUT)  
    {       
		LOG_TRACE(6, true, __FUNCTION__, "[HEADER_OUT]" << pData);
    }  
    else if(itype == CURLINFO_DATA_IN)  
    {  
 		LOG_TRACE(6, true, __FUNCTION__, "[DATA_IN]" << pData);
    }  
    else if(itype == CURLINFO_DATA_OUT)  
    {  
		LOG_TRACE(6, true, __FUNCTION__, "[DATA_OUT]" << pData); 
    }  
    return 0;  
}  
  
static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);  
    if( NULL == str || NULL == buffer )  
    {  
        return -1;  
    }  
  
    char* pData = (char*)buffer;  
    str->append(pData, size * nmemb);  
    return nmemb;  
}  
  
int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)  
{  
    CURLcode res;  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }

    if(m_bDebug)
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }
  
	//struct curl_slist *slist=NULL;
	//slist = curl_slist_append(slist, "channel:Qianbao Test");
	//slist = curl_slist_append(slist, "User-Agent:qbaonew-ios/4.0.0");
	//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	 
	//if (!m_szCookie.empty())
	//{
	//	 curl_easy_setopt(curl, CURLOPT_COOKIE, m_szCookie.c_str());
	//}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);  
    res = curl_easy_perform(curl);  
    return res;  
}  
  
int CHttpClient::Get(const std::string & strUrl, std::string & strResponse)  
{  
    CURLcode res;    
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }  
    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }  
	//struct curl_slist *slist=NULL;
	//slist = curl_slist_append(slist, "channel:Qianbao Test");
	//slist = curl_slist_append(slist, "User-Agent:qbaonew-ios/4.0.0");
	//slist = curl_slist_append(slist, "JSESSIONID:CSignleUser::GetInstance()->GetJsessionId().c_str()");
    

	//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	//if (!m_szCookie.empty())
	//{
	//	 curl_easy_setopt(curl, CURLOPT_COOKIE, m_szCookie.c_str());
	//}
	
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());	
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  
     
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);  
    res = curl_easy_perform(curl);  
    return res;  
}  
  
int CHttpClient::Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath)  
{  
    CURLcode res;  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }  

    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }  

	//struct curl_slist *slist=NULL;
	//slist = curl_slist_append(slist, "channel:Qianbao Test");
	//slist = curl_slist_append(slist, "User-Agent:qbaonew-ios/4.0.0");
	
	//if (!m_szCookie.empty())
	//{
	//	 curl_easy_setopt(curl, CURLOPT_COOKIE, m_szCookie.c_str());
	//}
	
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());  
	//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);  

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    if(NULL == pCaPath)  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);  
    }  
    else  
    {   
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);  
    }  
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);  
    res = curl_easy_perform(curl);  
    return res;  
}  
  
int CHttpClient::Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath)  
{  
    CURLcode res;  
    if(NULL == curl)  
    {  
        return CURLE_FAILED_INIT;  
    }  
    if(m_bDebug)  
    {  
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);  
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);  
    }  

	//struct curl_slist *slist=NULL;
	//slist = curl_slist_append(slist, "channel:Qianbao Test");
	//slist = curl_slist_append(slist, "User-Agent:qbaonew-ios/4.0.0");
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	
	//if (!m_szCookie.empty())
	//{
	//	 curl_easy_setopt(curl, CURLOPT_COOKIE, m_szCookie.c_str());
	//}
	
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
    if(NULL == pCaPath)  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);  
    }  
    else  
    {  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);  
    }  
    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);  
    res = curl_easy_perform(curl);  
    return res;  
}

void CHttpClient::SetDebug(bool bDebug)  
{  
    m_bDebug = bDebug;
}  
