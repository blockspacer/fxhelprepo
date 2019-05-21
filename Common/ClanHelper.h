#pragma once

#include <memory>
#include <string>
#include <vector>
#undef max
#undef min
#include "third_party/chromium/base/basictypes.h"

struct ClanSingerInfo
{
	std::string img_path;
	std::string user_logo;
	int64 kugou_id;
	int live_status;
	std::string nickname;
	int roomid;
	int star_level;
	int fanxing_id;
	int viewer_num;
};

class CurlWrapper;

class ClanHelper
{
public:
	ClanHelper(int clanid);
	~ClanHelper();

	bool GetAllClanSingers(std::vector<ClanSingerInfo>* clan_singers);

private:

	bool GetPageClanSinger(int page, std::vector<ClanSingerInfo>* clan_singers,
		uint32* total_count, uint32* total_page);

	int clan_id_;
	std::unique_ptr<CurlWrapper> curlWrapper_ = nullptr;

};

