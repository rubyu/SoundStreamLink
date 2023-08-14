#include "WavFileWriter.h"
#include "mmreg.h"

bool WavFileWriter::WriteWaveFile(const std::string& filename, const std::vector<BYTE>& data, const WAVEFORMATEX& format) {
    int subchunk2Size = (int)data.size();
    int chunkSize = 36 + (int)data.size();

    struct WAV_HEADER header;
    if (format.wFormatTag == WAVE_FORMAT_PCM) {
        header.AudioFormat = 1;
    }
    else if (format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        header.AudioFormat = 3;
    }
    else if (format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        header.AudioFormat = 3;
    }
    else {
        return false;
    }

    header.NumOfChan = format.nChannels;
    header.SamplesPerSec = format.nSamplesPerSec;
    header.bytesPerSec = format.nAvgBytesPerSec;
    header.blockAlign = format.nBlockAlign;
    header.bitsPerSample = format.wBitsPerSample;
    header.ChunkSize = chunkSize;
    header.Subchunk2Size = subchunk2Size;

    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
    return true;
}
