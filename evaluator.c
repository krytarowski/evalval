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

#include "astnode.h"

#include "evaluator.h"

#ifdef DEBUG_EVALUATOR
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

struct evaluator
{
};

double
evaluator_evalsubtree(struct evaluator *this, struct astnode *ast);

static struct evaluator this;

struct evaluator *
evaluator_singleton(void)
{
	return &this;
}

double
evaluator_eval(struct evaluator *this, struct astnode *n)
{

	assert(this);
	assert(n);

	evaluator_evalsubtree(this, n);
}

/* Private functions */

double
evaluator_evalsubtree(struct evaluator *this, struct astnode *n)
{
	double v1, v2;

	assert(this);
	assert(n);

	if (astnode_type(n) == astnode_type_number) {
                return astnode_value(n);
        } else if (astnode_type(n) == astnode_type_unaryminus) {
                return -evaluator_evalsubtree(this, astnode_left(n));
        } else {
                v1 = evaluator_evalsubtree(this, astnode_left(n));
                v2 = evaluator_evalsubtree(this, astnode_right(n));
                switch (astnode_type(n)) {
                        case astnode_type_plus:
                                return v1 + v2;
                        case astnode_type_minus:
                                return v1 - v2;
                        case astnode_type_mul:
                                return v1 * v2;
                        case astnode_type_div:
                                return v1 / v2;
                }
	}
}
