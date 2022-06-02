#pragma once

#include <string>

namespace tool
{
std::string WcharToChar(const wchar_t* wp, size_t encode);

std::wstring CharToWchar(const char* c, size_t encode);

}  // namespace tool
