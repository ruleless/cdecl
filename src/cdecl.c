#include "cdecl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_SIZE 64
#define BUF_SZ     256
#define STACK_SIZE 256

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
	
	char *output;
	int outpunt_len;

	struct token stack[STACK_SIZE];
	int top;

	struct token current;

	int analize_status;
};

static void cdecl_init(struct cdecl *c) {
	c->ptr = c->input;
	*c->output = '\0';
	c->top = 0;	
}		

struct cdecl *cdecl_new(int output_len) {
	struct cdecl *c = (struct cdecl *)malloc(sizeof(struct cdecl));
	memset(c, 0, sizeof(c));
	
	c->output = (char *)malloc(output_len);
	if (NULL == c->output)
		return NULL;
	
	c->outpunt_len = outpunt_len;
	cdecl_init(c);
	return c;
}

int cdecl_input(struct cdecl *c, const char *input) {
	cdecl_init(c);
	c->input_len = strlen(input)+1;
	c->input = (char *)malloc(c->input_len);
	if (NULL == c->input)
		return -1;
	return 0;
}

void cdecl_destroy(struct cdecl *c) {
	if (c->input)
		free(c->input);
	if (c->output)
		free(c->output);
	free(c);
}

static struct {
	const char *s;
	int type_tag;
} string_tag_map[] = {
	{"const", QUALIFIER},
	{"volatile", QUALIFIER},
	{"char", TYPE},
	{"short", TYPE},
	{"int", TYPE},
	{"long", TYPE},
	{"signed", TYPE},
	{"unsigned", TYPE},
	{"float", TYPE},
	{"double", TYPE},
	{"struct", TYPE},
};
static int classify(const char *s) {
	int i;
	static int sz = sizeof(string_tag_map)/sizeof(string_tag_map[0]);
	for (i = 0; i < sz; ++i) {
		if (strcmp(string_tag_map[i].s, s) == 0) {
			return string_tag_map[i].type_tag;
		}
	}
	return QUALIFIER;
}

static void get_next_token(struct cdecl *c) {
	
}

static void analize_function(struct cdecl *c) {
}

static void analize_array(struct cdecl *c) {
	
}

static void analize_pointers(struct cdecl *c) {
}

static void analize_declaration(struct cdecl *c) {
	switch (c->current.type) {
	case '(':
		analize_function(c);
		break;
	case '[':
		analize_array(c);
		break;
	}
	if (c->analize_status < 0)
		return;

	analize_pointers(c);

	while (c->top > 0) {
		if ('(' == c->stack[c->top].type) {
			top--;
			get_next_token(c);
			analize_declaration(c);			
		} else {
			snprintf(c->output, c->outpunt_len, "%s %s", c->output, c->stack[c->top--].str);
		}
	}
}
