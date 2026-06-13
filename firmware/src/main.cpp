/**
 * Rools — firmware entry (placeholder)
 *
 * M1: wire DaisyHW, AppShell, register apps.
 */

#include "app_shell.h"

// #include "daisy_seed.h"

int main(void)
{
    rools::AppShell shell;
    shell.init();
    shell.run_forever();
    return 0;
}
