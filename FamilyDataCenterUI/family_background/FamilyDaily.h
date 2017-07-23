#pragma once

#include <string>
#include <vector>

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��

#include "Network/CookiesHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"

struct SingerDailyData
{
    std::string date;
    uint32 onlinecount = 0;         //��������
    uint32 onlineminute = 0;        //�ۼ�ֱ��ʱ�������ӣ�
    std::string pc_hours;
    std::string phone_hours;
    uint32 effectivecount = 0;      //��Чֱ������������1��Сʱ��
    uint32 maxusers = 0;            //ֱ�����������
    double revenue = 0.0;             // �Ƕ�����
    uint32 blame = 0;               // �����ۼƿ۷�
};

class SingerSummaryData
{
public:
    uint32 singerid = 0;
    uint32 roomid = 0;
    std::string nickname;
    std::string singerlevel;    //�����ȼ�
    uint32 onlinecount = 0;         //��������
    uint32 onlineminute = 0;        //�ۼ�ֱ��ʱ�������ӣ�
    std::string total_hours;    // �ۼ�ֱ��ʱ��(Сʱ)
    std::string pc_hours;
    std::string phone_hours;
    uint32 effectivecount = 0;      //��Чֱ������������1��Сʱ��
    uint32 effectivedays = 0;       //��Чֱ���죬��������һ����Чֱ������
    uint32 maxusers = 0;            //ֱ�����������
    double revenue = 0.0;             // �Ƕ�����
};

class FamilyDaily
{
public:
    FamilyDaily();
    ~FamilyDaily();

    static void CurlInit();
    static void CurlCleanup();

    // ����ҳ��,��ȡ��Ӧ��cookie;
    bool Init();

    bool Login(const std::string& username, const std::string& password);

    bool GetDailyDataBySingerId(uint32 singerid,
                                const base::Time& begintime, 
                                const base::Time& endtime,
                                std::vector<SingerDailyData>* dailydata);

    bool GetSummaryData(const base::Time& begintime, const base::Time& endtime,
                        std::vector<SingerSummaryData>* summerydata);

    bool GetNormalSingerList(std::vector<uint32>* singerids);

    bool WriteResponseDataCallback(const std::string& data);
    bool WriteResponseHeaderCallback(const std::string& data);

private:
    bool GetSummaryDataByPage(const base::Time& begintime,
        const base::Time& endtime, uint32 pagenumber,
        std::string* pagedata);

    bool GetDailyDataBySingerIdAndPage(uint32 singerid,
        const base::Time& begintime,
        const base::Time& endtime, uint32 pagenumber,
        std::string* pagedata);

    bool GetSingerListByPage(uint32 pagenumber, std::string* pagedata);

    // ��ժȡ�������л�ȡ����������Ϣ�Լ����з�ҳ����
    bool ParseSummaryData(const std::string& pagedata, 
                          std::vector<SingerSummaryData>* summerydata) const;

    bool ParseSingerDailyData(const std::string& pagedata,
        std::vector<SingerDailyData>* singerdailydata) const;

    bool ParseSingerListData(const std::string& pagedata, 
        std::vector<uint32>* singerids) const;

    bool ParsePageCount(const std::string& pagedata, uint32* pagenumber) const;

    CookiesHelper cookies_helper_;

    std::string currentResponseData_;
    std::string currentResponseHeader_;
};
