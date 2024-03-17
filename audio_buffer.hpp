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

#include <alsa/asoundlib.h>

#include "common.hpp"
#include "snd_pcm_params.hpp"

constexpr size_t BITS_PER_BYTE = 8;

class AudioBuffer{
public:
    AudioBuffer(){
        fprintf(stderr, "AudioBuffer\n");
        m_audioBuffer = (u_char *)malloc(1024);
    };
    ~AudioBuffer(){
        if(m_audioBuffer){
            free(m_audioBuffer);
        }
    };

    u_char* get() {
        return m_audioBuffer;
    };

    bool init(ConfigParams& config){
        TR();
        MSG_AND_RETURN_IF(m_audioBuffer == nullptr, false, "Audio Buffer is Null. Abort.");
        config.bits_per_sample = snd_pcm_format_physical_width(config.format);
        config.bits_per_frame = config.bits_per_sample / config.channels;
        config.chunk_bytes = config.chunk_size * config.bits_per_frame / BITS_PER_BYTE;
        m_audioBuffer = (u_char*)realloc(m_audioBuffer, config.chunk_bytes);
        MSG_AND_RETURN_IF(m_audioBuffer == nullptr, false, "Could not allocate memory");
        config.buff_frames = config.buffer_size;
        return true;
    };
private:
    u_char *m_audioBuffer = nullptr;
};

#endif