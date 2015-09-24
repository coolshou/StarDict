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

#ifndef _STARDICT_LIBCONFIG_H_
#define _STARDICT_LIBCONFIG_H_

//stardict
#define SD_STANDARD_EDITION

//stardictd
//#define SD_SERVER_EDITION


#ifdef SD_STANDARD_EDITION
#define SD_CLIENT_CODE
#undef SD_SERVER_CODE
#endif

#ifdef SD_SERVER_EDITION
#undef SD_CLIENT_CODE
#define SD_SERVER_CODE
#endif

#endif
