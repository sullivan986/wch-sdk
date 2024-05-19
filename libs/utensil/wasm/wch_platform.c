#include <stdbool.h>
#include "platform_internal.h"
#include "platform_api_vmcore.h"
#include "stdio.h"

int os_thread_sys_init();
void os_thread_sys_destroy();

static int _stdout_hook_iwasm(int c)
{
    printf("%c", (char)c);
    return 1;
}

int bh_platform_init(void)
{
    extern void __stdout_hook_install(int (*hook)(int));
    /* Enable printf() in Zephyr */
    __stdout_hook_install(_stdout_hook_iwasm);

    return os_thread_sys_init();
}

void bh_platform_destroy()
{
    os_thread_sys_destroy();
}

int os_printf(const char *format, ...)
{
    int ret = 0;
    va_list ap;

    va_start(ap, format);
    ret += vprintf(format, ap);
    va_end(ap);

    return ret;
}

int os_vprintf(const char *format, va_list ap)
{
    return vprintf(format, ap);
}
