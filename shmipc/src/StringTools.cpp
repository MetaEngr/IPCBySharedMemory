#include "StringTools.h"

#include <Windows.h>
#include <string>

namespace tool
{
std::string WcharToChar(const wchar_t* wp, size_t encode)
{
    std::string str;
    int len = WideCharToMultiByte((UINT)encode, 0, wp, static_cast<int>(wcslen(wp)), NULL, 0, NULL, NULL);
    char* m_char = new char[len + 1];
    WideCharToMultiByte((UINT)encode, 0, wp, static_cast<int>(wcslen(wp)), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    str = m_char;
    delete m_char;
    return str;
}

std::wstring CharToWchar(const char* c, size_t encode)
{
    std::wstring str;
    int len = MultiByteToWideChar((UINT)encode, 0, c, static_cast<int>(strlen(c)), NULL, 0);
    wchar_t* m_wchar = new wchar_t[len + 1];
    MultiByteToWideChar((UINT)encode, 0, c, static_cast<int>(strlen(c)), m_wchar, len);
    m_wchar[len] = '\0';
    str = m_wchar;
    delete m_wchar;
    return str;
}
}  // namespace tool
