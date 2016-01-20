#include "stdafx.h"
#include "FanXingUI/family_background/FamilyBackground.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "Network/FamilyDaily.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

//FamilyBackground familyBackground;
//familyBackground.Init();
//familyBackground.Login("username", "password");

FamilyBackground::FamilyBackground()
{

}

FamilyBackground::~FamilyBackground()
{

}

bool FamilyBackground::Init()
{
    FamilyDaily::CurlInit();
    familyDaily_.reset(new FamilyDaily);
    bool result = familyDaily_->Init();
    return result;
}

bool FamilyBackground::Login(const std::string& username, 
    const std::string& password)
{
    bool result = familyDaily_->Login(username, password);
    return false;
}

