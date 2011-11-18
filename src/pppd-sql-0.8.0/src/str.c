/*
 *  str.c -- String handling functions for the Plugin.
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

/* generic includes. */
#include <string.h>
#include <ctype.h>

/* plugin includes. */
#include "str.h"

/* this function split the given string into tokens separated by delimiter. */
uint8_t *pppd__strsep(uint8_t **string_p, const uint8_t *delim) {

	/* some common variables. */
	uint8_t *start = *string_p;
	uint8_t *ptr;

	/* check if given string is a NULL pointer. */
	if (start == NULL) {
		return NULL;
	}

	/* optimize the case of no delimiters. */
	if (delim[0] == '\0') {
		*string_p = NULL;
		return start;
	}

	/* optimize the case of one delimiter. */
	if (delim[1] == '\0') {

		/* point to first character. */
		ptr = (uint8_t *)strchr((char *)start, delim[0]);
	} else {

		/* the general case. */
		ptr = (uint8_t *)strpbrk((char *)start, (char *)delim);
	}

	/* check if end reached. */
	if (ptr == NULL) {
		*string_p = NULL;
		return start;
	}

	/* add null terminator. */
	*ptr = '\0';
	*string_p = ptr + 1;

	/* return token. */
	return start;
}

/* this function convert a given hex value to an integer. */
int32_t pppd__htoi(uint8_t character) {

	/* check if source character is a numerical value. */
	if (character >= '0' && character <= '9') {
		return character - '0';
	}

	/* check if source character is a uppercase value. */
	if (character >= 'A' && character <= 'F') {
		return character - 'A' + 10;
	}

	/* check if source character is a lowercase value. */
	if (character >= 'a' && character <= 'f') {
		return character - 'a' + 10;
	}

	/* error on conversion. */
	return -1;
}
