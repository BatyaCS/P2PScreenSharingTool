#include <common.h>
#include <app/application.h>

int main()
{
    Application app;

    if (!app.init())
        return -1;

    app.run();

    return 0;
}