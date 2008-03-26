/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef __PROPERTYTYPES_H__
#define __PROPERTYTYPES_H__

#pragma once

#include "../libicpf/cfg.h"

// properties definitions
#define PP_PCLIPBOARDMONITORING		0
#define PP_PMONITORSCANINTERVAL		1
#define PP_PRELOADAFTERRESTART		2
#define PP_PSHUTDOWNAFTREFINISHED	3
#define PP_PTIMEBEFORESHUTDOWN		4
#define PP_PFORCESHUTDOWN			5
#define PP_PAUTOSAVEINTERVAL		6
#define PP_PPROCESSPRIORITYCLASS	7
#define PP_PAUTOSAVEDIRECTORY		8
#define PP_PLANGUAGE				9

#define PP_STATUSREFRESHINTERVAL	10
#define PP_STATUSSHOWDETAILS		11
#define PP_STATUSAUTOREMOVEFINISHED	12

#define PP_FDWIDTH					13
#define PP_FDHEIGHT					14
#define PP_FDSHORTCUTLISTSTYLE		15
#define PP_FDEXTENDEDVIEW			16
#define PP_FDIGNORESHELLDIALOGS		17

#define PP_MVSHOWFILENAMES			18
#define PP_MVSHOWSINGLETASKS		19
#define PP_MVREFRESHINTERVAL		20
#define PP_MVAUTOSHOWWHENRUN		21
#define PP_MVAUTOHIDEWHENEMPTY		22
#define PP_MVUSESMOOTHPROGRESS		23

#define PP_CMUSEAUTOCOMPLETEFILES	24
#define PP_CMSETDESTATTRIBUTES		25
#define PP_CMSETDESTDATE			26
#define PP_CMPROTECTROFILES			27
#define PP_CMLIMITMAXOPERATIONS		28
#define PP_CMREADSIZEBEFOREBLOCKING	29
#define PP_CMSHOWVISUALFEEDBACK		30
#define PP_CMUSETIMEDFEEDBACK		31
#define PP_CMFEEDBACKTIME			32
#define PP_CMAUTORETRYONERROR		33
#define PP_CMAUTORETRYINTERVAL		34
#define PP_CMDEFAULTPRIORITY		35
#define PP_CMDISABLEPRIORITYBOOST	36
#define PP_CMDELETEAFTERFINISHED	37
#define PP_CMCREATELOG				38

#define PP_SHSHOWCOPY				39
#define PP_SHSHOWMOVE				40
#define PP_SHSHOWCOPYMOVE			41
#define PP_SHSHOWPASTE				42
#define PP_SHSHOWPASTESPECIAL		43
#define PP_SHSHOWCOPYTO				44
#define PP_SHSHOWMOVETO				45
#define PP_SHSHOWCOPYMOVETO			46
#define PP_SHSHOWFREESPACE			47
#define PP_SHSHOWSHELLICONS			48
#define PP_SHUSEDRAGDROP			49
#define PP_SHDEFAULTACTION			50

#define PP_BFUSEONLYDEFAULT			51
#define PP_BFDEFAULT				52
#define PP_BFONEDISK				53
#define PP_BFTWODISKS				54
#define PP_BFCD						55
#define PP_BFLAN					56
#define PP_BFUSENOBUFFERING			57
#define PP_BFBOUNDARYLIMIT			58

#define PP_LOGPATH					59
#define PP_LOGENABLELOGGING			60
#define PP_LOGLIMITATION			61
#define PP_LOGMAXLIMIT				62
#define PP_LOGPRECISELIMITING		63
#define PP_LOGTRUNCBUFFERSIZE		64

#define PP_SNDPLAYSOUNDS			65
#define PP_SNDERRORSOUNDPATH		66
#define PP_SNDFINISHEDSOUNDPATH		67

#define PP_SHORTCUTS				68
#define PP_RECENTPATHS				69

// register function
bool RegisterProperties(icpf::config* pManager);

#endif