#include "stdafx.h"
#include "WebHandler.h"

#include <vector>

namespace
{
std::vector<std::wstring> SplitString(std::wstring str, const std::wstring& pattern)
{
    std::vector<std::wstring> result;
    str += pattern;//扩展字符串以方便操作
    int size = str.size();

    for (int i = 0; i < size; i++)
    {
        int pos = str.find(pattern, i);
        if (pos < size)
        {
            std::wstring s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
}

WebHandler::WebHandler(IHTMLDocument2* htmlDocument)
    : htmlDocument_(htmlDocument)
{
}


WebHandler::~WebHandler()
{
}

void WebHandler::WaitFor(int ms)
{
    CEvent evtTemp;
    int nMaxCnt = ms / 10;
    int nCnt = 0;
    while (WAIT_TIMEOUT == ::WaitForSingleObject(evtTemp.m_hObject, 10))
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (++nCnt > nMaxCnt)
            break;
    }
}


CComQIPtr<IHTMLElement> WebHandler::GetElement(
    CComQIPtr<IHTMLElementCollection> spElementCollection,
    const std::wstring& attrName, const std::wstring& attrValue,
    const std::wstring& textValue)
{
    long nItemCount = 0;				//取得表单数目
    HRESULT hr = spElementCollection->get_length(&nItemCount);
    if (FAILED(hr))
        return CComQIPtr<IHTMLElement>();

    for (long i = 0; i < nItemCount; i++)
    {
        CComDispatchDriver spDisp;	//取得第 j 项表单域
        hr = spElementCollection->item(CComVariant(i), CComVariant(), &spDisp);
        if (FAILED(hr))
            continue;

        CComQIPtr<IHTMLElement> spElement = spDisp;
        if (!spElement)
            break;

        bool bFind = false;
        std::wstring compareStr;

        if (attrName == L"class")
        {
            CComBSTR className;
            hr = spElement->get_className(&className);
            if (!FAILED(hr) && className)
            {
                compareStr = (LPCTSTR)className;
            }
         }   
         else
         {
             CComVariant vtAttr;
             hr = spDisp.GetPropertyByName(attrName.c_str(), &vtAttr);
             if (!FAILED(hr) && (vtAttr.vt == VT_BSTR) && (NULL != vtAttr.bstrVal))
             {
                 compareStr = (LPCTSTR)vtAttr.bstrVal;
             }
         }

        if (!compareStr.empty() && (compareStr.compare(attrValue) == 0))
        {
            if (textValue.empty())
            {
                bFind = true;
            }
            else
            {
                CComBSTR innerText;
                hr = spElement->get_innerText(&innerText);
                if (!FAILED(hr) && innerText)
                {
                    if ((textValue.compare((LPCTSTR)innerText) == 0))
                    {
                        bFind = true;
                    }
                }
            }
        }
        else
        {
            continue;
        }

        if (bFind)
        {
            return spElement;
        }
    }

    return CComQIPtr<IHTMLElement>();
}

void WebHandler::Execute()
{
    CComQIPtr<IHTMLElementCollection> spElementCollection;
    HRESULT hr = htmlDocument_->get_all(&spElementCollection);	//取得表单集合
    if (FAILED(hr))
        return;

    auto loginLabel = GetElement(spElementCollection, L"id", L"fxLogin");
    if (!loginLabel)
        return;

    loginLabel->click();
    WaitFor(50);

    auto username = GetElement(spElementCollection, L"name", L"username");
    if (username)
    {
        username->click();
        username->put_innerText(L"jordanlsw");
    }
    auto password = GetElement(spElementCollection, L"name", L"password");
    if (password)
    {
        password->click();
        password->put_innerText(L"123123");
    }
    auto login_btn = GetElement(spElementCollection, L"name", L"login_btn");
    if (login_btn)
    {
        login_btn->click();
    }
}

void WebHandler::Login(const CString& userName, const CString& pwd)
{
    CComQIPtr<IHTMLElementCollection> spElementCollection;
    HRESULT hr = htmlDocument_->get_all(&spElementCollection);	//取得表单集合
    if (FAILED(hr))
        return;

    auto loginLabel = GetElement(spElementCollection, L"id", L"fxLogin");
    if (!loginLabel)
        return;

    loginLabel->click();
    WaitFor(50);

    auto username = GetElement(spElementCollection, L"name", L"username");
    if (username)
    {
        username->click();
        CString temp = userName;
        username->put_innerText(temp.GetBuffer());
    }
    auto password = GetElement(spElementCollection, L"name", L"password");
    if (password)
    {
        password->click();
        CString temp = pwd;
        password->put_innerText(temp.GetBuffer());
    }
    auto login_btn = GetElement(spElementCollection, L"name", L"login_btn");
    if (login_btn)
    {
        login_btn->click();
    }
}


void WebHandler::ClickXY(HWND hwnd, int x, int y)
{
    ::PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    ::PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
}

void WebHandler::RewardStar()
{
    CComQIPtr<IHTMLElementCollection> spElementCollection;
    HRESULT hr = htmlDocument_->get_all(&spElementCollection);	//取得表单集合
    if (FAILED(hr))
        return;

    auto sendStar = GetElement(spElementCollection, L"id", L"sendStar");
    if (!sendStar)
        return;

    VARIANT bstrAttr;
    hr = sendStar->getAttribute(L"title", 1, &bstrAttr);
    if (FAILED(hr))
        return;

    std::wstring strAttr = (LPCTSTR)bstrAttr.bstrVal;
    std::wstring strNum;
    for (int i = 0; i < (int)strAttr.length(); ++i)
    {
        wchar_t c = strAttr[i];
        if ((c >= L'0') && (c <= L'9'))
            strNum += c;
    }

    if (strNum.empty())
        return;

    int num = _wtoi(strNum.c_str());
    if (num <= 0)
        return;

    sendStar->click();
}

void WebHandler::RewardGift(const std::wstring& giftNames)
{
    auto names = SplitString(giftNames, L",");
    if (names.size() != 2)
        return;

    auto tabName = names[0];
    auto giftName = names[1];

    CComQIPtr<IHTMLElementCollection> spElementCollection;
    HRESULT hr = htmlDocument_->get_all(&spElementCollection);	//取得表单集合
    if (FAILED(hr))
        return;

    auto chosenGiftName = GetElement(spElementCollection, L"id", L"chosenGiftName");
    if (!chosenGiftName)
        return;

    chosenGiftName->click();
    WaitFor(50);

    auto giftTab = GetElement(spElementCollection, L"id", L"giftTab");
    if (!giftTab)
        return;

    CComPtr<IDispatch> giftTabDispatch;
    hr = giftTab->get_all(&giftTabDispatch);	//取得表单集合
    if (FAILED(hr))
        return;

    CComQIPtr<IHTMLElementCollection> giftTabCollection;
    hr = giftTabDispatch->QueryInterface(IID_IHTMLElementCollection, (VOID**)&giftTabCollection);
    if (FAILED(hr))
        return;

    auto selectTab = GetElement(giftTabCollection, L"type", tabName);
    if (!selectTab)
        return;

    selectTab->click();
    WaitFor(50);

    CComQIPtr<IHTMLElementCollection> giftItemCollection;
    hr = htmlDocument_->get_all(&giftItemCollection);	//取得表单集合
    if (FAILED(hr))
        return;

    auto giftItem = GetElement(giftItemCollection, L"_giftname", giftName);
    if (!giftItem)
        return;

    giftItem->click();
    WaitFor(50);

    auto sendGift = GetElement(giftItemCollection, L"class", L"button", L"赠送");
    if (!sendGift)
        return;

    sendGift->click();
}

void WebHandler::GetChatMessage()
{
    CComQIPtr<IHTMLElementCollection> allElements;
    HRESULT hr = S_FALSE;
    //hr = htmlDocument_->get_all(&allElements);	//取得表单集合
    //if (FAILED(hr))
    //    return;

    CComQIPtr<IHTMLElementCollection> scrpitsCollection;
    htmlDocument_->get_scripts(&scrpitsCollection);
    if (FAILED(hr))
        return;

    long nItemCount = 0;				//取得表单数目
    hr = scrpitsCollection->get_length(&nItemCount);
    if (FAILED(hr))
        return;

    BSTR pstr = SysAllocString(L"");;
    
    hr = scrpitsCollection->toString(&pstr);
    if (FAILED(hr))
        return;

    for (long i = 0; i < nItemCount; i++)
    {
        CComDispatchDriver spDisp;	//取得第 j 项表单域
        hr = scrpitsCollection->item(CComVariant(i), CComVariant(), &spDisp);
        if (FAILED(hr))
            continue;

        CComQIPtr<IHTMLElement> spElement = spDisp;
        if (!spElement)
            break;

        CComBSTR className;
        std::wstring compareStr;
        hr = spElement->get_className(&className);
        if (FAILED(hr))
            continue;
        if (className)
        {
            compareStr = (LPCTSTR)className;
        }
        
    }
    // 未成功
    //DISPID dispid;
    //OLECHAR FAR* funcName = L"click";
    //CComBSTR funNameStr(funcName);
    //hr = scrpitsCollection->GetIDsOfNames(IID_NULL, &funNameStr, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
    //if (FAILED(hr))
    //    return;
}
