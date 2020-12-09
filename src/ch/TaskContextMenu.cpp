#include "stdafx.h"
#include "TaskContextMenu.h"
#include "ch.h"
#include "resource.h"
#include "GuiOptions.h"

void TaskContextMenu::Load()
{
	HMENU hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_TASK_MENU));
	Attach(hMenu);
}

BOOL TaskContextMenu::TrackPopupMenu(chengine::ETaskCurrentState eState, UINT nFlags, int x, int y, CWnd* pWnd)
{
	CMenu* pSubMenu = GetSubMenu(0);

	pSubMenu->EnableMenuItem(ID_TASK_MENU_PAUSE, MF_BYCOMMAND | (GuiOptions::IsPauseAvailable(eState) ? MF_ENABLED : MF_GRAYED));
	pSubMenu->EnableMenuItem(ID_TASK_MENU_RESUME, MF_BYCOMMAND | (GuiOptions::IsResumeAvailable(eState) ? MF_ENABLED : MF_GRAYED));
	pSubMenu->EnableMenuItem(ID_TASK_MENU_RESTART, MF_BYCOMMAND | (GuiOptions::IsRestartAvailable(eState) ? MF_ENABLED : MF_GRAYED));
	pSubMenu->EnableMenuItem(ID_TASK_MENU_CANCEL, MF_BYCOMMAND | (GuiOptions::IsCancelAvailable(eState) ? MF_ENABLED : MF_GRAYED));
	pSubMenu->EnableMenuItem(ID_TASK_MENU_REMOVE, MF_BYCOMMAND | (GuiOptions::IsDeleteAvailable(eState) ? MF_ENABLED : MF_GRAYED));
	pSubMenu->EnableMenuItem(ID_TASK_MENU_RESET_FEEDBACK, MF_BYCOMMAND | (GuiOptions::IsResetUserFeedbackAvailable(eState) ? MF_ENABLED : MF_GRAYED));

	return pSubMenu->TrackPopupMenu(nFlags, x, y, pWnd);
}

