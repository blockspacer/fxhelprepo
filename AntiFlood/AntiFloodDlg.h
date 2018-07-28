
// FanXingDlg.h : ͷ�ļ�
//

#pragma once
#include <memory>
#include <mutex>

#include "NetworkHelper.h"
#include "BlacklistHelper.h"

#include "afxwin.h"
#include "afxcmn.h"


// CFanXingDlg �Ի���
class CAntiFloodDlg : public CDialogEx
{
// ����
public:
    CAntiFloodDlg(CWnd* pParent = NULL);
    virtual ~CAntiFloodDlg();

// �Ի�������
    enum { IDD = IDD_FANXING_DIALOG };

    enum
    {
        WM_USER_01 = WM_USER + 1,
        WM_USER_ADD_ENTER_ROOM_INFO = WM_USER + 2,
        WM_USER_ADD_TO_BLACK_LIST = WM_USER + 3,
        WM_USER_ADD_RETRIVE_GIFT_COIN = WM_USER + 4,
    };
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
    // ���ɵ���Ϣӳ�亯��
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void OnClose();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
    
    afx_msg void OnBnClickedCancel();

    // �����������
    afx_msg void OnBnClickedButtonLogin();
    afx_msg void OnBnClickedButtonRewarstar();
    afx_msg void OnBnClickedButtonRewardgift();
    afx_msg void OnBnClickedButtonEnterRoom();
    afx_msg void OnBnClickedBtnGetmsg();
    afx_msg void OnBnClickedBtnClearInfo();
    afx_msg void OnBnClickedBtnSendChat();
    
    // �����б����
    afx_msg void OnBnClickedBtnAdd();
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnBnClickedBtnSelectAll();
    afx_msg void OnBnClickedBtnSelectReverse();
    afx_msg void OnBnClickedBtnKickoutMonth();
    afx_msg void OnBnClickedBtnKickoutHour();
    afx_msg void OnBnClickedBtnSilent();
    afx_msg void OnBnClickedBtnUnsilent();
    afx_msg void OnBnClickedBtnClear();
    afx_msg void OnBnClickedBtnGetViewerList();

    // �������б����
    afx_msg void OnBnClickedBtnKickoutMonthBlack();
    afx_msg void OnBnClickedBtnKickoutHourBlack();
    afx_msg void OnBnClickedBtnSilentBlack();
    afx_msg void OnBnClickedBtnUnsilentBlack();
    afx_msg void OnBnClickedBtnSelectAllBlack();
    afx_msg void OnBnClickedBtnSelectReverseBlack();
    afx_msg void OnBnClickedBtnRemoveBlack();
    afx_msg void OnBnClickedBtnLoadBlack();
    afx_msg void OnBnClickedBtnAddToBlack();
    afx_msg void OnBnClickedBtnSaveBlack();

    //�Զ�������Ϣ������
    afx_msg void OnBnClickedBtnSensitive(); // �������д�
    afx_msg void OnBnClickedBtnAddVest();   // �������
    afx_msg void OnBnClickedBtnRemoveVest();
    afx_msg void OnBnClickedRadioNoaction();
    afx_msg void OnBnClickedChkHandleAll();

    // ��������˲���
    afx_msg void OnBnClickedChkRobot();
    void UpdateRobotSetting();

    // �Զ���л����
    afx_msg void OnBnClickedChkThanks();
    void UpdateThanksSetting();
    afx_msg void OnBnClickedBtnThanksSetting();

    // �Զ���ӭ����
    afx_msg void OnBnClickedChkWelcome();
    void UpdateWelcomeSetting();
    afx_msg void OnBnClickedBtnWelcomeSetting();

    afx_msg void OnBnClickedChkCheckVipV(); // ������ӭ�������
    afx_msg void OnBnClickedChkPrivateNotify(); // ˽������:֪ͨ������ҽ���

    // ���⻶ӭ����
    afx_msg void OnBnClickedBtnAddWelcome();
    afx_msg void OnBnClickedBtnRemoveWelcome();

    // �Զ��ظ����Բ���
    afx_msg void OnBnClickedChkRepeatChat();
    void UpdateRepeatChatSetting();

    // ��ȡ�ֻ�ͬ������
    afx_msg void OnBnClickedBtnPhoneCityRank();

    // �����ͷ����
    afx_msg void OnHdnItemclickListUserStatus(NMHDR *pNMHDR, LRESULT *pResult);
    static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

    afx_msg void OnBnClickedBtnReceiveid();

    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayDataToViewerList(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayDtatToBlackList(WPARAM wParam, LPARAM lParam);
    LRESULT OnRetriveGiftCoin(WPARAM wParam, LPARAM lParam);

private:
    void SetHScroll();
    void Notify(MessageLevel level, const std::wstring& message);
    void NotifyEnterRoom(const RowData& rowdata);
    void NotifyRetriveGiftCoin(uint32 coin);
	bool LoginByRequest(const std::wstring& username,
		const std::wstring& password, const std::wstring& verifycode,
		const std::wstring& cookie, bool use_cookie);
    bool RefreshVerifyCode();
    bool GetSelectViewers(std::vector<EnterRoomUserInfo>* enterRoomUserInfos);
    bool GetSelectBlacks(std::vector<EnterRoomUserInfo>* enterRoomUserInfos);
    bool KickOut_(const std::vector<EnterRoomUserInfo>& enterRoomUserInfos,
        KICK_TYPE kicktype);
    bool BanChat_(const std::vector<EnterRoomUserInfo>& enterRoomUserInfos);
    bool UnbanChat_(const std::vector<EnterRoomUserInfo>& enterRoomUserInfos);
    bool SendChatMessage_(uint32 roomid, const std::wstring& message);

    HICON m_hIcon;
    CImage image;

    std::unique_ptr<NetworkHelper> network_;
    std::shared_ptr<AntiStrategy> antiStrategy_;
    std::shared_ptr<GiftStrategy> giftStrategy_;
    std::shared_ptr<EnterRoomStrategy> enterRoomStrategy_;

    std::mutex viewerRowdataMutex_;
    std::vector<RowData> viewerRowdataQueue_;

    std::mutex blackRowdataMutex_;
    std::vector<RowData> blackRowdataQueue_;

    uint32 listCtrlRowIndex_;
    CButton m_check_remember;
    uint32 roomid_ = 0;
    uint32 singerid_ = 0;
    std::wstring username_;

    CListBox InfoList_;
    int infoListCount_;
    CListCtrl m_ListCtrl_Viewers;
    CListCtrl m_ListCtrl_Blacks;
    std::unique_ptr<BlacklistHelper> blacklistHelper_;

    CStatic m_static_auth_info;
    CStatic m_static_login_info;

    CEdit m_edit_vest;
    CEdit m_edit_chatmsg;
    CListCtrl m_list_vest;
    int m_radiogroup;

    CButton m_chk_handle_all;

    CEdit m_edit_verifycode;
    CStatic m_static_verifycode;

    //�������������
    CButton m_chk_robot;
    CEdit m_edit_api_key;
 
    CButton m_chk_thanks;
    CComboBox m_combo_thanks;   // ��л�����Ǳ���

    CButton m_chk_welcome;
    CComboBox m_combo_welcome;  // ��ӭ��ҵȼ�
    CListCtrl m_list_user_strategy;

    CButton m_chk_repeat_chat;
    CComboBox m_combo_seconds;
    CEdit m_edit_auto_chat;   

    // ���дʴ���
    CEdit m_edit_sensitive;
    CComboBox m_combo_handle_level;

    CButton m_btn_add_sensitive;
    CButton m_btn_add_vest;
    CButton m_btn_remove_vest_sensitive;

    CButton m_chk_vip_v;
    CButton m_chk_private_notify;

    CButton m_btn_welcome_setting;
    CButton m_btn_thanks_setting;
    CEdit m_edit_retrive_gift_coin;
    CEdit m_edit_once_message;
    CEdit m_edit_receiveid;
    CEdit m_edit_cookie;
    CButton m_chk_use_cookie;
};
