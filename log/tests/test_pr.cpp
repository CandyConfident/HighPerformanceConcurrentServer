#include <stdio.h>
#include "../pr.h"

void pr_test(const char* title)
{
    printf("------------level %s-----------\r\n", title);
    PR_DEBUG("pr: %s\r\n", "debug");
    PR_INFO("pr: %s\r\n", "info");
    PR_WARN("pr: %s\r\n", "warn");
    PR_ERROR("pr: %s\r\n", "error");
    printf("\r\n");
}

int main()
{
    ostringstream oss;
    string tid = tid_to_string(this_thread::get_id());
    PR_INFO("tid is:%s\n", tid.c_str());

    pr_set_level(PR_LEVEL_DEBUG);
    pr_test("debug");

    pr_set_level(PR_LEVEL_INFO);
    pr_test("info");

    pr_set_level(PR_LEVEL_WARN);
    pr_test("warn");

    pr_set_level(PR_LEVEL_ERROR);
    pr_test("error");

    return 0;
}