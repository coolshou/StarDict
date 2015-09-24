/*
 * Copyright 2011 kubtek <kubtek@mail.com>
 *
 * This file is part of StarDict.
 *
 * StarDict is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StarDict is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StarDict.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GetWord_H_
#define _GetWord_H_

#include <windows.h>

typedef enum TKnownWndClass {
	kwcUnknown,
	kwcRichEdit,
	kwcMultiLineEdit,
	kwcSingleLineEdit,
	kwcInternetExplorer_Server,
	kwcConsole,
} TKnownWndClass;

TKnownWndClass GetWindowType(HWND WND, const TCHAR* WNDClass);
/* return value in utf-8 */
char* TryGetWordFromAnyWindow(TKnownWndClass WndType, HWND WND, POINT Pt, int *BeginPos);

#endif
