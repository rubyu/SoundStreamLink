#pragma once

#include <cstdint>
#include <vector>
#include <fstream>
#include <string>
#include <Mmdeviceapi.h>

struct WAV_HEADER {
    uint8_t RIFF[4] = { 'R', 'I', 'F', 'F' };  // RIFF Header
    uint32_t ChunkSize;                        // RIFF Chunk Size
    uint8_t WAVE[4] = { 'W', 'A', 'V', 'E' };  // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t fmt[4] = { 'f', 'm', 't', ' ' };   // Fmt header
    uint32_t Subchunk1Size;                    // Size of the fmt chunk
    uint16_t AudioFormat;                      // Audio format 1=PCM
    uint16_t NumOfChan;                        // Number of channels 1=Mono 2=Sterio
    uint32_t SamplesPerSec;                    // Sampling Frequency in Hz
    uint32_t bytesPerSec;                      // bytes per second
    uint16_t blockAlign;                       // 2=16-bit mono, 4=16-bit stereo
    uint16_t bitsPerSample;                    // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t Subchunk2ID[4] = { 'd', 'a', 't', 'a' }; // "data"  string
    uint32_t Subchunk2Size;                    // Sampled data length
};

class WavFileWriter {
public:
    static bool WriteWaveFile(const std::string& filename, const std::vector<BYTE>& data, const WAVEFORMATEX& format);
};
