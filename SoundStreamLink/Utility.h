#pragma once

#include <stdexcept>
#include <memory>
#include <string>
#include <Mmdeviceapi.h>

void CheckHresult(HRESULT hr, const char* failedOperation);
void CheckHandle(HANDLE handle, const char* failedOperation);
void CheckWSAStartupResult(int result, const char* failedOperation);
void CheckSocket(int result, const char* failedOperation);
