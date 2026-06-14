/**
 * Rools — firmware entry
 */

#include "app_shell.h"

int main(void)
{
    rools::AppShell shell;
    shell.init();
    shell.run_forever();
    return 0;
}
