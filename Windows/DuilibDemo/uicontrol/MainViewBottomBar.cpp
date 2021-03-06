#include "StdAfx.h"
#include "MainViewBottomBar.h"
#include "TRTCMainViewController.h"
#include "TRTCVideoViewLayout.h"
#include "DataCenter.h"
#include "TRTCCloudCore.h"
#include "TRTCSettingViewController.h"
#include "UiShareSelect.h"
#include "util/Base.h"
#include "util/log.h"
#include "MsgBoxWnd.h"
#include "TXLiveAvVideoView.h"
#include <Commdlg.h>
#include "UserMassegeIdDefine.h"

MainViewBottomBar::MainViewBottomBar(TRTCMainViewController * pMainWnd)
{
    m_pMainWnd = pMainWnd;
}

MainViewBottomBar::~MainViewBottomBar()
{
    m_pMainWnd->getPaintManagerUI().RemovePreMessageFilter(this);
}

void MainViewBottomBar::InitBottomUI()
{
    m_pMainWnd->getPaintManagerUI().AddPreMessageFilter(this);
}

void MainViewBottomBar::UnInitBottomUI()
{
    m_pMainWnd->getPaintManagerUI().RemovePreMessageFilter(this);

    if (m_pSettingWnd) {
        if (TRTCSettingViewController::getRef() > 0)
            m_pSettingWnd->preUnInit();
    }
}

void MainViewBottomBar::Notify(TNotifyUI& msg)
{
    if (msg.sType == _T("click"))
    {
        if (msg.pSender->GetName() == _T("btn_show_im") && m_pMainWnd)
        {
            CHorizontalLayoutUI* pIMAream = static_cast<CHorizontalLayoutUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("layout_im_meber_area")));
            int nImAreaWidth = 0;
            if (pIMAream) 
            {
                nImAreaWidth = pIMAream->GetFixedWidth();
            }
            if (nImAreaWidth > 0)
            {
                if (m_bOpenIMView)
                {
                    pIMAream->SetVisible(true);
                    SIZE sz = m_pMainWnd->getPaintManagerUI().GetClientSize();
                    sz.cx = sz.cx + nImAreaWidth;
                    if (m_pMainWnd)
                        m_pMainWnd->ResizeClient(sz.cx, sz.cy);
                }
                else
                {
                    pIMAream->SetVisible(false);
                    SIZE sz = m_pMainWnd->getPaintManagerUI().GetClientSize();
                    sz.cx = sz.cx - nImAreaWidth;
                    if (m_pMainWnd)
                        m_pMainWnd->ResizeClient(sz.cx, sz.cy);
                }
            }
        }
        else if (msg.pSender->GetName() == _T("btn_change_view_mode") && m_pMainWnd)
        {
            CButtonUI* pBtn = static_cast<CButtonUI*>(msg.pSender);
            if (pBtn)
            {
                if (m_bShowLectureModeUi)
                {
                    pBtn->SetForeImage(L"dest='4,1,20,19' source='0,0,16,18' res='videoview/gallery.png'");
                    pBtn->SetText(L"画廊视图");
                    m_bShowLectureModeUi = false;
                    if (m_pMainWnd && m_pMainWnd->getTRTCVideoViewLayout())
                        m_pMainWnd->getTRTCVideoViewLayout()->setLayoutStyle(ViewLayoutStyle_Lecture);
                }
                else
                {
                    m_bShowLectureModeUi = true;
                    pBtn->SetForeImage(L"dest='4,1,20,19' source='0,0,16,18' res='videoview/lecture.png'");
                    pBtn->SetText(L"演讲视图");
                    if (m_pMainWnd && m_pMainWnd->getTRTCVideoViewLayout())
                        m_pMainWnd->getTRTCVideoViewLayout()->setLayoutStyle(ViewLayoutStyle_Gallery);
                }
            }
        }
        else if (msg.pSender->GetName() == _T("btn_open_audio") && m_pMainWnd)
        {
            onClickMuteAudioBtn();
        }
        else if (msg.pSender->GetName() == _T("btn_open_video") && m_pMainWnd)
        {
            onClickMuteVideoBtn();
        }
        else if (msg.pSender->GetName() == _T("btn_open_log"))
        {
            m_showDashboardStyle++;
            int style = m_showDashboardStyle % 3;
            TXLiveAvVideoView::switchViewDashboardStyle((TXLiveAvVideoView::ViewDashboardStyleEnum)style);
            TRTCCloudCore::GetInstance()->showDashboardStyle(style);
        }
        else if (msg.pSender->GetName() == _T("btn_quit_room"))
        {
            if (m_pMainWnd)
                m_pMainWnd->DoExitRoom();
        }
        else if (msg.pSender->GetName() == _T("btn_audio_device"))
        {
            POINT point1;
            RECT rc = msg.pSender->GetClientPos();
            RECT winRc = { 0 };
            ::GetWindowRect(m_pMainWnd->GetHWND(), &winRc);
            point1.y = winRc.top + rc.top + 26;
            point1.x = winRc.left + rc.left + 5;
            CMenuWnd* pMenu = CMenuWnd::CreateMenu(NULL, _T("devicemenu.xml"), point1, &m_pMainWnd->getPaintManagerUI(), NULL, eMenuAlignment_Bottom);
            CMenuUI* rootMenu = pMenu->GetMenuUI();
            if (rootMenu != NULL)
            {
                // mic 
                {
                    //title
                    CMenuElementUI* pNewTabContainer = new CMenuElementUI;
                    pNewTabContainer->SetEnabled(false);
                    CHorizontalLayoutUI* pLayout = new CHorizontalLayoutUI();
                    CLabelUI * headerItem = new CLabelUI();
                    headerItem->SetText(L"选择麦克风");
                    headerItem->SetFont(0);
                    headerItem->SetTextPadding(RECT{ 10,0,0,0 });
                    headerItem->SetTextColor(0xFFE0E0E0);
                    pLayout->Add(headerItem);
                    pNewTabContainer->Add(pLayout);
                    rootMenu->Add(pNewTabContainer);
                    //list
                    std::vector<TRTCCloudCore::MediaDeviceInfo> micInfo = TRTCCloudCore::GetInstance()->getMicDevice();
                    for (auto info : micInfo)
                    {
                        CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                        pNewMenuElement->SetText(info._text.c_str());
                        pNewMenuElement->SetName(info._type.c_str());
                        if (info._select)
                        {
                            pNewMenuElement->SetIcon(L"file='menu/item_choose.png'");
                            pNewMenuElement->SetIconSize(12, 12);
                        }
                        rootMenu->Add(pNewMenuElement);
                    }
                }
                {   //line
                    CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                    pNewMenuElement->SetLineType();
                    pNewMenuElement->SetLinePadding(RECT{ 2,0,2,0 });
                    pNewMenuElement->SetFixedHeight(6);
                    pNewMenuElement->SetLineColor(0xFF707070);
                    rootMenu->Add(pNewMenuElement);
                }
                {   // speaker 
                    //title
                    CMenuElementUI* pNewTabContainer = new CMenuElementUI;
                    pNewTabContainer->SetEnabled(false);
                    CHorizontalLayoutUI* pLayout = new CHorizontalLayoutUI();
                    CLabelUI * headerItem = new CLabelUI();
                    headerItem->SetText(L"选择扬声器");
                    headerItem->SetFont(0);
                    headerItem->SetTextPadding(RECT{ 10,0,0,0 });
                    headerItem->SetTextColor(0xFFE0E0E0);
                    pLayout->Add(headerItem);
                    pNewTabContainer->Add(pLayout);
                    rootMenu->Add(pNewTabContainer);
                    //list
                    std::vector<TRTCCloudCore::MediaDeviceInfo> speakerInfo = TRTCCloudCore::GetInstance()->getSpeakDevice();
                    for (auto info : speakerInfo)
                    {
                        CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                        pNewMenuElement->SetText(info._text.c_str());
                        pNewMenuElement->SetName(info._type.c_str());
                        if (info._select)
                        {
                            pNewMenuElement->SetIcon(L"file='menu/item_choose.png'");
                            pNewMenuElement->SetIconSize(12, 12);
                        }
                        rootMenu->Add(pNewMenuElement);
                    }
                }
                {   //line
                    CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                    pNewMenuElement->SetLineType();
                    pNewMenuElement->SetLinePadding(RECT{ 2,0,2,0 });
                    pNewMenuElement->SetFixedHeight(0);
                    pNewMenuElement->SetLineColor(0xFF707070);
                    rootMenu->Add(pNewMenuElement);
                }
                {   //设置中心
                    CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                    pNewMenuElement->SetText(L"音频设置");
                    rootMenu->Add(pNewMenuElement);
                }
            }
            // 动态添加后重新设置菜单的大小
            pMenu->ResizeMenu();
        }
        else if (msg.pSender->GetName() == _T("btn_video_device"))
        {
            POINT point1;
            RECT rc = msg.pSender->GetClientPos();
            RECT winRc = { 0 };
            ::GetWindowRect(m_pMainWnd->GetHWND(), &winRc);
            point1.y = winRc.top + rc.top + 26;
            point1.x = winRc.left + rc.left + 5;
            CMenuWnd* pMenu = CMenuWnd::CreateMenu(NULL, _T("devicemenu.xml"), point1, &m_pMainWnd->getPaintManagerUI(), NULL, eMenuAlignment_Bottom);
            CMenuUI* rootMenu = pMenu->GetMenuUI();
            if (rootMenu != NULL)
            {
                // camera 
                {
                    //title
                    CMenuElementUI* pNewTabContainer = new CMenuElementUI;
                    pNewTabContainer->SetEnabled(false);
                    CHorizontalLayoutUI* pLayout = new CHorizontalLayoutUI();
                    CLabelUI * headerItem = new CLabelUI();
                    headerItem->SetText(L"选择摄像头");
                    headerItem->SetFont(0);
                    headerItem->SetTextPadding(RECT{ 10,0,0,0 });
                    headerItem->SetTextColor(0xFFE0E0E0);
                    pLayout->Add(headerItem);
                    pNewTabContainer->Add(pLayout);
                    rootMenu->Add(pNewTabContainer);
                    //list
                    std::vector<TRTCCloudCore::MediaDeviceInfo> micInfo = TRTCCloudCore::GetInstance()->getCameraDevice();
                    for (auto info : micInfo)
                    {
                        CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                        pNewMenuElement->SetText(info._text.c_str());
                        pNewMenuElement->SetName(info._type.c_str());
                        if (info._select)
                        {
                            pNewMenuElement->SetIcon(L"file='menu/item_choose.png'");
                            pNewMenuElement->SetIconSize(12, 12);
                        }
                        rootMenu->Add(pNewMenuElement);
                    }
                }
                {   //line
                    CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                    pNewMenuElement->SetLineType();
                    pNewMenuElement->SetLinePadding(RECT{ 2,0,2,0 });
                    pNewMenuElement->SetFixedHeight(6);
                    pNewMenuElement->SetLineColor(0xFF707070);
                    rootMenu->Add(pNewMenuElement);
                }
                {   //设置中心
                    CMenuElementUI* pNewMenuElement = new CMenuElementUI;
                    pNewMenuElement->SetText(L"视频设置");
                    rootMenu->Add(pNewMenuElement);
                }
            }
            // 动态添加后重新设置菜单的大小
            pMenu->ResizeMenu();
        }
		else if (msg.pSender->GetName() == _T("btn_open_screen")) {
			if (m_bOpenScreen) {
				CButtonUI* pBtn = static_cast<CButtonUI*>(msg.pSender);
				if (pBtn){
                    TRTCScreenCaptureSourceInfo info{};
                    info.type = TRTCScreenCaptureSourceTypeUnknown;
					OpenScreenBtnEvent(pBtn, info);
                }
			}
			else {
				UiShareSelect uiShareSelect;
				uiShareSelect.Create(m_pMainWnd->GetHWND(), _T("选择分享内容"), UI_WNDSTYLE_DIALOG, 0);
				uiShareSelect.CenterWindow();
				UINT nRet = uiShareSelect.ShowModal();
				if (nRet == IDOK)
				{
					TRTCScreenCaptureSourceInfo info = uiShareSelect.getSelectWnd();
					CButtonUI* pBtn = static_cast<CButtonUI*>(msg.pSender);
					if (pBtn)
						OpenScreenBtnEvent(pBtn, info);
				}
			}		
		}
		else if (msg.pSender->GetName() == _T("btn_play")) {
			CButtonUI* pBtn = static_cast<CButtonUI*>(msg.pSender);
			if (!pBtn) return;
			if (m_bPlay)
			{				
				TRTCCloudCore::GetInstance()->stopMedia();
			}
			else
			{				
				OPENFILENAME ofn;       // common dialog box structure
				wchar_t szFile[260];       // buffer for file name
				//HANDLE hf;              // file handle
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_pMainWnd->GetHWND();
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn) == TRUE) {
					TRTCCloudCore::GetInstance()->startMedia(Wide2Ansi(ofn.lpstrFile).c_str(), NULL);
					
				}
			}
		}
		else if (msg.pSender->GetName() == _T("btn_green_screen")) {
			CButtonUI* pBtn = static_cast<CButtonUI*>(msg.pSender);
			if (!pBtn) return;
			if (m_bGreenScreen)
			{
				//pBtn->SetForeImage(L"dest='22,3,52,33' source='0,0,30,30' res='bottom/camera_close.png'");
				pBtn->SetText(L"启动绿幕");
				m_bGreenScreen = false;
				//TRTCCloudCore::GetInstance()->stopGreenScreen();
			}
			else
			{
				m_bGreenScreen = true;
				//pBtn->SetForeImage(L"dest='22,3,52,33' source='0,0,30,30' res='bottom/camera_open.png'");
				pBtn->SetText(L"关闭绿幕");
				OPENFILENAME ofn;       // common dialog box structure
				wchar_t szFile[260];       // buffer for file name
				HANDLE hf;              // file handle
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_pMainWnd->GetHWND();
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (GetOpenFileName(&ofn) == TRUE) {
					//TRTCCloudCore::GetInstance()->startGreenScreen(Wide2Ansi(ofn.lpstrFile));
				}
			}
		}
        else if (msg.pSender->GetName() == _T("btn_open_pkview"))
        {
            CHorizontalLayoutUI* _pPKLayout = static_cast<CHorizontalLayoutUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("layout_container_pkview")));
            if (_pPKLayout)
                _pPKLayout->SetVisible(true);
            CLabelUI* pStatus = static_cast<CLabelUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("label_pkview_status")));
            if (pStatus) pStatus->SetText(L"");
            std::vector<PKUserInfo>& pkList = CDataCenter::GetInstance()->m_vecPKUserList;
            CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_pkview_stop")));
            if (pBtn)
            {
                if (pkList.size() > 0)
                    pBtn->SetEnabled(true);
                else
                    pBtn->SetEnabled(false);
            }
        }
        else if (msg.pSender->GetName() == _T("btn_close_pkview"))
        {
            CHorizontalLayoutUI* _pPKLayout = static_cast<CHorizontalLayoutUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("layout_container_pkview")));
            if (_pPKLayout)
                _pPKLayout->SetVisible(false);

        }
        else if (msg.pSender->GetName() == _T("btn_pkview_start"))
        {
            CLabelUI* pStatus = static_cast<CLabelUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("label_pkview_status")));
            CEditUI* pEditRoomID = static_cast<CEditUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("edit_pk_roomid")));
            CEditUI* pEditUserID = static_cast<CEditUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("edit_pk_userid")));
            if (pEditRoomID != nullptr && pEditUserID != nullptr && pStatus != nullptr)
            {
                m_pkRoomId = pEditRoomID->GetText();
                m_pkUserId = pEditUserID->GetText();
                if (m_pkRoomId.compare(L"") == 0)
                {
                    if (pStatus != nullptr)
                        pStatus->SetText(L"房间号不能为空");
                    return;
                }
                if (m_pkUserId.compare(L"") == 0)
                {
                    if (pStatus != nullptr)
                        pStatus->SetText(L"房间号不能为空");
                    return;
                }
                TRTCCloudCore::GetInstance()->connectOtherRoom(Wide2Ansi(m_pkUserId), _wtoi(m_pkRoomId.c_str()));

                std::wstring statusText = format(L"连接房间[%s]中...", m_pkUserId.c_str());
                pStatus->SetText(statusText.c_str());
                msg.pSender->SetEnabled(false);
            }
        }
        else if (msg.pSender->GetName() == _T("btn_pkview_stop"))
        {
            TRTCCloudCore::GetInstance()->getTRTCCloud()->disconnectOtherRoom();
        }
    }
}

LRESULT MainViewBottomBar::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool & bHandled)
{
    if (uMsg == WM_MENUCLICK)
    {
        MenuCmd* pMenuCmd = (MenuCmd*)wParam;
        BOOL bChecked = pMenuCmd->bChecked;
        CDuiString strMenuName = pMenuCmd->szName;
        CDuiString sUserData = pMenuCmd->szUserData;
        CDuiString sText = pMenuCmd->szText;
        m_pMainWnd->getPaintManagerUI().DeletePtr(pMenuCmd);
        if (strMenuName == _T("mic"))
            TRTCCloudCore::GetInstance()->selectMicDevice(sText.GetData());
        else if (strMenuName == _T("speaker"))
            TRTCCloudCore::GetInstance()->selectSpeakerDevice(sText.GetData());
        else if (strMenuName == _T("camera"))
            TRTCCloudCore::GetInstance()->selectCameraDevice(sText.GetData());
        else if (sText == _T("音频设置"))
        {
            if (m_pSettingWnd) {
                if (TRTCSettingViewController::getRef() > 0)
                    m_pSettingWnd->Close(ID_CLOSE_WINDOW_NO_QUIT_MSGLOOP);
                m_pSettingWnd = nullptr;
            }
            m_pSettingWnd = new TRTCSettingViewController(TRTCSettingViewController::SettingTag_Audio, m_pMainWnd->GetHWND());
            m_pSettingWnd->Create(m_pMainWnd->GetHWND(), _T("TRTCDuilibDemo"), WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, WS_EX_WINDOWEDGE);
            //pSetting->Create(GetHWND(), _T("设置"), WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW);
            m_pSettingWnd->CenterWindow();
            m_pSettingWnd->ShowWindow(true);
        }
        else if (sText == _T("视频设置"))
        {
            if (m_pSettingWnd) {
                if (TRTCSettingViewController::getRef() > 0)
                    m_pSettingWnd->Close(ID_CLOSE_WINDOW_NO_QUIT_MSGLOOP);
                m_pSettingWnd = nullptr;
            }
            m_pSettingWnd = new TRTCSettingViewController(TRTCSettingViewController::SettingTag_Video, m_pMainWnd->GetHWND());
            m_pSettingWnd->Create(m_pMainWnd->GetHWND(), _T("TRTCDuilibDemo"), WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, WS_EX_WINDOWEDGE);
            //pSetting->Create(GetHWND(), _T("设置"), WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW);
            m_pSettingWnd->CenterWindow();
            m_pSettingWnd->ShowWindow(true);
 
        }
    }
    else if (uMsg == WM_USER_CMD_DeviceChange)
    {
        TRTCDeviceType type = (TRTCDeviceType)wParam;
        TRTCDeviceState eventCode = (TRTCDeviceState)lParam;
        if (type == TRTCDeviceTypeCamera)
        {
            RefreshVideoDevice();
        }
        if (type == TRTCDeviceTypeMic)
        {
            RefreshAudioDevice();
        }
        if (type == TRTCDeviceTypeSpeaker)
        {

        }
    }
    else if (uMsg == ID_DELAY_SHOW_MSGBOX)
    {
        std::wstring * text = (std::wstring *)wParam;
        CMsgWnd::ShowMessageBox(m_pMainWnd->GetHWND(), _T("TRTCDuilibDemo"), text->c_str(), 0xFFF08080);
        delete text;
        text = nullptr;
    }
	else if (uMsg == WM_USER_CMD_ScreenStart)
	{
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_open_screen")));
		if (pBtn) {
			pBtn->SetForeImage(L"dest='24,4,50,30' source='0,0,26,26' res='bottom/screen_share_start.png'");
			pBtn->SetText(L"关闭分享");
		}
		m_bOpenScreen = true;		
		CDataCenter::LocalUserInfo info = CDataCenter::GetInstance()->getLocalUserInfo();
		m_pMainWnd->getTRTCVideoViewLayout()->dispatchVideoView(Ansi2Wide(info._userId), TRTCVideoStreamType::TRTCVideoStreamTypeSub);
	}
	else if (uMsg == WM_USER_CMD_ScreenEnd)
	{
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_open_screen")));
		if (pBtn) {
            pBtn->SetForeImage(L"dest='24,4,50,30' source='0,0,26,26' res='bottom/screen_share_normal.png'");
			pBtn->SetText(L"启动分享");
		}
		m_bOpenScreen = false;
		CDataCenter::LocalUserInfo info = CDataCenter::GetInstance()->getLocalUserInfo();
		m_pMainWnd->getTRTCVideoViewLayout()->deleteVideoView(Ansi2Wide(info._userId), TRTCVideoStreamType::TRTCVideoStreamTypeSub);
	}
	else if (uMsg == WM_USER_CMD_VodStart)
	{
		m_bPlay = true;
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_play")));
		if (pBtn) {
			//pBtn->SetForeImage(L"dest='22,3,52,33' source='0,0,30,30' res='bottom/camera_open.png'");
			pBtn->SetText(L"关闭播片");
		}
        CDataCenter::LocalUserInfo info = CDataCenter::GetInstance()->getLocalUserInfo();
		m_pMainWnd->getTRTCVideoViewLayout()->dispatchVideoView(Ansi2Wide(info._userId), TRTCVideoStreamType::TRTCVideoStreamTypeSub);

	}
	else if (uMsg == WM_USER_CMD_VodEnd)
	{
		CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_play")));
		if (pBtn) {
			//pBtn->SetForeImage(L"dest='22,3,52,33' source='0,0,30,30' res='bottom/camera_close.png'");
			pBtn->SetText(L"启动播片");
		}		
		m_bPlay = false;
        CDataCenter::LocalUserInfo info = CDataCenter::GetInstance()->getLocalUserInfo();
		m_pMainWnd->getTRTCVideoViewLayout()->deleteVideoView(Ansi2Wide(info._userId), TRTCVideoStreamType::TRTCVideoStreamTypeSub);
	}
    return 0;
}

void MainViewBottomBar::RefreshVideoDevice()
{
    std::wstring selectOldDevice = CDataCenter::GetInstance()->m_selectCamera;
    bool _bMuteVideo = CDataCenter::GetInstance()->m_loginInfo._bMuteVideo;
    std::vector<TRTCCloudCore::MediaDeviceInfo> vecDevice = TRTCCloudCore::GetInstance()->getCameraDevice();
    std::wstring selectNewDevice = L"Unknow";
    for (auto info : vecDevice){
        if (info._select) {
            selectNewDevice = info._text; break;
        }
    }

    bool bReSelectDevice = false;
    //没有设备变成有设备
    if (selectOldDevice.compare(L"") == 0 && _bMuteVideo)
    {
        onClickMuteVideoBtn();
        bReSelectDevice = true;
    }

    //选择设备被删除了
    if (selectOldDevice.compare(selectNewDevice) != 0)
        bReSelectDevice = true;

    //有设备变成没设备
    if (!_bMuteVideo && vecDevice.size() <= 0)
    {
        onClickMuteVideoBtn();
        bReSelectDevice = true;
    }
    if (bReSelectDevice)
    {
        for (auto info : vecDevice)
        {
            if (info._select) {
                TRTCCloudCore::GetInstance()->selectCameraDevice(info._text); break;
            }
        }
    }
}

void MainViewBottomBar::RefreshAudioDevice()
{
    std::wstring selectOldDevice = CDataCenter::GetInstance()->m_selectMic;
    bool _bMuteAudio = CDataCenter::GetInstance()->m_loginInfo._bMuteAudio;
    std::vector<TRTCCloudCore::MediaDeviceInfo> vecDevice = TRTCCloudCore::GetInstance()->getMicDevice();
    std::wstring selectNewDevice = L"Unknow";
    for (auto info : vecDevice) {
        if (info._select) {
            selectNewDevice = info._text; break;
        }
    }

    bool bReSelectDevice = false;
    //没有设备变成有设备
    if (selectOldDevice.compare(L"") == 0 && _bMuteAudio)
    {
        onClickMuteAudioBtn();
        bReSelectDevice = true;
    }

    //选择设备被删除了
    if (selectOldDevice.compare(selectNewDevice) != 0)
        bReSelectDevice = true;

    //有设备变成没设备
    if (!_bMuteAudio && vecDevice.size() <= 0)
    {
        onClickMuteAudioBtn();
        bReSelectDevice = true;
    }
    if (bReSelectDevice)
    {
        for (auto info : vecDevice)
        {
            if (info._select) {
                TRTCCloudCore::GetInstance()->selectMicDevice(info._text); break;
            }
        }
    }
}

void MainViewBottomBar::muteLocalVideoBtn(bool bMute)
{
    CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_open_video")));
    if (pBtn == nullptr)
        return;
    if (bMute)
    {
        pBtn->SetForeImage(L"dest='24,4,50,30' source='0,0,26,26' res='bottom/camera_mute.png'");
        pBtn->SetText(L"启动视频"); 
    }
    else
    {
        pBtn->SetForeImage(L"dest='24,4,50,30' source='0,0,26,26' res='bottom/camera_start.png'");
        pBtn->SetText(L"关闭视频");
    }
}

void MainViewBottomBar::muteLocalAudioBtn(bool bMute)
{
    CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_open_audio")));
    if (pBtn == nullptr)
        return;
    if (bMute)
    {
        pBtn->SetForeImage(L"dest='24,4,50,30' source='0,0,26,26' res='bottom/audio_mute.png'");
        pBtn->SetText(L"解除静音");
    }
    else
    {
        pBtn->SetForeImage(L"dest='24,4,50,30' source='0,0,26,26' res='bottom/audio_start.png'");
        pBtn->SetText(L"静音");
    }
}

bool MainViewBottomBar::onPKUserLeaveRoom(std::string userId)
{
    std::vector<PKUserInfo>& pkList = CDataCenter::GetInstance()->m_vecPKUserList;
    std::vector<PKUserInfo>::iterator result;
    for (result = pkList.begin(); result != pkList.end(); result++)
    {
        if (result->_userId.compare(userId.c_str()) == 0)
        {
            pkList.erase(result);
            std::string localUserId = CDataCenter::GetInstance()->getLocalUserID();
            CDuiString strFormat;
            strFormat.Format(L"%s连麦用户[%s]离开房间", Log::_GetDateTimeString().c_str(), Ansi2Wide(userId).c_str());
            TXLiveAvVideoView::appendEventLogText(localUserId, TRTCVideoStreamTypeBig, strFormat.GetData());
            return true;
        }
    }
    return false;
}

bool MainViewBottomBar::onPKUserEnterRoom(std::string userId, uint32_t& roomId)
{
    std::vector<PKUserInfo>& pkList = CDataCenter::GetInstance()->m_vecPKUserList;
    std::vector<PKUserInfo>::iterator result;
    for (result = pkList.begin(); result != pkList.end(); result++)
    {
        if (result->_userId.compare(userId.c_str()) == 0)
        {
            result->bEnterRoom = true;
            roomId = result->_roomId;
            std::string localUserId = CDataCenter::GetInstance()->getLocalUserID();
            CDuiString strFormat;
            strFormat.Format(L"%s连麦用户[%s]进入房间", Log::_GetDateTimeString().c_str(), Ansi2Wide(userId).c_str());
            TXLiveAvVideoView::appendEventLogText(localUserId, TRTCVideoStreamTypeBig, strFormat.GetData());
            return true;
        }
    }
    return false;
}

void MainViewBottomBar::onConnectOtherRoom(int errCode, std::string errMsg)
{
    CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_pkview_start")));
    if (pBtn == nullptr)
        return;
    pBtn->SetEnabled(true);
    std::string localUserId = CDataCenter::GetInstance()->getLocalUserID();
    CDuiString strFormat;
    if (errCode == 0)
    {
        PKUserInfo info;
        info._userId = Wide2Ansi(m_pkUserId);
        info._roomId = _wtoi(m_pkRoomId.c_str());

        std::wstring statusText = format(L"连麦成功:[room:%d, user:%s]", info._roomId, m_pkUserId.c_str());
        CLabelUI* pStatus = static_cast<CLabelUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("label_pkview_status")));
        if (pStatus)
        pStatus->SetText(statusText.c_str());

        strFormat.Format(L"%s连麦成功[room:%d, user:%s])", Log::_GetDateTimeString().c_str(), info._roomId, m_pkUserId.c_str());
        TXLiveAvVideoView::appendEventLogText(localUserId, TRTCVideoStreamTypeBig, strFormat.GetData());

        std::vector<PKUserInfo>& pkList = CDataCenter::GetInstance()->m_vecPKUserList;
        pkList.push_back(info);
        CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_pkview_stop")));
        if (pBtn)
            pBtn->SetEnabled(true);
    }
    else
    {
        CLabelUI* pStatus = static_cast<CLabelUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("label_pkview_status")));
        if (pStatus != nullptr)
        {
            std::wstring statusText = format(L"连麦失败,errCode:%d", errCode);
            pStatus->SetText(statusText.c_str());
        }
        strFormat.Format(L"%s连麦失败[userId:%s, roomId:%s, errCode:%d, msg:%s]", m_pkUserId.c_str(), m_pkRoomId.c_str() ,Log::_GetDateTimeString().c_str(), errCode, UTF82Wide(errMsg).c_str());
        TXLiveAvVideoView::appendEventLogText(localUserId, TRTCVideoStreamTypeBig, strFormat.GetData());
    }
}

void MainViewBottomBar::onDisconnectOtherRoom(int errCode, std::string errMsg)
{
    std::string localUserId = CDataCenter::GetInstance()->getLocalUserID();
    CDuiString strFormat;
    if (errCode == 0)
    {
        CLabelUI* pStatus = static_cast<CLabelUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("label_pkview_status")));
        if (pStatus != nullptr)
            pStatus->SetText(L"取消连麦成功");
        CButtonUI* pBtn = static_cast<CButtonUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("btn_pkview_stop")));
        if (pBtn)
            pBtn->SetEnabled(false);
        std::vector<PKUserInfo>& pkList = CDataCenter::GetInstance()->m_vecPKUserList;
        pkList.clear();
        strFormat.Format(L"%s取消连麦成功[msg:%s]", Log::_GetDateTimeString().c_str(), UTF82Wide(errMsg).c_str());
        TXLiveAvVideoView::appendEventLogText(localUserId, TRTCVideoStreamTypeBig, strFormat.GetData());
    }
    else
    {
        std::wstring statusText = format(L"取消连麦失败,errCode:%d", errCode);
        CLabelUI* pStatus = static_cast<CLabelUI*>(m_pMainWnd->getPaintManagerUI().FindControl(_T("label_pkview_status")));
        if (pStatus)
            pStatus->SetText(statusText.c_str());

        strFormat.Format(L"%s取消连麦失败:[room:%d, user:%s])", Log::_GetDateTimeString().c_str(), errCode, UTF82Wide(errMsg).c_str());
        TXLiveAvVideoView::appendEventLogText(localUserId, TRTCVideoStreamTypeBig, strFormat.GetData());
    }
}

void MainViewBottomBar::onClickMuteVideoBtn()
{
    UI_EVENT_MSG *msg = new UI_EVENT_MSG;
    msg->_id = UI_EVENT_MSG::UI_BTNMSG_ID_MuteVideo;
    msg->_userId = Ansi2Wide(CDataCenter::GetInstance()->getLocalUserID());
    msg->_streamType = TRTCVideoStreamTypeBig;
    ::PostMessage(m_pMainWnd->GetHWND(), WM_USER_VIEW_BTN_CLICK, (WPARAM)msg, 0);
}

void MainViewBottomBar::onClickMuteAudioBtn()
{
    UI_EVENT_MSG *msg = new UI_EVENT_MSG;
    msg->_id = UI_EVENT_MSG::UI_BTNMSG_ID_MuteAudio;
    msg->_userId = Ansi2Wide(CDataCenter::GetInstance()->getLocalUserID());
    msg->_streamType = TRTCVideoStreamTypeBig;
    ::PostMessage(m_pMainWnd->GetHWND(), WM_USER_VIEW_BTN_CLICK, (WPARAM)msg, 0);
}

void MainViewBottomBar::OpenScreenBtnEvent(CButtonUI * pBtn, const TRTCScreenCaptureSourceInfo &source)
{
	if (m_bOpenScreen)
	{		
		TRTCCloudCore::GetInstance()->stopScreen();
	}
	else
	{
		RECT rect;
		rect.bottom = 0;
		rect.left = 0;
		rect.right = 0;
		rect.top = 0;
		TRTCCloudCore::GetInstance()->selectScreenCaptureTarget(source, rect);
		TRTCCloudCore::GetInstance()->startScreen(NULL);
	}
}
