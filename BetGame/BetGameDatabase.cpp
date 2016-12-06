#include "stdafx.h"


#include <shlwapi.h>

#undef max
#undef min
#include "BetGameDatabase.h"
#include "Network/BetData.h"

#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/sys_info.h"
#include "third_party/chromium/sql/statement.h"

#include "third_party/chromium/base/strings/string_util.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

using std::wstring;
using sql::Statement;
using sql::Connection;
using std::string;
using base::PlatformFile;
using base::Bind;
using base::Unretained;
using base::SequencedTaskRunner;
using base::FilePath;
//
//struct BetResult
//{
//    uint32 time = 0;
//    uint32 result = 0;
//    uint32 display_result = 0;
//    uint32 random = 0;
//};

namespace
{
    std::string GetCreateWorshipTableSql()
    {
        std::string sql = "Create TABLE ";
        sql += "tb_worship_raw";
        sql += "(server_time integer PRIMARY KEY NOT NULL,";
        sql += "result integer NOT NULL DEFAULT 0,";
        sql += "display_result integer NOT NULL DEFAULT 0,";
        sql += "random integer NOT NULL DEFAULT 0);";
        return std::move(sql);
    }

    std::string GetInsertWorshipRecodeSql(const BetResult& bet_result)
    {
        std::string sql = "Insert Into tb_worship_raw ";
        sql += "( server_time, result, display_result, random )";
        sql += "(" + base::UintToString(bet_result.time);
        sql += "," + base::UintToString(bet_result.result);
        sql += "," + base::UintToString(bet_result.display_result);
        sql += "," + base::UintToString(bet_result.random) + ");";
        return std::move(sql);
    }

    std::string GetQueryAllRecodeSql()
    {
        std::string sql = "Select server_time, result, display_result, random ";
        sql += "From tb_worship_raw;";
        return std::move(sql);
    }
}


int BetGameDatabase::SQLErrorLog::OnError(int error, sql::Connection* connection,
    sql::Statement* stmt)
{
    // Avoid too much logs;
    if (errCounter_ > 500)
    {
        return error;
    }
    ++errCounter_;

    const char* sqlstmt = stmt ? stmt->GetSQLStatement() : "";
    sqlstmt = sqlstmt ? sqlstmt : "";

    LOG(ERROR) << L"SQLite error {code: " << error << L", msg: \""
        << connection->GetErrorMessage() << L"\"} while executing {"
        << sqlstmt << L"}.";

    return error;
}


BetGameDatabase::BetGameDatabase()
    :db_conn_()
    ,sqlLog_(new SQLErrorLog())
{
    sqlLog_->AddRef();
}


BetGameDatabase::~BetGameDatabase()
{
    sqlLog_->Release();
}

bool BetGameDatabase::Initialize(const std::wstring& file_name)
{
    bool isOpened = false;
    FilePath filePath(file_name);

    db_conn_.set_page_size(4096);
    db_conn_.set_cache_size(64);

    FilePath database(file_name);
    if (!base::PathExists(database))
    {
        if (!base::CreateDirectory(database.DirName()))
        {
            return false;
        }
    }

    if (!OpenDatabase(filePath.value()))
    {
        return false;
    }

    if (!db_conn_.DoesTableExist("tb_worship_raw"))
    {
        db_conn_.Execute(GetCreateWorshipTableSql().c_str());
    }

    return false;
}

void BetGameDatabase::Finalize()
{
    db_conn_.Close();
}

bool BetGameDatabase::InsertRecord(const BetResult& bet_result)
{
    if (!db_conn_.BeginTransaction())
    {
        return false;
    }
    std::string insert_sql = GetInsertWorshipRecodeSql(bet_result);
    if (!db_conn_.Execute(insert_sql.c_str()))
    {
        return false;
    }
    if (!db_conn_.CommitTransaction())
    {
        return false;
    }
    return true;
}

bool BetGameDatabase::Query(std::vector<BetResult>* bet_results)
{
    std::string sql = GetQueryAllRecodeSql();
    try
    {
        Statement stmt(db_conn_.GetUniqueStatement(sql.c_str()));
        while (stmt.Step())
        {
            BetResult bet_result;
            bet_result.time = stmt.ColumnInt(0);
            bet_result.result = stmt.ColumnInt(1);
            bet_result.display_result = stmt.ColumnInt(2);
            bet_result.random = stmt.ColumnInt(3);
            bet_results->push_back(bet_result);
        }
    }
    catch (...)
    {
        std::string error_msg;
        const char* error = db_conn_.GetErrorMessage();
        if (error)
            error_msg = string(" error: ") + error;
        assert(false && L"query error!");
        return false;
    }

    return true;
}

bool BetGameDatabase::OpenDatabase(const std::wstring& file_name)
{    
    FilePath database(file_name);

    db_conn_.Close();
    SetFileAttributes(file_name.c_str(), FILE_ATTRIBUTE_NORMAL);
    sqlLog_->reset();
    db_conn_.set_error_delegate(sqlLog_);

    if (db_conn_.Open(database))
    {
        if (IntegrityCheck())
            return true;

        db_conn_.Execute("VACUUM");
        if (IntegrityCheck())
            return true;

        db_conn_.Close();
        return false;
    }

    db_conn_.Close();
    return false;
}

bool BetGameDatabase::IntegrityCheck()
{
    if (!db_conn_.is_open())
        return false;

    Statement stmt(db_conn_.GetCachedStatement(SQL_FROM_HERE,
        "PRAGMA Integrity_Check"));

    if (!stmt.is_valid())
        return false;

    if (stmt.Step())
        return FilePath::CompareEqualIgnoreCase(stmt.ColumnString16(0), L"ok");

    return false;
}

