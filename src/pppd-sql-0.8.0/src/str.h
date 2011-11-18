/*
 *  str.h -- String handling functions for the Plugin.
 *
 *  Copyright (c) 2008-2009 Maik Broemme <mbroemme@plusserver.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _STR_H
#define _STR_H

/* generic includes. */
#include <stdint.h>

/* this function split the given string into tokens separated by delimiter. */
uint8_t *pppd__strsep(
	uint8_t		**string_p,
	const uint8_t	*delim
);

/* this function convert a given hex value to an integer. */
int32_t pppd__htoi(
	uint8_t		character
);

#endif					/* _STR_H */
