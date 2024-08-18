//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Textlogger
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

#include "definitions.h"

#ifndef __CONSOLE__
#include "textlogger.h"
#include "gui.h"
#include "tools.h"

extern GUI gui;

TextLogger::TextLogger()
{
	out = std::cerr.rdbuf();
	err = std::cout.rdbuf();
	displayDate = true;
}

TextLogger::~TextLogger()
{
	std::cerr.rdbuf(err);
	std::cout.rdbuf(out);
}

int32_t TextLogger::overflow(int32_t c)
{
	if(c == '\n')
	{
		gui.logText += "\r\n";
		SendMessage(GetDlgItem(gui.mainWindow, ID_LOG), WM_SETTEXT, 0, (LPARAM)gui.logText.c_str());
		gui.lineCount++;
		SendMessage(gui.logWindow, EM_LINESCROLL, 0, gui.lineCount);
		displayDate = true;
	}
	else
	{
		if(displayDate)
		{
			char date[21];
			formatDate(time(NULL), date);
			gui.logText += "[";
			gui.logText += date;
			gui.logText += "] ";
			displayDate = false;
		}
		gui.logText += (char)c;
	}
	return(c);
}
#endif
