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
#ifndef _SND_PCM_PARAMS_H_
#define _SND_PCM_PARAMS_H_

extern "C"{
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
}

#include <map>

#include "common.hpp"
#include "config.hpp"

std::map<_snd_pcm_format, uint8_t> format2bytes =  {
    {SND_PCM_FORMAT_S8, 1},
    {SND_PCM_FORMAT_U8, 1},
    {SND_PCM_FORMAT_S16_LE, 2},
    {SND_PCM_FORMAT_S16_BE, 2},
    {SND_PCM_FORMAT_U16_LE, 2},
    {SND_PCM_FORMAT_U16_BE, 2}
    // todo finish this
};

class HwParams{
public:
    HwParams(){
        TR_MSG("Hwparams");
    };

    ~HwParams(){
        // TODO
    };

    bool init(snd_pcm_t *handle, HwConfig& config){
        TR();
        //snd_pcm_hw_params_t *params;
        snd_pcm_hw_params_alloca(&m_param);
        MSG_AND_RETURN_IF(handle == nullptr, false, "Handle is null");
        MSG_AND_RETURN_IF(snd_pcm_hw_params_any(handle, m_param) < 0, false, "Configuration for PCM broken.");
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_access(handle, m_param, config.access_mode) < 0, false, "Fail to set access mode %d", config.access_mode);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_format(handle, m_param, config.format) < 0, false, "Fail to set format %d", config.format);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_channels(handle, m_param, config.channels) < 0, false, "Fail to set channels %d", config.channels);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_rate_near(handle, m_param, &config.rate, 0) < 0, false, "Fail to set rate %u", config.rate);
        if(config.size_near > 0) {
            snd_pcm_uframes_t val = (snd_pcm_uframes_t)config.size_near;
            MSG_AND_RETURN_IF(snd_pcm_hw_params_set_period_size_near(handle, m_param, &val, 0) < 0, false, "Fail to set size near %zu", val);
        }
        MSG_AND_RETURN_IF(snd_pcm_hw_params(handle, m_param) < 0, false, "Could not set HW Params");
        m_config = config;
        MSG_AND_RETURN_IF(snd_pcm_hw_params_get_period_size(m_param, &m_periodSizeInSamples, 0) < 0, false, "could not get period size near.");
        return true;
    };

    int getPeriodSizeInBytes(){
        int samples = getPeriodSizeInSamples();
        if(samples < 0){
            TR_MSG("get samples failed");
            return -1;
        }
        int bytesPerSample = getBytesPerSample();
        if(bytesPerSample < 0){
            TR_MSG("get bytes per sample failed");
            return -1;
        }
        unsigned int val = samples * bytesPerSample;
        TR_MSG("Period Size in Bytes: %u", val);
        return val > INT_MAX ? -1 : (int)val;
    }

    int getPeriodSizeInSamples(){
        /*
        snd_pcm_uframes_t size;
        int res = snd_pcm_hw_params_get_period_size(m_param, &size, 0);
        if(res < 0){
            TR_MSG("get samples per period failed");
            return -1;
        }
        TR_MSG("Period Size in Samples: %zu", size);
        return (unsigned int)size > INT_MAX ? -1 : (int)size;
        */
        return m_periodSizeInSamples;
    }

    int getBytesPerSample(){
        snd_pcm_format_t format = m_config.format;
        /*
         * same problem as for get_channels below
        int res = snd_pcm_hw_params_get_format(m_param, &format);
        if(res < 0){
            TR_MSG("get formate failed");
            return -1;
        }
        */
        auto it = format2bytes.find(format);
        if(it == format2bytes.end()){
            TR_MSG("could not find formate %d", format);
            return -1;
        }
        uint8_t bytesPerSample = it->second;
        /*
         * Although this code works when called in init() it does not here.
         * I am pretty confused and dont know why...
        unsigned int numChannels = 0;
        res = snd_pcm_hw_params_get_channels(m_param, &numChannels);
        if(res < 0){
            TR_MSG("get channels failed");
            return -1;
        }
        */
        unsigned int val = bytesPerSample * m_config.channels;
        TR_MSG("Bytes per sample: %u", val);
        return val > INT_MAX ? -1 : (int)val;
    }

    int getPeriodTimeUs(){
        unsigned int val;
        int res = snd_pcm_hw_params_get_period_time(m_param, &val, 0);
        if(res < 0){
            TR_MSG("get period time failed");
            return -1;
        }
        TR_MSG("Period Time: %u us", val);
        return val > INT_MAX ? -1 : (int)val;
    }

private:
    snd_pcm_hw_params_t *m_param;
    snd_pcm_uframes_t m_periodSizeInSamples;
    HwConfig m_config;
};
#endif

/*
class SwParams{
public:
    SwParams(){
        TR_MSG("Swparams");
    };
    ~SwParams(){
        // todo
    };

    bool init(snd_pcm_t *handle, HwConfig& config){
        TR();
        snd_pcm_sw_params_t *param;
        snd_pcm_sw_params_alloca(&param);
        MSG_AND_RETURN_IF(handle == nullptr, false, "Handle is null");
        MSG_AND_RETURN_IF(snd_pcm_sw_params_current(handle, param) < 0, false, "Sw Configuration for PCM broken.");
        // not sure if next line is okay. TODO check/test
        config.avail_min = config.chunk_size;
        MSG_AND_RETURN_IF(snd_pcm_sw_params_set_avail_min(handle, param, config.avail_min) < 0, false, "Can not set avail min: %u", config.avail_min);
        config.start_threshold = config.start_threshold == 0 ? config.buffer_size : config.start_threshold;
        MSG_AND_RETURN_IF(snd_pcm_sw_params_set_start_threshold(handle, param, config.start_threshold) < 0, false, "Can not set start threshold: %zu.", config.start_threshold);
        config.stop_threshold = config.stop_threshold == 0 ? config.buffer_size : config.stop_threshold;
        MSG_AND_RETURN_IF(snd_pcm_sw_params_set_stop_threshold(handle, param, config.stop_threshold) < 0, false, "Can not set stop threshold: %zu.", config.stop_threshold);
        MSG_AND_RETURN_IF(snd_pcm_sw_params(handle, param) < 0, false, "Sw Configuration for PCM could not be installed.");
        m_param = param;
        return true;
    };

private:
    snd_pcm_sw_params_t *m_param;
};
*/