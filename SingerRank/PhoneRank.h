#pragma once

#include <map>
#include <string>
#include <vector>
#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/base/single_thread_task_runner.h"
#include "third_party/chromium/base/memory/scoped_ptr.h"

#include "RankTypes.h"

class CitySingersManager;

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;

class PhoneRank
{
public:
    PhoneRank();
    ~PhoneRank();

    bool Initialize(base::SingleThreadTaskRunner* runner,
        const base::Callback<void(uint32, bool, const RowData&)>& singer_info_callback,
        const base::Callback<void(const std::wstring&)>& message_callback);

    void Finalize();

    void BreakRequest();

    void DoStop();

    void InitCheckGroupSingers(bool beauty, bool newsinger);
    void RetriveSingerRankResult(uint32 roomid);

    bool GetCityRankInfos(uint32 roomid);

    bool InitNewSingerRankInfos();
    bool GetNewSingerRankByRoomid(uint32 roomid, uint32* rank, uint32* all) const;

    bool InitBeautifulSingerRankInfos();
    bool GetBeautifulSingerRankByRoomid(uint32 roomid, uint32* rank, uint32* all) const;

private:
	bool DoGetCityRankInfos(uint32 roomid, RankSingerInfo* singer_info,
		uint32* rank_id, uint32* online_num, uint32* all_count);

    struct NormalRoomInfo // ֻȡ�˲���Ŀǰ��Ϊ���õ���Ϣ
    {
        uint32 kugou_id = 0;
        //uint32 type_ = 0; // ������ֱ��״̬�ķ���
        uint32 fanxing_id = 0;
        std::string nickname = "";
    };

    struct StarCard // ֻȡ�˲���Ŀǰ��Ϊ���õ���Ϣ
    {
        std::string location; // ������λ�����ڳ���
    };


    bool GetEnterRoomInfoByRoomId(uint32 roomid, 
        NormalRoomInfo* normal_room_info) const;

    bool GetStarCardByKugouId(uint32 kugouid, StarCard* star_card) const;

    // ��ȡ�����б�
    bool GetSinglePageData(
        const std::string& url,
        const std::map<std::string, std::string>& param_map,
        std::vector<RankSingerInfo>* rank_singer_infos,
        bool* has_next_page, bool* all_online, uint32* doubleLiveFirst,
        uint32* mobileFromIndex) const;

    scoped_refptr<base::SingleThreadTaskRunner> runner_;


    std::vector<RankSingerInfo> new_singers_rank_;
    std::vector<RankSingerInfo> beautiful_singers_rank_;

    std::unique_ptr<CitySingersManager> city_manager_;


    bool beauty_ = false;
    bool newsinger_ = false;
    // δʵ��
    std::vector<RankSingerInfo> register_singer_rank_;
    std::vector<RankSingerInfo> man_singer_rank_;
    std::vector<RankSingerInfo> good_voice_rank_;
    std::vector<RankSingerInfo> band_rank_;
    std::vector<RankSingerInfo> group_rank_;
    std::vector<RankSingerInfo> dj_rank_;

    base::Callback<void(uint32, bool, const RowData&)> singer_info_callback_;
    base::Callback<void(const std::wstring&)> message_callback_;

    // �Ż�����Ĳ���
    bool break_all_request_;
};

