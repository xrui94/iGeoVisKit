#include "PchApp.h"

#include "ViewportListener.h"
#include "utils/Console.h"

void ViewportListener::notify_viewport()
{
	Console::write("Default notify viewport (probably not what you want)\n");
}

void ViewportListener::notify_bands()
{
	Console::write("Default notify bands (probably not what you want)\n");
}
