#include "base0/platform.h"
#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/EcServerPackDef.h"
#include "ServerUtilMgr.h"
#include "ServerConn/UserConMgr.h"
#include "./ClientSide/ClientSide.h"
#include "ServerMgr.h"
#include "EcRecentOrdersTask.h"

void CEcRecentOrdersTask::DoForwardPackageFail()
{
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Forward package fail url = " << EC_RECENTORDER_API);

    int liResult = RET_SYS_PACK_TYPE_INVALID;
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    STRU_PACKAGE_RS<T_EC_GETRECENTORDERS_RS> loSendPack(NEW_DEF_BUSINESS_GETRECENTORDERS_RS);
    std::vector< flatbuffers::Offset<ecpack::T_ORDER_INFO> > std_ecorder_info_vec;
    auto fbs_ecorders_info_vec = loSendPack.fbbuilder.CreateVector(std_ecorder_info_vec);
    ecpack::T_EC_GETRECENTORDERS_RSBuilder c1 = ecpack::T_EC_GETRECENTORDERS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_code(0);
    c1.add_order_list(fbs_ecorders_info_vec);
    loSendPack.fbbuf = c1.Finish();

    CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

    if(!lpUser)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
        return;
    }

    m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);
    return;
}

void CEcRecentOrdersTask::run()
{
    if (NULL == m_parm)
    {
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = EC_RECENTORDER_API + "?" + m_data;

    std::string szReponse;
    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "http get url=" << szUrl);

	if (CURLE_OK != phttpclient->Get(szUrl.c_str(), szReponse))
	{		
        DoForwardPackageFail();
		return;
	}

	if (szReponse.empty())
	{
        DoForwardPackageFail();
		return;
	}

    RJDocument json_document;

    if (!JsonParse::parse(json_document, szReponse.c_str()))
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "json is error =" << szReponse);
        DoForwardPackageFail();
		return;
    }
	
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
    
    STRU_PACKAGE_RS<T_EC_GETRECENTORDERS_RS> loSendPack(NEW_DEF_BUSINESS_GETRECENTORDERS_RS);

    int licode = json_document["code"].GetInt();
    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);
    
    RJsonValue& jsondata = json_document["data"];

    std::vector< flatbuffers::Offset<ecpack::T_ORDER_INFO> > std_ecorder_info_vec;
    for (unsigned int i = 0 ; i < jsondata.Size(); i++)
    {
        RJsonValue& jsondataobj = jsondata[i];
        uint64_t buyerId = 0;
        uint64_t orderId = 0;
        uint64_t payTime = 0;
        uint64_t sellerId = 0;
        uint64_t totalAmount = 0;
        int      orderState = 0;
        std::string orderStateStr("");

        JsonParse::get(jsondataobj, "buyerId", buyerId); 
        JsonParse::get(jsondataobj, "orderId", orderId); 
        JsonParse::get(jsondataobj, "payTime", payTime);
        JsonParse::get(jsondataobj, "sellerId", sellerId);
        JsonParse::get(jsondataobj, "totalAmount", totalAmount);
        JsonParse::get(jsondataobj, "orderState", orderState);
        JsonParse::get(jsondataobj, "orderStateStr", orderStateStr);

        RJsonValue& jsongoodsInfoDto4IMsobj = JsonParse::get(jsondataobj, "goodsInfoDto4IMs");
        std::vector< flatbuffers::Offset<ecpack::T_GOODSINFTOIMS> > std_goods_infotoims_vec;
        for (unsigned int j = 0; j < jsongoodsInfoDto4IMsobj.Size(); j++)
        {
            RJsonValue& jsontoimobj = jsongoodsInfoDto4IMsobj[j];
            uint64_t goodsId = 0;
            int goodsNum = 0;
            uint64_t skuId = 0;
            int unitPrice = 0;
            std::string goodsName("");
            std::string skuText("");
            std::string imgUrl("");

            JsonParse::get(jsontoimobj, "goodsId", goodsId); 
            JsonParse::get(jsontoimobj, "goodsNum", goodsNum); 
            JsonParse::get(jsontoimobj, "skuId", skuId);
            JsonParse::get(jsontoimobj, "unitPrice", unitPrice);
            JsonParse::get(jsontoimobj, "goodsName", goodsName);
            JsonParse::get(jsontoimobj, "skuText", skuText);
            JsonParse::get(jsontoimobj, "imgUrl", imgUrl);

            auto fbsgoodsname = loSendPack.fbbuilder.CreateString(goodsName.c_str());
            auto fbsskutext = loSendPack.fbbuilder.CreateString(skuText.c_str());
            auto fbsimgurl = loSendPack.fbbuilder.CreateString(imgUrl.c_str());

            ecpack::T_GOODSINFTOIMSBuilder fbsgoodsInfoDto4IMs = ecpack::T_GOODSINFTOIMSBuilder(loSendPack.fbbuilder);
            fbsgoodsInfoDto4IMs.add_goodsId(goodsId);
            fbsgoodsInfoDto4IMs.add_goodsNum(goodsNum);
            fbsgoodsInfoDto4IMs.add_skuId(skuId);
            fbsgoodsInfoDto4IMs.add_unitPrice(unitPrice);
            fbsgoodsInfoDto4IMs.add_goodsName(fbsgoodsname);
            fbsgoodsInfoDto4IMs.add_skuText(fbsskutext);
            fbsgoodsInfoDto4IMs.add_imgUrl(fbsimgurl);
            std_goods_infotoims_vec.push_back(fbsgoodsInfoDto4IMs.Finish());            
        }
       
        auto fbs_orderstatestr = loSendPack.fbbuilder.CreateString(orderStateStr);
        auto fbs_goodsInfoDto4IMs_vec = loSendPack.fbbuilder.CreateVector(std_goods_infotoims_vec);
        ecpack::T_ORDER_INFOBuilder fbsorderinfo = ecpack::T_ORDER_INFOBuilder(loSendPack.fbbuilder);
        fbsorderinfo.add_buyerId(buyerId);
        fbsorderinfo.add_orderId(orderId);
        fbsorderinfo.add_payTime(payTime);
        fbsorderinfo.add_sellerId(sellerId);
        fbsorderinfo.add_totalAmount(totalAmount);
        fbsorderinfo.add_orderstate(orderState);
        fbsorderinfo.add_orderstatestr(fbs_orderstatestr);
        fbsorderinfo.add_goodsInfoDto4IMs(fbs_goodsInfoDto4IMs_vec);
        std_ecorder_info_vec.push_back(fbsorderinfo.Finish());       
    }

    auto fbs_ecorders_info_vec = loSendPack.fbbuilder.CreateVector(std_ecorder_info_vec);
    ecpack::T_EC_GETRECENTORDERS_RSBuilder c1 = ecpack::T_EC_GETRECENTORDERS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_code(licode);
    c1.add_order_list(fbs_ecorders_info_vec);
    loSendPack.fbbuf = c1.Finish();

    CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

    if(!lpUser)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
        return;
    }

    m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

    uint64 interval =  GetInterValTime();
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "userid" << m_aiUserID << " pop queue time = " << interval); 
    
    return;
}

