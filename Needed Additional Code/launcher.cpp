#include <windows.h>

int main() {
    LoadLibraryA("testLibrary.dll");
    Sleep(5000);
    return 0;
}
