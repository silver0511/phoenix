// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
// 
// 文件名称: DBI.h
// 创 建 人: 史云杰
// 文件说明:  MySQL C　API封装连接池头文件
// $_FILEHEADER_END *****************************

#ifndef ___COMMON_DBI_H_
#define ___COMMON_DBI_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <string>

namespace U9
{
    ///< DBI 类型
    enum DBITYPE
    {
        MYSQL_DBI = 1 , ///< mysql
    } ;

    ///< 数据库结构信息
    struct CDBinfo
    {
        std::string   modbi;
        uint32 muDBtype;
        uint32 muindexID;			//在同类型数据库中的索引
        std::string   modbip;
        uint32 mudbport;
        uint32 mudbindex;			//在全部数据库中的索引
        std::string   modbname;
        std::string   modbuser;
        std::string   modbpwd;
        uint32 mudbstarttime ;
        uint32 mudbendtime ;
        uint32 mudbusertype ;
        uint32 mupoolsize;
    };

    typedef std::vector<CDBinfo> CDBInfoList ;
    typedef std::vector<CDBinfo>::iterator CDBInfoListPtr ;

	class IRecordSet
	{
	public:
        IRecordSet() {};
		virtual ~IRecordSet() {};

		virtual uint32 getRecordCount() = 0;
		virtual uint32 getFieldCount() = 0;
		virtual bool getRecord() = 0;
		virtual const char* GetField(uint32 index) = 0;
		virtual uint32 getFieldLength(uint32 index) = 0;
	};

    // 自动获取释放查询结果的对象
    class AutoReleaseRecordSet
    {
    public:
        AutoReleaseRecordSet() ;
        AutoReleaseRecordSet(IRecordSet* apRecSet) ;
        virtual ~AutoReleaseRecordSet() ;

        IRecordSet& operator *() ;
        IRecordSet* operator ->() ;
        bool IsValid() ;
        IRecordSet* GetRecordSet() ;

    private:
        IRecordSet*				mpRecSet;
    };

    class IConnItem
    {
    public:
        IConnItem() {};
        virtual ~IConnItem() {};

        ///< 连接是否有效
        virtual bool IsValid() = 0 ;

        // 直接执行SQL语句.
        virtual IRecordSet* Query(const char* aszQuery, uint32 auiLength) = 0;

        // 选择数据库
        virtual int SelectDB(const char* aszDBName)  = 0 ;

        // 准备绑定参数的语句
        virtual bool StmtPrepare(const char* aszQuery, uint32 auiLength)  = 0 ;

        // 执行绑定参数的SQL语句
        virtual bool StmtExecute()  = 0 ;

        // 执行绑定参数的SQL语句
        virtual bool Execute(const char* aszScript, uint32 auiLength)  = 0 ;

        // 关闭预处理语句
        virtual int StmtClose() = 0 ;

        // 确定连接有效
        virtual int Ping() = 0 ;

        // 返回当前默认字符集名字
        virtual const char* GetCharacterSetName()  = 0 ;

        ///< 获取执行影响的行数
        virtual int GetAffectedRows() = 0 ; 

        // 设置当前默认字符集名字,参数不为const类型,因为mysqlAPI就不是const类型的参数
        virtual int SetCharacterSet(const char* aszCsName) = 0 ;

        ///< 字符串需要序列化
        virtual uint32 escapeString(const char* apszSrc, uint32 auSrclen, char* apszDst) = 0  ;

        ///< 获取最后mysql的错误码字符串
        virtual const char* GetLastErrMsg()   = 0 ;

        ///< 获取最后mysql的错误码
        virtual uint32 getLastError() = 0;

		virtual void CleanAllNextResult() = 0;
    };

	class IDriverPool
	{
	public:
        IDriverPool() {};
		virtual ~IDriverPool() {};
        virtual void init() = 0 ;
        virtual void Release() = 0 ;

        virtual uint32 Open(const CDBInfoList& aDBList) = 0 ;	
        virtual uint32 Open(const CDBinfo& aDBInfo) = 0 ;
        virtual void Close() = 0;	
        virtual void Close(uint32 auDBindex) = 0;

        virtual IConnItem*	MallocItem(const uint64& auid, const uint32 auindex) = 0;	          // 获取空闲连接对象
        virtual int FreeItem(const uint64& auid, const uint32 auDBtype, IConnItem* apItem) = 0 ;   // 释放连接
        virtual uint32 size() = 0 ;
	};

    // 自动获取释放连接的对象
    class AutoReleaseConnItemExt
    {
    public:
        AutoReleaseConnItemExt() ;
        AutoReleaseConnItemExt(uint64 auid, uint32 auDBindex, IDriverPool* apDriverPool) ;
        virtual ~AutoReleaseConnItemExt() ;

        IConnItem& operator *() ;
        IConnItem* operator ->() ;
        bool IsValid() ;
        IConnItem* GetConnItem() ;

    private:
        IConnItem*				mpConnItem;
        IDriverPool*            mpDriverPool ;
        uint64				muid;
        uint32                muDBindex ;
    };

	/*!
	 *	Create a DB connection object.
	 *	@param name connection type,can be one of "mysql,mysqlsp".
     * mysql (1)
	 */
	IDriverPool* GetDBConnectionByName(const DBITYPE& auDBItype);
	void FreeDBConnection(IDriverPool* apDBIConn);
}

#endif	/*___COMMON_DBI_H_*/

