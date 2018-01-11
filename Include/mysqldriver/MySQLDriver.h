// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: MySQLDriver.h
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装连接池头文件
// $_FILEHEADER_END *****************************

#ifndef __MYSQLDRIVER_H_
#define __MYSQLDRIVER_H_

#include <mysql/mysql.h>
#include <memory>
#include "common/Debug_log.h"
#include "common/CacheList.h"
#include "common/RefObjMgrTemplate.h"
#include "MySQLRes.h"
#include "DBI.h"

using namespace U9;

/////////////////////////////////////////////////////////////////////////////
// CMySQLItem 类
class CMySQLItem:public U9::IConnItem
{
public:
	CMySQLItem();									// 构造函数
	virtual ~CMySQLItem();							// 析构函数

    virtual bool IsValid() ;

	//初始化连接
    bool Open(const char* apszServerAddr, uint32 auPort,
        const char* apszDBname, const char* apszUser, const char* apszPassword);	

	int Close();									// 关闭连接

	// 确定连接有效
	int Ping();

	int RollBack();								// 回滚
	
	// 判断一个数据库是否存在
	bool IsDBExist(const char* aszDBName);

	// 创建MYSQL_STMT句柄
	MYSQL_STMT* StmtInit();

	// 直接执行查询SQL语句.
	IRecordSet* Query(const char* aszQuery, uint32 auiLength);

	// 选择数据库
	int SelectDB(const char* aszDBName);

	// 准备绑定参数的语句
	bool StmtPrepare(const char* aszQuery, uint32 auiLength);

	// 绑定参数,此参数以MYSQL_BIND类型的数组形式传入,数组的个数可以由StmtPrepare中?的数量确定
	bool StmtBindParam(MYSQL_BIND* aoBind);

    // 执行绑定参数的SQL语句
	bool StmtExecute();	

    // 直接执行SQL语句
    virtual bool Execute(const char* aszScript, uint32 auiLength) ;

	// 关闭预处理语句
	int StmtClose();

	// 返回当前默认字符集名字
	const char* GetCharacterSetName();

	// 设置当前默认字符集名字,参数不为const类型,因为mysqlAPI就不是const类型的参数
	int SetCharacterSet(const char* aszCsName);

    uint32 escapeString(const char* apszSrc, uint32 auSrclen, char* apszDst) ;
	const char* GetLastErrMsg() ;
    uint32 getLastError() ;

	MYSQL* GetMySQL(){return mpMySQL;}
	MYSQL_STMT* GetStmt(){return mpStmt;}

	int GetAffectedRows();

	void CleanAllNextResult();

	auto_ptr<CMySQLRecordSet> StoreResult();

	// $_FUNCTION_BEGIN ******************************
	// 方法名: Connect
	// 访  问: private 
	// 参  数: const char * aszHost	主机IP地址
	// 参  数: const char * aszUser	用户名
	// 参  数: const char * aszPasswd	密码
	// 参  数: const char * aszDb	数据库名
	// 参  数: unsigned int auiPort	端口号
	// 返回值: 0表示成功,其他表示失败
	// 说  明: 连接
	// $_FUNCTION_END ********************************
	bool Connect(const char *aszHost, const char *aszUser, const char *aszPasswd,
		const char *aszDb, uint32 auiPort);

   // void checkDBConnection() ;

private:
    bool ReConnect() ;

private:
	MYSQL* mpMySQL;						// MySQL连接句柄
	MYSQL_STMT * mpStmt;				// 绑定参数

	static const uint32 MAXERRMSGLENGTH = 4096;	// 错误消息最长的大小
	char mszLastErrMsg[MAXERRMSGLENGTH];
    bool mbIsRun ;
    uint32 muKeepLiveTimeStamp ;
    std::string modbaddr ;
    uint32 muPort ;
    std::string modbname ;
    std::string modbuser ;
    std::string modbPassword ;
    bool mbConnectting ;
};

/////////////////////////////////////////////////////////////////////////////
// CMySQLPool 类
class CMySQLPool:public U9::CRefObjectforCache
{
public:
	CMySQLPool();
	virtual ~CMySQLPool();

    void init() ;

	// 打开数据库
    uint32 Open(const char* apszServerAddr, uint32 auPort,
        const char* apszDBname, const char* apszUser, const char* apszPassword, 
        const uint32 auDBUsertype, const uint32 auIndex, const uint32 auPoolSize);	

	void Close();// 关闭数据池

    inline uint32 getDBtype(){return m_uDBtype;} ;
    inline uint32 getDBIndex(){return m_uIndex;} ;
    inline uint32 getPoolSize(){return m_uPoolSize;} ;

	IConnItem*	MallocItem();				// 获取MySQL空闲连接对象

	inline void FreeItem(IConnItem* apItem)			// 释放MySQL连接
	{
		moMySQLList.AddTail(apItem);
	}

	inline int Size(){return moMySQLList.size();}	// 获取并发连接数

private:
	CSafeStaticQueue<IConnItem> moMySQLList;				// 缓冲队列

	bool mbActive;
    uint32 m_uDBtype ;
    uint32 m_uIndex ;
    uint32 m_uPoolSize ;
};

typedef CMySQLPool* PMySQLPool;

#endif	// __MYSQLDRIVER_H_
