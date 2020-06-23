#include "pr.h"

int pr_level = PR_LEVEL_INFO;

int pr_set_level(int level)
{
    if(level >  PR_LEVEL_DEBUG)
    {
        level = PR_LEVEL_DEBUG;
    }
    else if(level < PR_LEVEL_ERROR)
    {
        level = PR_LEVEL_ERROR;
    }

    int old_level = pr_level;
    pr_level = level;

    return old_level;
}

string tid_to_string(const thread::id& tid)
{
    ostringstream oss;
    oss << tid << endl;
    return oss.str();
}

long long tid_to_ll(const thread::id& tid)
{
    return atoll(tid_to_string(tid).c_str());
}