#include "PchApp.h"

#include "Console.h"

#if USE_CONSOLE


using namespace std;

char* console_buffer;
const int console_buffer_length = 512;

// 将 UTF-8 字节串转换为 UTF-16 并以宽字符输出到控制台
static void writeUtf8ToConsole(const char* utf8)
{
	if (!utf8) return;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (wlen <= 1) return;
	std::wstring wstr; wstr.resize((size_t)wlen - 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], wlen);
	DWORD written = 0;
	WriteConsoleW(hOut, wstr.c_str(), (DWORD)wstr.size(), &written, NULL);
}

// display console window
void Console::open()
{
    AllocConsole();
    COORD mySize;
    mySize.X = 80;
    mySize.Y = 10000;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), mySize);
    // 设置控制台代码页为 UTF-8，确保宽/窄字符转换一致
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Redirect C stdio to the newly allocated console so printf/puts are visible
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$",  "r", stdin);
    console_buffer = new char[512];
}

// hide console window    
void Console::close()
{
    FreeConsole();
    delete[] console_buffer;
}

// write string to console window
void Console::write(const char *msg_format, ...)
{
	va_list	ap;
	
	assert(msg_format != NULL);
	
	va_start(ap, msg_format);
    vsnprintf(console_buffer, console_buffer_length, msg_format, ap);
	va_end(ap);

	writeUtf8ToConsole(console_buffer);
}

// write string to console window
void Console::write(string *msg)
{
    if (!msg) return;
    writeUtf8ToConsole(msg->c_str());
}

// write a RECT structure to console window
void Console::writeRECT(RECT *rect)
{
    Console::write("top: %d, left: %d, right: %d, bottom: %d\n", rect->top, rect->left, rect->right, rect->bottom);
}

// write integer to console window
void Console::write(int msg)
{
    // 转成字符串并按 UTF-8 输出
    std::string s = std::to_string(msg);
    writeUtf8ToConsole(s.c_str());
}

// wait for user to press a enter in the console window
void Console::waitForEnter()
{
	DWORD bytesRead;
	ReadConsole(GetStdHandle(STD_INPUT_HANDLE),console_buffer,1,&bytesRead,0);
}

#endif // USE_CONSOLE
