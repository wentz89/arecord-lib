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
#ifndef _RECORDER_H_
#define _RECORDER_H_

#include <climits>
#include <fstream>
#include <stdlib.h>

#include "audio_buffer.hpp"
#include "config.hpp"
#include "snd_pcm_params.hpp"
#include "handle.hpp"
#include "common.hpp"

class CaptureFile{
public:
    CaptureFile() {TR_MSG("CaptureFile");};
    ~CaptureFile(){
        if(m_fd != -1){
            close();
        }
    }

    bool init(ConfigParams &config) {
        TR();
        MSG_AND_RETURN_IF(m_fd != -1, false, "Already initialized");
        if(fileExists(config.capture_file_name)) {
            MSG_AND_RETURN_IF(std::remove(config.capture_file_name.c_str()) != 0, false, "Can not remove existing file");
        }
        MSG_AND_RETURN_IF(internalOpen(config.capture_file_name) == false, false, "Already initialized");
        m_fileName = config.capture_file_name;
        TR_MSG("Init CaptureFile Done");
        // TODO create file header depending of file-type, for now just record raw data
        return true;
    };

    bool write(u_char *buff, size_t size){
        if(m_fd == -1){
            return false;
        }
        size_t off = 0;
        while( off < size ){
            int res = ::write(m_fd, buff + off, size - off);
            if(res < 0) {
                return false;
            }
            off += res;
        }
        // todo store global offset?
        return true;
    }

    void close(){
        ::close(m_fd);
        m_fd = -1;
    };

    int get() {
        return m_fd;
    };

    std::string getFilename() {
        return m_fileName;
    }

private:
    int m_fd = -1;
    std::string m_fileName = "";

    bool fileExists (const std::string& name) {
        std::ifstream f(name.c_str());
        return f.good();
    };

    bool internalOpen(std::string& fname) {
        m_fd = open(fname.c_str(), O_WRONLY | O_CREAT, 0644);
        if(m_fd == -1) {
            fprintf(stderr, "Open %s failed: %s", fname.c_str(), strerror(errno));
            if(errno == ENOENT) {
                fprintf(stderr, "Can not create new directories.");
            }
            return false;
        }
        return true;
    };
};

class Recorder{
public:
    Recorder(ConfigParams &config) : m_audioBuffer(), m_handle(), m_hwparams(), m_swparams(), m_config(config), m_file()
    {
        TR_MSG("Recorder");
    };
    ~Recorder() = default;

    bool init(){
        TR();
        MSG_AND_RETURN_IF(m_config.capture_file_name.empty(), false, "Name can not be empty. STDOUT currently not supported");
        MSG_AND_RETURN_IF(m_handle.init(m_config) == false, false, "Handle could not be initialized");
        TR_MSG("Pointer to Handle %p", m_handle.get());
        MSG_AND_RETURN_IF(m_hwparams.init(m_handle.get(), m_config) == false, false, "HwParams could not be initialized");
        MSG_AND_RETURN_IF(m_swparams.init(m_handle.get(), m_config) == false, false, "SwParams could not be initialized");
        MSG_AND_RETURN_IF(m_audioBuffer.init(m_config) == false, false, "AudioBuffer could not be initialized");
        MSG_AND_RETURN_IF(m_config.duration == m_config.samples && m_config.duration == -1, false, "Duration and sample can not be both -1");
        m_bytesToRead = getBytesToRead();
        MSG_AND_RETURN_IF(m_bytesToRead == 0, false, "Zero bytes to be read");
        MSG_AND_RETURN_IF(m_file.init(m_config) == false, false, "Can not init file.");
        TR_MSG("Initialization of Recorder done");
        // max_file_size = 0
        m_init = true;
        return true;
    };

    bool doRecord(){
        TR();
        // todo do recording in thread async and inform user via callback
        if(!m_init){
            TR_MSG("Not initialized. Abort");
            return false;
        }
        TR();
        off_t bytesRead = 0;
        while(bytesRead < m_bytesToRead) {
            size_t read = 0;
            TR_MSG("Read %zu Bytes into Buffer", m_config.chunk_size);
            if(!readFromPcm(m_audioBuffer.get(), m_config.chunk_size, read)) {
                return false;
            }
            bytesRead += read;

            if(!m_file.write(m_audioBuffer.get(), read)) {
                TR_MSG("Could not write to file %s", m_file.getFilename().c_str());
                return false;
            }
        }
        m_file.close();
        return true;
    };

private:
    AudioBuffer m_audioBuffer;
    Handle m_handle;
    HwParams m_hwparams;
    SwParams m_swparams;
    ConfigParams m_config;
    CaptureFile m_file;
    off_t m_bytesToRead = 0;
    bool m_init = false;

    off_t getBytesToRead(){
        TR();
        if(m_config.duration != -1 && m_config.samples != -1){
            fprintf(stderr, "Both duration and samples are set. Priority has samples: %d", m_config.samples);
        }
        off_t bytes = 0;
        if(m_config.samples > -1){
            bytes = snd_pcm_format_size(m_config.format, m_config.samples * m_config.channels);
        }
        else if(m_config.duration > -1){
            bytes = snd_pcm_format_size(m_config.format, m_config.rate * m_config.channels);
            bytes *= (off_t)m_config.duration;
        }
        // normalize bytes
        bytes = bytes < LLONG_MAX ? bytes + bytes % 2 : bytes - bytes % 2;
        return bytes;
    };

    bool readFromPcm(u_char* buff, snd_pcm_uframes_t size, size_t &read){
        TR();
        size_t readCountTotal = 0;
        constexpr int TIMEOUT = 50;
        while(readCountTotal < size) {
            TR_MSG("Attemp to read %ld bytes.", size);
            uint8_t data[size];
            ssize_t readCount = snd_pcm_readi(m_handle.get(), &data, size);
            TR_MSG("Got %ld Bytes", readCount);
            if( readCount == -EAGAIN || (readCount >= 0 && readCount < size)){
                snd_pcm_wait(m_handle.get(), TIMEOUT); 
            }
            // TODO handle pcm_state_changes (-EPIPE) ? see xrun
            // TODO handle pcm_resume (-ESTRPIPE) ? see suspend
            else if(readCount < 0){
                fprintf(stderr, "General Error. Abort");
                return false;
            }
            readCountTotal += readCount;
            buff += readCount * m_config.bits_per_frame / BITS_PER_BYTE;
        }
        read = readCountTotal;
        return true;
    };
};

#endif