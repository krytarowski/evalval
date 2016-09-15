/* $NetBSD$ */

/*-
 * Copyright (c) 2016 Kamil Rytarowski
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD$");

#include <assert.h>
#include <err.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

#ifdef DEBUG_MY_GETLINE
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

char *
my_getline(FILE *stream)
{
	char *buf, *lbuf;
	size_t len;

	assert(stream);

	DPRINTF(("Getting line from stream=%p\n", stream));

	if ((buf = fgetln(stream, &len)) != NULL) {
		if (buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
			/* Strip trailing \r */
			if (len - 2 > 0 && buf[len - 2] == '\r')
				buf[len - 2] = '\0';
			lbuf = strndup(buf, len);
			if (lbuf == NULL)
				err(EXIT_FAILURE, "strndup");
		} else {
			lbuf = strndup(buf, len + 1);
			if (lbuf == NULL)
				err(EXIT_FAILURE, "strndup");
			lbuf[len] = '\0';
			/* Strip trailing \r */
			if (len - 1 > 0 && lbuf[len - 1] == '\r')
				lbuf[len - 1] = '\0';
		}
		return lbuf;
	}

	if (ferror(stream))
		err(EXIT_FAILURE, "ferror");

	assert(feof(stream));

	return NULL;
}
