#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "y.tab.h"
#include "mgl-code"

extern FILE *yyin, *yyout;

extern int screen_done;
extern char *cmd_str, *act_str, *item_str;

static char current_screen[100];
static int done_start_init;
static int done_end_init;
static int current_line;
struct item {
    char *desc;
    char *cmd;
    int action;
    char *act_str;
    int attribute;
    struct item *next;
} *item_list, *last_item;

#define SCREEN_SIZE 80
void cfree(void *);
void dump_data(char **);
int check_name(char *);
void warning(char *, char *);

int start_screen(char *name) {
    long tm = time((long *)0);

    if (!done_start_init) {
        fprintf(yyout,
            "/*\n * Generated by MGL: %s */\n\n",
            ctime(&tm));
        dump_data(screen_init);
        done_start_init = 1;
    }

    if (check_name(name) == 0) {
        warning("Reuse of name", name);
    }

    fprintf(yyout, "/* screen %s */\n", name);
    fprintf(yyout, "void menu_%s() {\n", name);
    fprintf(yyout, "\textern struct item menu_%s_items[];\n\n", name);
    fprintf(yyout, "\tif (!init) menu_init();\n\n");
    fprintf(yyout, "\tclear();\n");
    fprintf(yyout, "\trefresh();\n\n");

    if (strlen(name) > sizeof(current_screen)) {
        warning("Screen name is large than buffer", (char *)0);
    }
    strncpy(current_screen, name, sizeof(current_screen) - 1);

    screen_done = 0;
    current_line = 0;

    return 0;
}

void add_title(char *line) {
    int length = strlen(line);
    int space = (SCREEN_SIZE - length) / 2;

    fprintf(yyout, "\tmove(%d, %d); \n", current_line, space);
    current_line++;
    fprintf(yyout, "\taddstr(\"%s\");\n", line);
    fprintf(yyout, "\trefresh();\n\n");
}

void add_line(int action, int attrib) {
    struct item *n = (struct item*)malloc(sizeof(struct item));

    if (!item_list) {
        item_list = last_item = n;
    } else {
        last_item->next = n;
        last_item = n;
    }

    n->next = NULL;
    n->desc = item_str;
    n->cmd = cmd_str;
    n->action = action;

    switch (action) {
        case EXECUTE:
            n->act_str = act_str;
            break;
        case MENU:
            n->act_str = act_str;
            break;
        default:
            n->act_str = (char *)0;
            break;
    }
    n->attribute = attrib;
}

void process_items() {
    int cnt = 0;
    struct item *ptr;

    if (item_list == (struct item *)0) {
        return;
    }

    fprintf(yyout, "struct item menu_%s_items[] = {\n", current_screen);
    ptr = item_list;

    while (ptr) {
        struct item *optr;
        if (ptr->action == MENU) {
            fprintf(yyout,
                "\t{\"%s\", \"%s\", %d, \"\", %s, %d}, \n",
                ptr->desc, ptr->cmd, ptr->action,
                ptr->act_str, ptr->attribute);
        } else {
            fprintf(yyout,
                "\t{\"%s\", \"%s\", %d, \"%s\", 0, %d}, \n",
                ptr->desc, ptr->cmd, ptr->action,
                ptr->act_str ? ptr->act_str : "",
                ptr->attribute);
        }

        cfree(ptr->desc);
        cfree(ptr->cmd);
        cfree(ptr->act_str);
        optr = ptr;
        ptr = ptr->next;
        cfree(optr);
        cnt++;
    }

    fprintf(yyout, "{(char *)0, (char *)0, 0, (char *)0, 0, 0},\n");
    fprintf(yyout, "};\n\n");
    item_list = (struct item *)0;
}

int end_screen(char *name) {
    fprintf(yyout, "\tmenu_runtime(menu_%s_items);\n", name);

    if (strcmp(current_screen, name) != 0) {
        warning("name mismatch at end of screen", current_screen);
    }
    fprintf(yyout, "}\n");
    fprintf(yyout, "/* end %s */\n\n", current_screen);

    process_items();

    if (!done_end_init) {
        done_end_init = 1;
        dump_data(menu_init);
    }

    current_screen[0] = '\0';
    screen_done = 1;

    return 0;
}

void dump_data(char **array) {
    while (*array) {
        fprintf(yyout, "%s\n", *array++);
    }
}

void end_file() {
    dump_data(menu_runtime);
}

void add_main(char *main_screen) {
    fprintf(yyout, "\nint main(int argc, char *argv[]) {\n");
    fprintf(yyout, "\tmenu_%s();\n", main_screen);
    fprintf(yyout, "\tmenu_cleanup();\n");
    fprintf(yyout, "\treturn 0;\n}\n");
}

int check_name(char *name) {
    static char **names = (char **)0;
    static int name_count = 0;
    char **ptr, *newstr;

    if (!names) {
        names = (char **)malloc(sizeof(char *));
        *names = (char *)0;
    }

    ptr = names;
    while (*ptr) {
        if (strcmp(name, *ptr++) == 0) {
            return 0;
        }
    }

    name_count++;
    names = (char **)realloc(names, (name_count+1)*sizeof(char *));
    names[name_count] = (char *)0;
    names[name_count-1] = strdup(name);

    return 1;
}

void cfree(void *p) {
    if (p) free(p);
}

