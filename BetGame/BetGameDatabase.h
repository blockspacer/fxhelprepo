#pragma once
#include <vector>

#include "third_party/chromium/sql/connection.h"


struct BetResult;

class BetGameDatabase
{
public:
    class SQLErrorLog : public sql::ErrorDelegate
    {
    public:
        SQLErrorLog() : errCounter_(0)
        {
        }

        virtual int OnError(int error, sql::Connection* connection, sql::Statement* stmt);

        inline void reset()
        {
            errCounter_ = 0;
        }

    protected:
        int errCounter_;
    };

    BetGameDatabase();
    ~BetGameDatabase();

    // 打开数据库，如果不存在则创建后打开
    bool Initialize(const std::wstring& file_name);

    // 把数据flush到数据库并且关闭数据库及文件
    void Finalize();

    // 执行插入数据操作
    bool InsertRecord(const BetResult& bet_result);

    // 查询所有的原始记录数据
    bool Query(std::vector<BetResult>* bet_results);
    
private:
    sql::Connection db_conn_;
    SQLErrorLog* sqlLog_;

    bool OpenDatabase(const std::wstring& file_name);
    bool IntegrityCheck();
};


