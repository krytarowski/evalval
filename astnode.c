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
#include <ctype.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util.h>

#include "astnode.h"

#ifdef DEBUG_ASTNODE
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

struct astnode {
	enum astnode_type	 type;
	double			 value;
	struct astnode		*left;
	struct astnode		*right;
};

struct astnode *
astnode_new_node(enum astnode_type type, struct astnode *left,
                struct astnode *right)
{
	struct astnode *this;

	this = ecalloc(1, sizeof(*this));

	this->type = type;
	this->left = left;
	this->right = right;

	return this;
}

struct astnode *
astnode_new_unarynode(struct astnode *left)
{
	struct astnode *this;

	assert(left);

	this = ecalloc(1, sizeof(*this));

	this->type = astnode_type_unaryminus;
	this->left = left;

	return this;
}

struct astnode *
astnode_new_numbernode(double value)
{
	struct astnode *this;

	this = ecalloc(1, sizeof(*this));

	this->type = astnode_type_number;
	this->value = value;

	return this;
}

void
astnode_delete(struct astnode *this)
{
	assert(this);

	free(this);
}

enum astnode_type
astnode_type(struct astnode *this)
{

	assert(this);

	return this->type;
}

double
astnode_value(struct astnode *this)
{

	assert(this);

	return this->value;
}

struct astnode *
astnode_left(struct astnode *this)
{

	assert(this);

	return this->left;
}

struct astnode *
astnode_right(struct astnode *this)
{

	assert(this);

	return this->right;
}
