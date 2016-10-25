#include "cdecl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TOKEN_SIZE 64
#define STACK_SIZE 256
#define MAX_BUF    256

#define END_TAG    0
#define QUALIFIER  1
#define TYPE       2
#define IDENTIFIER 3

struct token {
	char str[TOKEN_SIZE];
	int type;
};

struct cdecl {
    char *input, *ptr;
	int input_len;

	/* valuables witch help to analysis */
	struct token stack[STACK_SIZE];
	int top;
	struct token current;
	int error;
	char err_buf[MAX_BUF];

	/* analyzed results */
	char identifier[MAX_BUF];
};

void strncat_safe(char *dest, const char *src, int destlen) {
	char *ptr = dest, *end_ptr = dest+destlen-1;	
	while (*ptr && ptr < end_ptr)
		ptr++;

	if ('\0' == *ptr && ptr < end_ptr) {
		while (ptr < end_ptr && *src) {
			*ptr++ = *src++;
		}
		*ptr = '\0';
	}
}

struct cdecl *cdecl_new() {
	struct cdecl *c = (struct cdecl *)malloc(sizeof(struct cdecl));
	memset(c, 0, sizeof(*c));
	
	return c;
}

void cdecl_destroy(struct cdecl *c) {
	if (c->input)
		free(c->input);
	free(c);
}

int cdecl_input(struct cdecl *c, const char *input) {
	if (c->input) {
		free(c->input);
	}
	c->input_len = strlen(input)+1;
	c->input = (char *)malloc(c->input_len);	
	if (NULL == c->input)
		return -1;
	
	memcpy(c->input, input, c->input_len);
	c->ptr = c->input;
	return 0;
}

int cdecl_error(cdecl *c) {
	return c->error;
}

static void handle_error(cdecl *c, const char *msg) {
	c->error = -1;
	snprintf(c->err_buf, sizeof(c->err_buf), "%s", msg);
}

static struct {
	const char *s;
	int type_tag;
} string_tag_map[] = {
	{"const", QUALIFIER},
	{"volatile", QUALIFIER},

	{"void", TYPE},
	{"char", TYPE},
	{"short", TYPE},
	{"int", TYPE},
	{"long", TYPE},
	{"signed", TYPE},
	{"unsigned", TYPE},
	{"float", TYPE},
	{"double", TYPE},

	/*
	{"struct", TYPE},
	{"union", TYPE},
	{"enum", TYPE},
	*/
};
static int classify(const char *s) {
	int i;
	static int sz = sizeof(string_tag_map)/sizeof(string_tag_map[0]);
	for (i = 0; i < sz; ++i) {
		if (strcmp(string_tag_map[i].s, s) == 0) {
			return string_tag_map[i].type_tag;
		}
	}
	return IDENTIFIER;
}

static int get_next_token(struct cdecl *c) {
	c->current.type = END_TAG;

	if (!(c->ptr && *c->ptr))
		return c->current.type;
	while (*c->ptr == ' ')
		c->ptr++;
	if ('\0' == *c->ptr)
		return c->current.type;

	char *write_ptr = c->current.str, *end_ptr = c->current.str+sizeof(c->current.str)-1;
	if (isalpha(*c->ptr)) {
		*write_ptr++ = *c->ptr++;
		while (isalnum(*c->ptr) && write_ptr < end_ptr) {
			*write_ptr++ = *c->ptr++;
		}
		*write_ptr = '\0';
		
		c->current.type = classify(c->current.str);
		return c->current.type;
	}

	c->current.type = *c->ptr;
	*write_ptr++ = *c->ptr++;
	*write_ptr = '\0';
	return c->current.type;
}

static void analyze_function(struct cdecl *c, char *output, int output_len) {
	while (c->current.type != ')' && c->current.type != END_TAG) {
		get_next_token(c);
	}
	if (c->current.type != ')') {
		handle_error(c, "no matching ')'");
		return;
	}

	strncat_safe(output, "function return ", output_len);
	get_next_token(c);
}

static void analyze_array(struct cdecl *c, char *output, int output_len) {
	while ('[' == c->current.type) {
		strncat_safe(output, "array with ", output_len);
		
		get_next_token(c);
		while (c->current.type != ']' && c->current.type != END_TAG) {
			get_next_token(c);
		}
		if (c->current.type != ']') {
			handle_error(c, "no matching ']'");
			break;
		}
		get_next_token(c);
	}
}

static void analyze_declaration(struct cdecl *c, char *output, int output_len) {
	switch (c->current.type) {
	case '(':
		analyze_function(c, output, output_len);
		break;
	case '[':
		analyze_array(c, output, output_len);
		break;
	}
	if (cdecl_error(c) < 0)
		return;

	char buf[MAX_BUF];
	while (c->top > 0 && cdecl_error(c) == 0) {
		int t = c->stack[c->top].type;
		if ('(' == t) { /* analyze () */
			if (')' == c->current.type) {
				c->top--;
				get_next_token(c);
				analyze_declaration(c, output, output_len);				
			} else {
				handle_error(c, "no matching ')'");
			}
		} else if ('*' == t) { /* pointer */
			c->top--;
			strncat_safe(output, "pointer to ", output_len);
		} else if (QUALIFIER == t || /* like 'const', 'volatile' */
				   TYPE == t) { /* like 'void', 'char', 'int', 'float' etc. */
			int fake_top = c->top, i;			
			while (fake_top > 0 &&
				   ((QUALIFIER == t && QUALIFIER == c->stack[fake_top].type) ||
					(TYPE == t && (QUALIFIER == c->stack[fake_top].type || TYPE == c->stack[fake_top].type))))
				fake_top--;

			buf[0] = '\0';
			for (i = fake_top+1; i <= c->top; ++i) {
				strncat_safe(buf, c->stack[i].str, sizeof(buf));
				strncat_safe(buf, " ", sizeof(buf));
			}
			strncat_safe(output, buf, output_len);
			c->top = fake_top;
		} else {
			handle_error(c, "unknown error");
		}
	}
}

static void read_to_identifier(struct cdecl *c) {
	get_next_token(c);
	while (c->top < STACK_SIZE-1 &&
		   c->current.type != IDENTIFIER && c->current.type != END_TAG) {
		c->stack[++c->top] = c->current;
		get_next_token(c);
	}
}

int cdecl_analyze(struct cdecl *c, char *output, int output_len) {
	if (!(c->input && c->input_len > 0))
		return -1;
	if (!(output && output_len > 0))
		return -1;

	c->top = 0;
	c->current.type = END_TAG;
	c->error = 0;

	read_to_identifier(c);
	if (IDENTIFIER == c->current.type) {
		snprintf(c->identifier, sizeof(c->identifier), "%s", c->current.str);
		get_next_token(c);
		analyze_declaration(c, output, output_len);
	} else {
		handle_error(c, "no identification!");
	}
	if (cdecl_error(c)) {
		snprintf(output, output_len, "invalid declaration: %s", c->err_buf);
		return 1;
	}
	return 0;
}

char *cdecl_identifier(struct cdecl *c) {
	return c->identifier;
}
