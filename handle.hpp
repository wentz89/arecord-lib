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
#ifndef _HANDLE_H_
#define _HANDLE_H_

extern "C" {
#include <alsa/asoundlib.h>
}
#include "config.hpp"
#include "common.hpp"

class Handle{
public:
    Handle(){
        TR_MSG("Handle");
    };
    ~Handle(){
        snd_pcm_drain(m_handle);
        if(m_handle){
            snd_pcm_close(m_handle);
        }
    };

    bool init(ConfigParams& config){
        TR();
        snd_pcm_info_t *pcminfo;
        snd_pcm_info_alloca(&pcminfo);
        MSG_AND_RETURN_IF(snd_pcm_open(&m_handle, config.pcm_name.c_str(), config.stream, config.pcm_open_mode) < 0, false, "Error open PCM. Abort.");
        MSG_AND_RETURN_IF(snd_pcm_info(m_handle, pcminfo) < 0, false, "pcm_info error. Abort.");
        // TODO nonblock ?
        return true;
    };

    inline snd_pcm_t* get() {
        return m_handle;
    };

private:
    snd_pcm_t *m_handle;
};

#endif