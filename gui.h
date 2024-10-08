//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// GUI
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#ifndef __FORGOTTENSERVER_GUI_H__
#define __FORGOTTENSERVER_GUI_H__

#ifndef __CONSOLE__
#include "playerbox.h"
#include "resources.h"

class GUI
{
	public:
		void initTrayMenu();
		void initFont();
		bool connections, minimized;
		PlayerBox pBox;
		HFONT font;
		HWND mainWindow, statusBar, logWindow;
		HMENU trayMenu;
		std::string logText;
		uint64_t lineCount;
};

#endif
#endif
