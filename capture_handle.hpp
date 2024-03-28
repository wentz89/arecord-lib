/*
MIT License

Copyright (c) 2024 Alexander Wentz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _CAPTUREHANDLE_H_
#define _CAPTUREHANDLE_H_

#include "common.hpp"
#include "config.hpp"

#include <string>
#include <fstream>
#include <stdlib.h>
#include <cstdio>

struct WAV_HEADER {
  uint8_t riff[4] = {'R', 'I', 'F', 'F'};   // Magic header                             0-3
  uint32_t chunk_data_size = 44;            // size of the data + header                4-7
  uint8_t wave[4] = {'W', 'A', 'V', 'E'};   // wave header                              8-11
  uint8_t fmt[4] = {'f', 'm', 't', ' '};    // fmt header                               12-15
  uint32_t fmt_chunk_data = 16;             // fmt chunk                                16-19
  uint16_t audio_format = 1;                // 1=PCM,6=mulaw,7=alaw, 257=ibm mulaw,     20-21
                                            // 258=ibm alaw, 259=adpcm
  uint16_t channels = 1;                    //                                          22-23
  uint32_t rate = 48000;                    // sampling rate                            24-27
  uint32_t bytes_per_sec = 48000 * 2;       //                                          28-31
  uint16_t block_alignment = 2;             // 2=16-bit mono, 4=16-bit stereo           32-33
  uint16_t bits_per_sample = 16;            //                                          34-35
  uint8_t data_section[4] = {'d', 'a', 't', 'a'};//                                     36-39
  uint32_t size_of_data = 0;                // length of sampled data                   40-43
};

void setU32LEinStream(std::fstream& ss, uint32_t val){
    ss.put((uint8_t)((val & 0x000000FF)));
    ss.put((uint8_t)((val & 0x0000FF00) >> 8));
    ss.put((uint8_t)((val & 0x00FF0000) >> 16));
    ss.put((uint8_t)((val & 0xFF000000) >> 24));
    return;
}

class CaptureHandle{
public:
    CaptureHandle(CaptureConfig config) {
        TR_MSG("CaptureHandle");
        if(config.mode & CAPTURE_MODE::RAW){
            TR_MSG("Capture RAW.");
            m_raw = true;
        }
        if(config.mode & CAPTURE_MODE::STDOUT){
            TR_MSG("Write to STDOUT.");
            m_stdout = true;
        }
        if(config.mode & CAPTURE_MODE::WAV){
            TR_MSG("Capture WAV");
            m_wav = true;
        }
        m_rawFileName = config.raw_file_name;
        m_wavFileName = config.wav_file_name;
        m_overwrite = config.overwriteExistingFiles;
    }

    ~CaptureHandle(){}

    bool init(const HwConfig& streamInfo, int bytesPerSample) {
        TR();
        MSG_AND_RETURN_IF(m_init, true, "Already initialized");
        if(m_raw){
            MSG_AND_RETURN_IF(!prepareFile(m_rawFileName), false, "Could not prepare %s", m_rawFileName.c_str());
        }
        if(m_wav){
            MSG_AND_RETURN_IF(!prepareFile(m_wavFileName), false, "Could not prepare %s", m_wavFileName.c_str());
            MSG_AND_RETURN_IF(!prepareWavHeader(m_wavFileName, streamInfo, bytesPerSample), false, "Could not write wav-header.");
        }
        m_init = true;
        return true;
    };

    bool write(u_char *buff, size_t size){
        if(!m_init){
            return false;
        }
        if(m_wav){
            MSG_AND_RETURN_IF(!internalWrite(m_wavFileName, buff, size), false, "Failed to write %zu bytes to %s", size, m_wavFileName.c_str());
            MSG_AND_RETURN_IF(!updateWavHeader(m_wavFileName), false, "Failed updating Wav Header");
        }
        if(m_raw){
            MSG_AND_RETURN_IF(!internalWrite(m_rawFileName, buff, size), false, "Failed to write %zu bytes to %s", size, m_rawFileName.c_str());
        }
        if(m_stdout){
            int res = ::write(1, buff, size);
            MSG_AND_RETURN_IF(res < 0, false, "Write to stdout failed");
        }

        return true;
    }

private:
    bool m_wav = false;
    bool m_stdout = false;
    bool m_raw = false;
    bool m_init = false;
    bool m_overwrite = false;
    bool m_newCreated = false;
    std::string m_wavFileName = "";
    std::string m_rawFileName = "";

    bool fileExists (const std::string& name) {
        std::ifstream f(name.c_str());
        return f.good();
    };

    bool prepareFile(const std::string& fileName){
        if(fileExists(fileName) && m_overwrite){
            MSG_AND_RETURN_IF(std::remove(fileName.c_str()) != 0, false, "Can not remove existing file");
            m_newCreated = true;
        }
        int fd = open(fileName.c_str(), O_WRONLY | O_CREAT, 0644);
        MSG_AND_RETURN_IF(fd < 0, false, "Failed to prepare file.");
        (void)::close(fd);
        return true;
    }

    bool prepareWavHeader(const std::string& file, const HwConfig& streamInfo, int bytesPerSample){
        if(!m_newCreated){
            return true;
        }
        WAV_HEADER header;
        header.channels = streamInfo.channels;
        header.rate = streamInfo.rate;
        header.bytes_per_sec = streamInfo.rate * bytesPerSample;
        // TODO find out: not sure if I should devide by channels? (but 2 works for now) 
        header.bits_per_sample = bytesPerSample * BITS_PER_BYTE / 2;
        constexpr uint8_t STEREO = 4;
        constexpr uint8_t MONO = 2;
        header.block_alignment = streamInfo.channels > 1 ? STEREO : MONO;

        u_char* buff = (u_char*)malloc(sizeof(header));
        if(buff == nullptr){
            return false;
        }

        memcpy(buff, &header, sizeof(header));
        bool res = internalWrite(file, buff, sizeof(header));
        MSG_AND_RETURN_IF(!res, false, "failed creating wav header");
        free(buff);
        return res;
    }

    bool internalWrite(const std::string& file, u_char* buff, size_t size){
        std::ofstream ss;
        ss.open(file, std::ios_base::app|std::ios::binary);
        if(!ss.good()){
            return false;
        }
        ss.write((char *)buff, size);
        if(!ss.good()){
            return false;
        }
        ss.close();
        return true;
    }

    bool updateWavHeader(std::string file){
        std::fstream ss;
        ss.open(file, std::ios::in|std::ios::out|std::ios::binary);
        if(!ss.good()){
            return false;
        }
        ss.seekg(0, ss.end);
        int fileLength = ss.tellg();
        int chunkSize = fileLength - 8;
        int dataSize = fileLength - sizeof(WAV_HEADER);
        constexpr size_t POS_CHUNK_DATA_SIZE = 4;
        constexpr size_t POS_SAMPLE_DATA_SIZE = 40;
        ss.seekp(POS_CHUNK_DATA_SIZE, std::ios_base::beg);
        setU32LEinStream(ss, (uint32_t)chunkSize);
        ss.seekp(POS_SAMPLE_DATA_SIZE, std::ios_base::beg);
        setU32LEinStream(ss, (uint32_t)dataSize);
        ss.close();
        return true;
    }

};

#endif