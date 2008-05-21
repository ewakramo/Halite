
//         Copyright Eoin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define IDC_NAME_STATIC                 15012
#define IDC_TRACKER_STATIC              15013
#define IDC_STATUS_STATIC               15014
#define IDC_TIME_STATIC                 15015
#define IDC_COMPLETED_STATIC            15016

#ifndef RC_INVOKED

#include "stdAfx.hpp"
#include "DdxEx.hpp"
#include "HaliteSortListViewCtrl.hpp"
#include "HaliteDialogBase.hpp"
#include "HaliteEditCtrl.hpp"
#include "halIni.hpp"

#include "halTorrent.hpp"
#include "halEvent.hpp"

class ui_signal;

class HaliteDialog :
	public CDialogImpl<HaliteDialog>,
	public CAutoSizeWindow<HaliteDialog, false>,
	public CWinDataExchangeEx<HaliteDialog>,
	public CHaliteDialogBase<HaliteDialog>,
	private boost::noncopyable
{	
	typedef HaliteDialog thisClass;
	typedef CDialogImpl<thisClass> baseClass;
	typedef CAutoSizeWindow<thisClass, false> autosizeClass;
	typedef CHaliteDialogBase<thisClass> dialogBaseClass;
		
	class DialogListView :
		public CHaliteSortListViewCtrl<DialogListView, const hal::PeerDetail>,
		public hal::IniBase<DialogListView>,
		private boost::noncopyable
	{
	protected:
		typedef HaliteDialog::DialogListView thisClass;
		typedef hal::IniBase<thisClass> iniClass;
		typedef CHaliteSortListViewCtrl<DialogListView, const hal::PeerDetail> listClass;
		typedef const hal::PeerDetail pD;
	
		friend class listClass;
		
		struct ColumnAdapters
		{
		
		typedef listClass::ColumnAdapter ColAdapter_t;
		
		struct SpeedDown : public ColAdapter_t
		{
			virtual int compare(pD& l, pD& r)	{ return hal::compare(l.speed.first, r.speed.first); }		
			virtual std::wstring print(pD& p) 
			{
				return (wformat(L"%1$.2fkb/s") % (p.speed.first/1024)).str(); 
			}		
		};
		
		struct SpeedUp : public ColAdapter_t
		{
			virtual int compare(pD& l, pD& r)	{ return hal::compare(l.speed.second, r.speed.second); }		
			virtual std::wstring print(pD& p) 
			{
				return (wformat(L"%1$.2fkb/s") % (p.speed.second/1024)).str(); 
			}		
		};
		
		};
	
	public:	
		enum { 
			LISTVIEW_ID_MENU = 0,
			LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_COS,
			LISTVIEW_ID_COLUMNWIDTHS = HAL_DIALOGPEER_LISTVIEW_COS_DEFAULTS
		};
	
		BEGIN_MSG_MAP_EX(thisClass)
			MSG_WM_DESTROY(OnDestroy)
			CHAIN_MSG_MAP(listClass)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
	
		DialogListView() :
			iniClass("listviews/dialog", "DialogPeersList")
		{					
			std::vector<wstring> names;	
			wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

			// "Peer;Country;Download;Upload;Type;Client"
			boost::split(names, column_names, boost::is_any_of(L";"));
			
			array<int, 6> widths = {100,20,70,70,70,100};
			array<int, 6> order = {0,1,2,3,4,5};
			array<bool, 6> visible = {true,true,true,true,true,true};
			
			SetDefaults(names, widths, order, visible);
			Load();
		}
		
		void saveSettings()
		{
			GetListViewDetails();
			save();
		}
		
		bool SubclassWindow(HWND hwnd)
		{
			if(!parentClass::SubclassWindow(hwnd))
				return false;
				
			SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
			SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
			
			ApplyDetails();
			
			SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
			SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedUp());
				
			return true;
		}
		
		void OnDestroy()
		{
			saveSettings();
		}
		
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("listview", 
				boost::serialization::base_object<listClass>(*this));
		}
		
		pD CustomItemConversion(LVCompareParam* param, int iSortCol)
		{			
			return peerDetails_[param->dwItemData];
		}		
		
		void uiUpdate(const hal::TorrentDetails& tD);
		
	private:
		hal::PeerDetails peerDetails_;
	};
	
public:
	enum { IDD = IDD_HALITEDLG };

	HaliteDialog(HaliteWindow& HalWindow);
	BOOL PreTranslateMessage(MSG* pMsg)	{ return this->IsDialogMessage(pMsg); }

	void saveStatus();

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MESSAGE_HANDLER_EX(WM_USER_HAL_EDITCHANGED, OnHalEditChanged)
		
		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		COMMAND_ID_HANDLER_EX(BTNPAUSE, OnPause)
		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, OnReannounce)
		COMMAND_ID_HANDLER_EX(BTNREMOVE, OnRemove)

		CHAIN_MSG_MAP(dialogBaseClass)
		CHAIN_MSG_MAP(autosizeClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(thisClass)
    END_DDX_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	void uiUpdate(const hal::TorrentDetails& allTorrents); 
	void focusChanged(const hal::TorrentDetail_ptr pT);
	
protected:
	LRESULT OnInitDialog(HWND, LPARAM);
	void OnClose();

	void OnPause(UINT, int, HWND);
	void OnReannounce(UINT, int, HWND);
	void OnRemove(UINT, int, HWND);

	LRESULT OnCltColor(HDC hDC, HWND hWnd);	
	LRESULT OnHalEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CButton m_btn_start;
	DialogListView m_list;
	CProgressBarCtrl m_prog;
	
	CHaliteEditCtrl<int> totalConnections_;
	CHaliteEditCtrl<int> uploadConnections_;
	CHaliteEditCtrl<float> downloadRate_;
	CHaliteEditCtrl<float> uploadRate_;
	
	string current_torrent_name_;
};

#endif // RC_INVOKED
