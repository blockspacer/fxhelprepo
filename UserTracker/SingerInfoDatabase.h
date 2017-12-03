#pragma once
#include <vector>

#include "third_party/chromium/sql/connection.h"


struct SingerInfo;

class SingerInfoDatabase
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

    SingerInfoDatabase();
    ~SingerInfoDatabase();

    // 打开数据库，如果不存在则创建后打开
    bool Initialize(const std::wstring& file_name);

    // 把数据flush到数据库并且关闭数据库及文件
    void Finalize();

    // 执行插入数据操作
    bool InsertRecord(const SingerInfo& bet_result);

    // 查询所有的原始记录数据
    bool Query(std::vector<SingerInfo>* bet_results);
    
private:
    sql::Connection db_conn_;
    SQLErrorLog* sqlLog_;
    std::string table_name_;

    std::string date_;

    bool OpenDatabase(const std::wstring& file_name);
    bool IntegrityCheck();
};


