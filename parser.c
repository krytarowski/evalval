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
#include "token.h"

#include "parser.h"

#ifdef DEBUG_PARSER
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

/*
expression = 
    term
    | expression "+" term
    | expression "-" term .
term = 
    factor
    | term "*" factor
    | term "/" factor .
factor = 
    number
    | - expression
    | "(" expression ")" .

=> Left recursion elimination http://web.cs.wpi.edu/~kal/PLT/PLT4.1.2.html

expression =
    term expression1

expression1 = 
    "+" term expression1
    | "-" term expression1
    | {epislon}

term =
   factor term1

term1 = 
    "*" factor term1
    | "/" factor term1
    | {epsilon}

factor = 
    number
    | - expression
    | "(" expression ")"
*/

struct parser {
	struct token	 token;
	char 		*text;
	size_t 		 index;
	jmp_buf		 jmpbuf;
};

static void
parser_getnexttoken(struct parser *this);

static void
parser_skipwhitespace(struct parser *this);

static double
parser_getnumber(struct parser *this);

static struct astnode *
parser_expression(struct parser *this);

static struct astnode *
parser_expression1(struct parser *this);

static struct astnode *
parser_term(struct parser *this);

static struct astnode *
parser_term1(struct parser *this);

static struct astnode *
parser_factor(struct parser *this);

static struct astnode *
parser_new_node(enum astnode_type type, struct astnode *left,
                struct astnode *right);

static struct astnode *
parser_new_unarynode(struct astnode *left);

static struct astnode *
parser_new_numbernode(double val);

struct parser *
parser_new(void)
{
	struct parser *this;

	this = ecalloc(1, sizeof(*this));

	return this;
}

void
parser_delete(struct parser *this)
{

	assert(this);

	free(this->text);
	free(this);
}

struct astnode *
parser_parse(struct parser *this, char *text)
{

	assert(this);
	assert(text);

	this->text = estrdup(text);
	this->index = 0;


	if (setjmp(this->jmpbuf) == 0) {
		parser_getnexttoken(this);
		return parser_expression(this);
	} else {
		return NULL;
	}
}

/* Private functions */

void
parser_getnexttoken(struct parser *this)
{

	assert(this);

	parser_skipwhitespace(this);

	switch (this->text[this->index]) {
	case '\0':
		this->token.type = token_type_eot;
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '.':
		this->token.type = token_type_number;
		this->token.value = parser_getnumber(this);
		break;
	case '+':
		this->token.type = token_type_plus;
		++this->index;
		break;
	case '-':
		this->token.type = token_type_minus;
		++this->index;
		break;
	case '*':
		this->token.type = token_type_mul;
		++this->index;
		break;
	case '/':
		this->token.type = token_type_div;
		++this->index;
		break;
	case '(':
		this->token.type = token_type_openparen;
		++this->index;
		break;
	case ')':
		this->token.type = token_type_closeparen;
		++this->index;
		break;
	default:
		fprintf(stderr, "Unrecognized input symbol: '%c'\n",
		        this->text[this->index]);
		longjmp(this->jmpbuf, 1);
	}
}

void
parser_skipwhitespace(struct parser *this) 
{               

	assert(this);

	while(isspace((unsigned char)this->text[this->index]))
		this->index++;
}

double
parser_getnumber(struct parser *this) 
{
	char buffer[32] = {};
	size_t index;
	size_t delta;
	double rv;

	assert(this);

	parser_skipwhitespace(this);

	index = this->index;

	while (isdigit((unsigned char)this->text[this->index]))
		this->index++;

	if (this->text[this->index] == '.')
		this->index++;

	while (isdigit((unsigned char)this->text[this->index]))
		this->index++;

	delta = this->index - index;        
	if (delta == 0 || delta > __arraycount(buffer)) {
		fprintf(stderr, "Unrecognized input symbol: '%c'\n",
		        this->text[this->index]);
		longjmp(this->jmpbuf, 1);
	}

	memcpy(buffer, &this->text[index], delta);

	/* strtod(3) */
	rv = atof(buffer);

	return rv;
}

void
parser_match(struct parser *this, enum token_type token)
{

	assert(this);

	if (this->token.type != token) {
		fprintf(stderr, "Unrecognized input symbol: '%c'\n",
		        this->text[this->index]);
		longjmp(this->jmpbuf, 1);
	}
}

struct astnode *
parser_expression(struct parser *this)
{
	struct astnode *tn;
	struct astnode *e1n;

	assert(this);

	tn = parser_term(this);
	e1n = parser_expression1(this);

	return parser_new_node(astnode_type_plus, tn, e1n);
}

struct astnode *
parser_expression1(struct parser *this)
{
	struct astnode *tn;
	struct astnode *e1n;

	assert(this);

	if (this->token.type == token_type_plus) {
		parser_getnexttoken(this);
		tn = parser_term(this);
		e1n = parser_expression1(this);
		return parser_new_node(astnode_type_plus, e1n, tn);
	} else if (this->token.type == token_type_minus) {
		parser_getnexttoken(this);
		tn = parser_term(this);
		e1n = parser_expression1(this);
		return parser_new_node(astnode_type_minus, e1n, tn);
	}

	return parser_new_numbernode(0);
}

struct astnode *
parser_term(struct parser *this)
{
	struct astnode *fn;
	struct astnode *t1n;

	assert(this);

	fn = parser_factor(this);
	t1n = parser_term1(this);

	return parser_new_node(astnode_type_mul, fn, t1n);
}

struct astnode *
parser_term1(struct parser *this)
{
	struct astnode *fn;
	struct astnode *t1n;

	assert(this);

	if (this->token.type == token_type_mul) {
		parser_getnexttoken(this);
		fn = parser_factor(this);
		t1n = parser_term1(this);

		return parser_new_node(astnode_type_mul, t1n, fn);
	} else if (this->token.type == token_type_div) {
		parser_getnexttoken(this);
		fn = parser_factor(this);
		t1n = parser_term1(this);

		return parser_new_node(astnode_type_div, t1n, fn);
	}

	return parser_new_numbernode(1);
}

struct astnode *
parser_factor(struct parser *this)
{
	struct astnode *n;
	double v;

	assert(this);

	if (this->token.type == token_type_openparen) {
		parser_getnexttoken(this);
		n = parser_expression(this);
		parser_match(this, token_type_closeparen);
		parser_getnexttoken(this);

		return n;
	} else if (this->token.type == token_type_minus) {
		parser_getnexttoken(this);
		n = parser_factor(this);

		return parser_new_unarynode(n);
	} else if (this->token.type == token_type_number) {
		v = this->token.value;
		parser_getnexttoken(this);

		return parser_new_numbernode(v);
	}
}

struct astnode *
parser_new_node(enum astnode_type type, struct astnode *left,
                struct astnode *right)
{
	struct astnode *n;

	n = astnode_new_node(type, left, right);

	DPRINTF(("%s(): node=%p type=%d left=%p right=%p\n", __func__, n, type, left, right));

	return n;
}

struct astnode *
parser_new_unarynode(struct astnode *left)
{
	struct astnode *n;

	n = astnode_new_unarynode(left);

	DPRINTF(("%s(): node=%p left=%p\n", __func__, n, left));

	return n;
}

struct astnode *
parser_new_numbernode(double val)
{
	struct astnode *n;

	n = astnode_new_numbernode(val);

	DPRINTF(("%s(): node=%p val=%lf\n", __func__, n, val));

	return n;
}
