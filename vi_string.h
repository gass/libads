/*
 A single include file for different string implementations. This was
 introduced to ease change from one string implementation to another.

 Part of VISUAL, a human machine interface and data acquisition program
 
 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.

 VISUAL is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 VISUAL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Visual; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  
*/
#ifndef use_string_2
#include <string>
using std::string;
#define from(x) substr(x)
#define after(x) substr(1+x)
#define before(x) substr(0,x)
#define Find find  
#else
#include <String.h>
#define erase del  
#define Find index  
#define substr(x,y) at(x,y)
#define string String
#define c_str chars
#define getline readline
#endif
