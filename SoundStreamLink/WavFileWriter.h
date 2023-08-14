#pragma once

#include <cstdint>
#include <vector>
#include <fstream>
#include <string>
#include <Mmdeviceapi.h>

class WavFileWriter {
private:
    struct WAV_HEADER {
        uint8_t RIFF[4] = { 'R', 'I', 'F', 'F' };
        uint32_t ChunkSize;
        uint8_t WAVE[4] = { 'W', 'A', 'V', 'E' };
        /* "fmt" sub-chunk */
        uint8_t fmt[4] = { 'f', 'm', 't', ' ' };
        uint32_t Subchunk1Size;
        uint16_t AudioFormat;
        uint16_t NumOfChan;
        uint32_t SamplesPerSec;
        uint32_t bytesPerSec;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
        /* "data" sub-chunk */
        uint8_t Subchunk2ID[4] = { 'd', 'a', 't', 'a' };
        uint32_t Subchunk2Size;
    };
public:
    static bool WriteWaveFile(const std::string& filename, const std::vector<BYTE>& data, const WAVEFORMATEX& format);
};