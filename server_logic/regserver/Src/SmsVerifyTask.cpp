//
// Created by shiyunjie on 2018/1/8.
//

#include "SmsVerifyTask.h"
#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "const/constdef.h"
#include "common/basefun.h"

void SmsVerifyTask::DoForwardPackageFail()
{

}

void SmsVerifyTask::run()
{
    if (NULL == m_parm)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "m_parm is null");
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;
    int random_key = rand();
    ULONG cur_time = CSystem::GetTime();
    string url_param = "sdkappid=" + TT_SMS_APP_ID +
                       "&random=" + std::to_string(random_key);
    string tt_sms_url = TT_SMS_VERIFY_API + "?" + url_param;

    RJ_CREATE_O_DOCUMENT(l_document, l_allocator)

    RJ_CREATE_OBJECT(tel_object)
    JsonParse::add(tel_object, "nationcode", m_nation_code, l_allocator);
    JsonParse::add(tel_object, "mobile", m_mobile, l_allocator);
    JsonParse::add(l_document, "tel", tel_object, l_allocator);

    string sign = "";
    JsonParse::add(l_document, "sign", sign, l_allocator);
    JsonParse::add<int>(l_document, "tpl_id", TT_SMS_APP_T_ID, l_allocator);

    RJ_CREATE_ARRAY(params_array)
    JsonParse::push(params_array, verify_code, l_allocator);
    JsonParse::push(params_array, valid_time, l_allocator);
    JsonParse::add(l_document, "params", params_array, l_allocator);

    string origin_sign = "appkey=" + TT_SMS_APP_KEY +
            "&random=" + std::to_string(random_key) +
            "&time=" + std::to_string(cur_time) +
            "&mobile=" + m_mobile;
    string sign_sha256 = sha_256(origin_sign.c_str(), origin_sign.length());
    JsonParse::add(l_document, "sig", sign_sha256, l_allocator);
    JsonParse::add<uint64_t>(l_document, "time", cur_time, l_allocator);
    string extend = "";
    JsonParse::add(l_document, "extend", extend, l_allocator);
    string ext = "";
    JsonParse::add(l_document, "ext", ext, l_allocator);
    string post_param = "";
    JsonParse::to_string(l_document, post_param);

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "http post url=" << tt_sms_url
                                            << " content = " << post_param
                                            << " origin_sign = " << origin_sign
                                            << " sign_sha256 = " << sign_sha256);
    string l_response;
    int http_result = phttpclient->Posts(tt_sms_url, post_param, l_response, NULL);
    if(CURLE_OK != http_result)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http post error url=" << tt_sms_url << " http_result=" << http_result);
        DoForwardPackageFail();
        return;
    }

    if (l_response.empty())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http response empty=" << tt_sms_url);
        DoForwardPackageFail();
        return;
    }

    RJ_CREATE_EMPTY_DOCUMENT(response_doc)
    if(!JsonParse::parse(response_doc, l_response.c_str()))
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http response not json =" << l_response);
        DoForwardPackageFail();
        return;
    }

    int result;
    string errmsg = "";
    JsonParse::get(response_doc, "result", result);
    JsonParse::get(response_doc, "errmsg", errmsg);
    if(result != 0 || errmsg != "OK")
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http response error " << l_response);
    }
}