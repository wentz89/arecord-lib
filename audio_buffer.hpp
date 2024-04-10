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

#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_

#include <cstdlib>
#include <mutex>
#include <stdint.h>

#include "common.hpp"


class AudioBuffer{
public:
    AudioBuffer(){
        TR_MSG("AudioBuffer");
    };
    ~AudioBuffer(){
        if(m_audioBuffer){
            free(m_audioBuffer);
        }
    };

    uint8_t* get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_audioBuffer;
    };

    bool init(uint64_t maxSize){
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_audioBuffer) {
            free(m_audioBuffer);
            m_audioBuffer = nullptr;
        }
        TR_MSG("Allocation %zu Bytes", maxSize);
        m_audioBuffer = (uint8_t *)malloc(maxSize);
        MSG_AND_RETURN_IF(m_audioBuffer == nullptr, false, "Could not allocate memory");
        m_maxSize = maxSize;
        m_pos = 0;
        m_size = 0;
        return true;
    };

    void add(uint8_t* data, uint64_t size){
        std::lock_guard<std::mutex> lock(m_mutex);
        // Cases
        // size = maxSize -> (easy)  memcpy(m_audioBuffer + 0, data + 0, size); m_pos += size % maxSize // +0;
        // size > maxSize -> add last n bytes. n = maxSize, data += (size - maxSize), m_pos += n % maxSize
        // size < maxSize
        //      m_pos + size < maxSize -> memcpy(m_audioBuffer + m_pos, data + 0, size); m_pos += size % maxSize // + size;
        //      m_pos + size > maxSize -> memcpy(m_audioBuffer + m_pos, data + 0, size - off1); // update pos; memcpy(m_audioBuffer + m_pos, data + off2, size - off3);
        bool exceedsMaxSize = size >= m_maxSize;
        bool hasRollover = ((m_pos + size) > m_maxSize) && !exceedsMaxSize;
        uint64_t sizeToAdd = (hasRollover || exceedsMaxSize) * (m_maxSize - m_pos) + !hasRollover * size;
        uint64_t dataOffset = exceedsMaxSize * (size - m_maxSize) + !exceedsMaxSize * 0;
        uint64_t bufferOffset = exceedsMaxSize * 0 + !exceedsMaxSize * m_pos;
    
        memcpy(m_audioBuffer + bufferOffset, data + dataOffset, sizeToAdd);
        if(hasRollover){
            uint64_t remaining = size - sizeToAdd;
            uint64_t off2 = sizeToAdd;
            memcpy(m_audioBuffer + 0, data + off2, remaining);
        }
        m_size = full() ? m_maxSize : m_size + sizeToAdd;
        m_pos += size;
    }

    bool full(){
        return m_size == m_maxSize;
    }

private:
    uint8_t *m_audioBuffer = nullptr;
    uint64_t m_maxSize;
    uint64_t m_pos = 0;
    uint64_t m_size = 0;
    std::mutex m_mutex;
};

#endif