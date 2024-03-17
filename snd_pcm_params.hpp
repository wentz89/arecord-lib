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

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#include "common.hpp"
#include "config.hpp"
             

class HwParams{
public:
    HwParams(){
        TR_MSG("Hwparams");
    };

    ~HwParams(){
        // TODO
    };

    bool init(snd_pcm_t *handle, ConfigParams& config){
        TR();
        snd_pcm_hw_params_t *params;
        snd_pcm_hw_params_alloca(&params);
        MSG_AND_RETURN_IF(handle == nullptr, false, "Handle is null");
        MSG_AND_RETURN_IF(snd_pcm_hw_params_any(handle, params) < 0, false, "Configuration for PCM broken.");
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_access(handle, params, config.access_mode) < 0, false, "Fail to set access mode %d", config.access_mode);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_format(handle, params, config.format) < 0, false, "Fail to set format %d", config.format);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_subformat(handle, params, config.subformat) < 0, false, "Fail to set subformat %d", config.subformat);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_channels(handle, params, config.channels) < 0, false, "Fail to set channels %d", config.channels);
        MSG_AND_RETURN_IF(snd_pcm_hw_params_set_rate_near(handle, params, &config.rate, 0) < 0, false, "Fail to set rate %u", config.rate);
        MSG_AND_RETURN_IF(updateBuffer(handle, config, params) != true, false, "Failed to update Buffer Parameter");
        MSG_AND_RETURN_IF(updatePeriod(handle, config, params) != true, false, "Failed to update Period Parameter");
        MSG_AND_RETURN_IF(snd_pcm_hw_params(handle, params) < 0, false, "Could not set HW Params");
        MSG_AND_RETURN_IF(checkSize(handle, config, params) != true, false, "Could not set HW Params");
        m_param = params;
        return true;
    };

private:
    snd_pcm_hw_params_t *m_param;

    bool updateBuffer(snd_pcm_t *handle, ConfigParams& config, snd_pcm_hw_params_t *params) {
        TR();
        if(config.buffer_time > 0){
            MSG_AND_RETURN_IF(snd_pcm_hw_params_set_buffer_time_near(handle, params, &config.buffer_time, 0) < 0, false, "Could not set Buffer Time to %u", config.buffer_time);
        } else if( config.buffer_time == 0 && config.buff_frames > 0) {
            MSG_AND_RETURN_IF(snd_pcm_hw_params_set_buffer_size_near(handle, params, &config.buff_frames) < 0, false, "Could not set Buffer size to %zu", config.buff_frames);
        } else {
            MSG_AND_RETURN_IF(snd_pcm_hw_params_get_buffer_time_max(params, &config.buffer_time, 0) < 0, -1, "Not able to get Max Buffer Time");
            MSG_AND_RETURN_IF(snd_pcm_hw_params_set_buffer_time_near(handle, params, &config.buffer_time, 0) < 0, false, "Could not set Buffer Time to %u", config.buffer_time);
        }
        return true;
    };

    bool updatePeriod(snd_pcm_t *handle, ConfigParams& config, snd_pcm_hw_params_t *params) {
        TR();
        if(config.period_frames == 0 && config.period_time == 0){
            config.period_frames = config.buff_frames / 4;
            config.period_time = config.buffer_time / 4;
        }

        if(config.period_time > 0){
            MSG_AND_RETURN_IF(snd_pcm_hw_params_set_period_time_near(handle, params, &config.period_time, 0) < 0, false, "Could not set Period Time to %u", config.period_time);
        } else if( config.period_time == 0 && config.period_frames > 0) {
            MSG_AND_RETURN_IF(snd_pcm_hw_params_set_period_size_near(handle, params, &config.period_frames, 0) < 0, false, "Could not set period size to %zu", config.period_frames);
        }
        return true;
    };

    bool checkSize(snd_pcm_t *handle, ConfigParams& config, snd_pcm_hw_params_t *params) {
        TR();
        MSG_AND_RETURN_IF(snd_pcm_hw_params_get_period_size(params, &config.chunk_size, 0) < 0, false, "Could not get period size");
        MSG_AND_RETURN_IF(snd_pcm_hw_params_get_buffer_size(params, &config.buffer_size) < 0, false, "Could not get buffer size");
        MSG_AND_RETURN_IF(config.buffer_size == config.chunk_size, false, "Buffer Size and Chunk Size should not be equal.");
        return true;
    }
};

class SwParams{
public:
    SwParams(){
        TR_MSG("Swparams");
    };
    ~SwParams(){
        // todo
    };

    bool init(snd_pcm_t *handle, ConfigParams& config){
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

#endif