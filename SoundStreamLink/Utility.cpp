#include "Utility.h"

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

void CheckWSAStartupResult(int result, const char* failedOperation) {
    ReturnMustCodeBeZero(result, failedOperation);
}

void CheckSocket(int result, const char* failedOperation) {
    ReturnMustCodeBeZero(result, failedOperation);
}

void ReturnMustCodeBeZero(int result, const char* failedOperation) {
    if (result == 0) return;
    const size_t bufferSize = 1024;
    auto errorMessage = std::make_unique<char[]>(bufferSize);
    sprintf_s(errorMessage.get(), bufferSize, "'%s' failed with ReturnCode: %d (0x%08X)\n", failedOperation, result, result);
    throw std::runtime_error(errorMessage.get());
}
