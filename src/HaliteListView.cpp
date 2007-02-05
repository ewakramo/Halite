﻿
#include "HaliteListView.hpp"

#include "GlobalIni.hpp"
#include "ini/Window.hpp"
#include "halTorrent.hpp"

void HaliteListViewCtrl::onShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

	CHeaderCtrl hdr = GetHeader();
	hdr.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);

	AddColumn(L"Name", hdr.GetItemCount());
	AddColumn(L"Status", hdr.GetItemCount());
	AddColumn(L"Completed", hdr.GetItemCount());
	AddColumn(L"Downloaded", hdr.GetItemCount());
	AddColumn(L"Upload", hdr.GetItemCount());
	AddColumn(L"Peers", hdr.GetItemCount());
	AddColumn(L"Seeds", hdr.GetItemCount());
	AddColumn(L"ETA", hdr.GetItemCount());
	AddColumn(L"Copies", hdr.GetItemCount());

	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		SetColumnWidth(i, INI().windowConfig().mainListColWidth[i]);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		SetColumnWidth(i+WindowConfig::numMainCols, INI().windowConfig().mainListColWidthEx[i]);
}

void HaliteListViewCtrl::updateListView()
{
	halite::TorrentDetails TD;
	halite::bittorrent().getAllTorrentDetails(TD);
	
	for (halite::TorrentDetails::const_iterator i = TD.begin(); i != TD.end(); ++i) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>((*i)->filename().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
			itemPos = AddItem(0, 0, (*i)->filename().c_str(), 0);
		
		SetItemText(itemPos, 1, (*i)->state().c_str());
		
		SetItemText(itemPos, 2, (wformat(L"%1$.2f%%") 
				% ((*i)->completion()*100)).str().c_str());
		
		SetItemText(itemPos, 3, (wformat(L"%1$.2fkb/s") 
				% ((*i)->speed().first/1024)).str().c_str());	
		
		SetItemText(itemPos, 4, (wformat(L"%1$.2fkb/s") 
				% ((*i)->speed().second/1024)).str().c_str());
		
		SetItemText(itemPos, 5,	(lexical_cast<wstring>((*i)->peers())).c_str());
		
		SetItemText(itemPos, 6,	(lexical_cast<wstring>((*i)->seeds())).c_str());	

		if (!(*i)->estimatedTimeLeft().is_special())
		{
			SetItemText(itemPos, 7,	(hal::to_wstr(
				boost::posix_time::to_simple_string((*i)->estimatedTimeLeft())).c_str()));
		}
		else
		{
			SetItemText(itemPos, 7,	L"∞");		
		}
		
		SetItemText(itemPos, 8,	(wformat(L"%1$.2f") 
				% ((*i)->distributedCopies())
			).str().c_str());	
	}	
}

void HaliteListViewCtrl::saveStatus()
{
	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		INI().windowConfig().mainListColWidth[i] = GetColumnWidth(i);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		INI().windowConfig().mainListColWidthEx[i] = GetColumnWidth(i+WindowConfig::numMainCols);
}
/*
LRESULT HaliteListViewCtrl::OnClick(int, LPNMHDR pnmh, BOOL&)
{
	manager().sync_list(true);

	return 0;
}

LRESULT HaliteListViewCtrl::OnRClick(int i, LPNMHDR pnmh, BOOL&)
{
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;	
	manager().sync_list(true);
	
	assert (torrentMenu_.IsMenu());
	CMenuHandle sMenu = torrentMenu_.GetSubMenu(0);
	assert (sMenu.IsMenu());
	
	POINT ptPoint;
	GetCursorPos(&ptPoint);
	sMenu.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnColClick(int i, LPNMHDR pnmh, BOOL&)
{
	LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;
//	MessageBox(lexical_cast<wstring>(pnlv->iSubItem).c_str(), L"ListView",0);
//	DeleteColumn(pnlv->iSubItem);

	return 0;
}
*/
LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::resumeTorrent, &halite::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::pauseTorrent, &halite::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::stopTorrent, &halite::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::removeTorrent, &halite::bittorrent(), _1));

	manager().clearAllSelected();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::removeTorrentWipeFiles, &halite::bittorrent(), _1));
	
	manager().clearAllSelected();
	return 0;
}


//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}