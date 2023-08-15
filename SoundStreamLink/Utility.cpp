#include "Utility.h"
#include <codecvt>
#include <locale>

void CheckHresult(HRESULT hr, const char* failedOperation) {
    if (!FAILED(hr)) return;
    const size_t bufferSize = 1024;
    auto errorMessage = std::make_unique<char[]>(bufferSize);
    sprintf_s(errorMessage.get(), bufferSize, "'%s' failed with HRESULT: %d (0x%08X)\n", failedOperation, hr, hr);
    throw std::runtime_error(errorMessage.get());
}

void CheckHandle(HANDLE handle, const char* failedOperation) {
    if (handle != NULL) return;
    const size_t bufferSize = 1024;
    DWORD lastError = GetLastError();
    auto errorMessage = std::make_unique<char[]>(bufferSize);
    sprintf_s(errorMessage.get(), bufferSize, "'%s' failed with LastError: %d (0x%08X)\n", failedOperation, lastError, lastError);
    throw std::runtime_error(errorMessage.get());
}

void CheckSocket(SOCKET socket, const char* failedOperation) {
    if (INVALID_SOCKET != socket) return;
    const size_t bufferSize = 1024;
    auto errorMessage = std::make_unique<char[]>(bufferSize);
    sprintf_s(errorMessage.get(), bufferSize, "'%s' failed because INVALID_SOCKET '%d (0x%08X)' == given socket '%d (0x%08X)'\n", failedOperation, INVALID_SOCKET, INVALID_SOCKET, socket, socket);
    throw std::runtime_error(errorMessage.get());
}

void MustEquals(int expected, int actual, const char* failedOperation) {
    if (expected == actual) return;
    const size_t bufferSize = 1024;
    auto errorMessage = std::make_unique<char[]>(bufferSize);
    sprintf_s(errorMessage.get(), bufferSize, "'%s' failed because expected value '%d (0x%08X)' != actual value '%d (0x%08X)'\n", failedOperation, expected, expected, actual, actual);
    throw std::runtime_error(errorMessage.get());
}


std::wstring to_wstring(const std::string& stringToConvert) {
    std::wstring wideString = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
    return wideString;
}
