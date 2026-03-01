
#include <stdlib.h>
#include <time.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define APP_NAME "ryi"

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    Nob_String_Builder sb = {0};
    nob_sb_appendf(&sb, "#ifndef __BUILD_DATE__\n");
    nob_sb_appendf(&sb, "#define __BUILD_DATE__ \"%d-%02d-%02d %02d:%02d:%02d\"\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char* os = NULL;
#if defined(__linux__) || defined(__unix__)
    char* user = getenv("USER");
    nob_sb_appendf(&sb, "#define __BUILD_BY__ \"%s\"\n", user);
    os = "Linux/Unix";
#elif defined(_WIN32) || defined(_WIN64)
    os = "Windows";
#endif

    nob_sb_appendf(&sb, "#define __BUILD_ON__ \"%s\"\n", os);


    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "g++");
    nob_cmd_append(&cmd, "-Wall");
    nob_cmd_append(&cmd, "-ggdb");
    nob_cmd_append(&cmd, "main.cpp");
    nob_cmd_append(&cmd, "tinyfiledialogs.c");
    nob_cmd_append(&cmd, "-o");
    nob_cmd_append(&cmd, APP_NAME);
    nob_cmd_append(&cmd, "-lraylib");


    Nob_String_Builder line_sb = {0};
    for (int i = 0; i < cmd.count; ++i) {
        nob_sb_appendf(&line_sb, "%s", cmd.items[i]);
        if (i < cmd.count - 1)
            nob_sb_appendf(&line_sb, " ");
    }
    nob_sb_appendf(&sb, "#define __BUILD_COMMAND__ \"%s\"\n", line_sb.items);
    nob_sb_appendf(&sb, "#endif //__BUILD_DATE__\n");
    char* build = "build.h";
    nob_write_entire_file(build, sb.items, sb.count);
    sb.count = 0;

    if (!nob_cmd_run(&cmd)) {
        return 1;
    }

    return 0;
}
