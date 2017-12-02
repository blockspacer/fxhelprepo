#include "stdafx.h"


#include <shlwapi.h>

#undef max
#undef min
#include "SingerInfoDatabase.h"
#include "SingerInfo.h"

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
        sql += "singer_info";
        // UserInfo
        sql += "(fxid integer PRIMARY KEY NOT NULL,";
        sql += "kgid integer NOT NULL DEFAULT 0,";
        sql += "clanid integer NOT NULL DEFAULT 0,";
        sql += "nickname nvarchar(260) NOT NULL DEFAULT '',";
        sql += "richlevel integer NOT NULL DEFAULT 0,";
        sql += "starlevel integer NOT NULL DEFAULT 0,";
        sql += "fanscount integer NOT NULL DEFAULT 0,";
        sql += "followcount integer NOT NULL DEFAULT 0,";
        sql += "sex nvarchar(260) NOT NULL DEFAULT '',";
        sql += "location nvarchar(260) NOT NULL DEFAULT '',";

        // RoomInfo
        sql += "roomid integer NOT NULL DEFAULT 0,";
        sql += "declare nvarchar(260) NOT NULL DEFAULT '',";
        sql += "billboard_month_1 integer NOT NULL DEFAULT 0,";
        sql += "billboard_month_2 integer NOT NULL DEFAULT 0,";
        sql += "billboard_month_3 integer NOT NULL DEFAULT 0,";
        sql += "billboard_month_4 integer NOT NULL DEFAULT 0,";
        sql += "billboard_month_5 integer NOT NULL DEFAULT 0,";
        sql += "billboard_all_1 integer NOT NULL DEFAULT 0,";
        sql += "billboard_all_2 integer NOT NULL DEFAULT 0,";
        sql += "billboard_all_3 integer NOT NULL DEFAULT 0,";
        sql += "billboard_all_4 integer NOT NULL DEFAULT 0,";
        sql += "billboard_all_5 integer NOT NULL DEFAULT 0,";

        // other
        sql += "star_singer integer NOT NULL DEFAULT 0,";
        sql += "tags nvarchar(260) NOT NULL DEFAULT '',";
        sql += "last_online nvarchar(260) NOT NULL DEFAULT '',";
        sql += "last_online_day integer NOT NULL DEFAULT 0,";
        sql += "date nvarchar(260) NOT NULL DEFAULT '',);";

        return std::move(sql);
    }

    std::string GetInsertWorshipRecodeSql(const SingerInfo& singer_info)
    {
        std::string sql = "Insert Into tb_worship_raw ";
        //sql += "( server_time, result, display_result, random ) Values ";
        //sql += "(" + base::UintToString(singer_info.time);
        //sql += "," + base::UintToString(singer_info.result);
        //sql += "," + base::UintToString(singer_info.display_result);
        //sql += "," + base::UintToString(singer_info.random) + ");";


        return std::move(sql);
    }

    std::string GetQueryAllRecodeSql()
    {
        std::string sql = "Select server_time, result, display_result, random ";
        sql += "From tb_worship_raw;";
        return std::move(sql);
    }
}


int SingerInfoDatabase::SQLErrorLog::OnError(int error, sql::Connection* connection,
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


SingerInfoDatabase::SingerInfoDatabase()
    :db_conn_()
    ,sqlLog_(new SQLErrorLog())
{
    sqlLog_->AddRef();
}


SingerInfoDatabase::~SingerInfoDatabase()
{
    sqlLog_->Release();
}

bool SingerInfoDatabase::Initialize(const std::wstring& file_name)
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

void SingerInfoDatabase::Finalize()
{
    db_conn_.Close();
}

bool SingerInfoDatabase::InsertRecord(const SingerInfo& singer_info)
{
    if (!db_conn_.BeginTransaction())
    {
        return false;
    }
    std::string insert_sql = GetInsertWorshipRecodeSql(singer_info);
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

bool SingerInfoDatabase::Query(std::vector<SingerInfo>* singer_infos)
{
    std::string sql = GetQueryAllRecodeSql();
    try
    {
        Statement stmt(db_conn_.GetUniqueStatement(sql.c_str()));
        while (stmt.Step())
        {
            SingerInfo singer_info;
            //singer_info.time = stmt.ColumnInt(0);
            //singer_info.result = stmt.ColumnInt(1);
            //singer_info.display_result = stmt.ColumnInt(2);
            //singer_info.random = stmt.ColumnInt(3);
            singer_infos->push_back(singer_info);
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

bool SingerInfoDatabase::OpenDatabase(const std::wstring& file_name)
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

bool SingerInfoDatabase::IntegrityCheck()
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

