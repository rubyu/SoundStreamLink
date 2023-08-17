#pragma once

#include <stdexcept>
#include <memory>
#include <string>
#include <Mmdeviceapi.h>

void CheckHresult(HRESULT hr, const char* failedOperation);
void CheckHandle(HANDLE handle, const char* failedOperation);
void CheckSocket(SOCKET socket, const char* failedOperation);
void MustEquals(int mustBe, int actual, const char* failedOperation);
void MustNotEquals(int mustNotBe, int actual, const char* failedOperation);

std::wstring to_wstring(const std::string& stringToConvert);
