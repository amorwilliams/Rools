/**
 * Rools 固件入口。
 *
 * 框架层仅 AppShell：init 外设 + 加载默认 App，run_forever 阻塞主循环。
 * 具体功能见 apps/ 下各 App 实现。
 */

#include "app_shell.h"

int main(void)
{
    rools::AppShell shell;
    shell.init();
    shell.run_forever();
    return 0;
}
