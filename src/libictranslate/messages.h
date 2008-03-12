#ifndef __MESSAGES_H__
#define __MESSAGES_H__

// messages used by the app framework's modules
#define WM_TRAYNOTIFY	(WM_USER+0)
#define WM_CFGNOTIFY	(WM_USER+1)
#define WM_RMNOTIFY		(WM_USER+2)

// message routing
// types of routing
// sends a message everywhere it could be sent (hwnds, registered modules, ...)
#define ROT_EVERYWHERE		0x0000000000000000
// sends a message to all hwnds in an app
#define ROT_HWNDS			0x0100000000000000
// sends a message to all registered modules
#define ROT_REGISTERED		0x0200000000000000
// sends a message to one exact module
#define ROT_EXACT			0x0300000000000000

#endif
