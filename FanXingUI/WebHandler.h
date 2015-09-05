#pragma once
#include <mshtml.h>

class WebHandler
{
public:
    WebHandler(IHTMLDocument2* htmlDocument);
    ~WebHandler();

    void Execute();
    void ClickXY(HWND hwnd, int x, int y);
    void RewardStar();
    void RewardGift(const std::wstring& giftNames);
    void GetChatMessage();

private:
    CComQIPtr<IHTMLElement> GetElement(
        CComQIPtr<IHTMLElementCollection> spElementCollection,
        const std::wstring& attrName, const std::wstring& attrValue,
        const std::wstring& textValue = L"");
    void WebHandler::WaitFor(int ms);

    IHTMLDocument2* htmlDocument_;
};

