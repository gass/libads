/*
    libads is an implementation of the Beckhoff's ADS protocol.

    libads is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libads is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with libads.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#ifdef LINUX
#include <unistd.h>
#endif

#include "ads.h"

AMSNetID me = { 172, 16, 17, 5, 1, 1 };
AMSNetID other = { 172, 16, 17, 1, 1, 1 };

int main(int argc, char **argv)
{
	ADSparseNetID("172.116.17.5.15.1", &me);
	printf("%d.%d\n", me.b4, me.b5);
	return 0;
}

/*
    Changes:
    
    05/12/2002 created
    23/12/2002 added sys/time.h
*/
