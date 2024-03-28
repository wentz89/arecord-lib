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
#include <thread>
#include <atomic>
#include <stdlib.h>

//#include "audio_buffer.hpp"
#include "config.hpp"
#include "snd_pcm_params.hpp"
#include "handle.hpp"
#include "common.hpp"
#include "capture_handle.hpp"

enum class DurationMs : int;
enum class SampleCount : int;

constexpr int INFINITE = -1;
constexpr int DEFAULT_RECORDER_SIZE_NEAR = 512;

class Recorder{
public:
    Recorder(HwConfig &config, CaptureConfig capture) : m_handle(), m_thread(), m_hwparams(), m_config(config), m_capture(capture)
    {
        TR_MSG("Recorder");
    };
    ~Recorder() {};

    bool init(){
        TR();
        if(m_config.size_near < 0){
            m_config.size_near = DEFAULT_RECORDER_SIZE_NEAR;
        }
        MSG_AND_RETURN_IF(m_handle.init(m_config) == false, false, "Handle could not be initialized");
        MSG_AND_RETURN_IF(m_hwparams.init(m_handle.get(), m_config) == false, false, "HwParams could not be initialized");
        m_periodTimeUs = m_hwparams.getPeriodTimeUs();
        MSG_AND_RETURN_IF(m_periodTimeUs <= 0, false, "Failed to get Period Time.");
        m_periodSizeInBytes = m_hwparams.getPeriodSizeInBytes();
        MSG_AND_RETURN_IF(m_periodSizeInBytes < 0, false, "Failed to get Period Size.");
        int bytesPerSample = m_hwparams.getBytesPerSample();
        MSG_AND_RETURN_IF(bytesPerSample < 0, false, "failed to get bytes per sample");
        MSG_AND_RETURN_IF(m_capture.init(m_config, bytesPerSample) == false, false, "Failed init capture handler");
        m_init = true;
        return true;
    };

    bool start(){
        SampleCount count = static_cast<SampleCount>(INFINITE);
        return start(count);
    }

    bool start(DurationMs duration){
        if((int)duration < 0){
            return start();
        }
        unsigned int durUs = static_cast<unsigned int>(duration) * 1000;
        double periodCount = durUs / m_periodTimeUs;
        double sampleCount = periodCount * (double)m_hwparams.getPeriodSizeInSamples();
        SampleCount count = static_cast<SampleCount>(sampleCount+0.5); // magic to always round up
        return start(count);
    }

    bool start(SampleCount count){
        if(!m_init){
            return false;
        }
        m_stop = false;
        m_isFinished = false;
        m_thread = std::thread(&Recorder::internalStart, this, (int)count);
        return true;
    }

    void stop(bool force = false){
        m_stop = true;
        if(force){
            // todo use native thread handle and kill
        }
    }

    bool hasFinished(){
        return m_isFinished;
    }

private:
    Handle m_handle;
    std::thread m_thread;
    HwParams m_hwparams;
    HwConfig m_config;
    CaptureHandle m_capture;
    std::atomic_bool m_stop{false};
    std::atomic_bool m_isFinished{false};
    bool m_init = false;
    int m_periodTimeUs = 0;
    int m_periodSizeInBytes = 0;

    void internalStart(int totalSamplesToRead){
        int bytesPerSample = m_hwparams.getBytesPerSample();

        unsigned int totalBytesToRead = totalSamplesToRead * bytesPerSample;
        unsigned int loopCount = totalBytesToRead / m_periodSizeInBytes;

        u_char* buffer = (u_char*)malloc(m_periodSizeInBytes);
        off_t bytesRead = 0;
        int samplesPerPeriod = m_hwparams.getPeriodSizeInSamples();
        if(bytesPerSample < 0 || samplesPerPeriod < 0 ){
            TR_MSG("Abort. Samples|Bytes = %d|%d",samplesPerPeriod, bytesPerSample);
            m_isFinished = true;
            return;
        }
        TR_MSG("Attempt to read %d samples", totalSamplesToRead);
        while((!m_stop && totalSamplesToRead == INFINITE) 
              || ( totalSamplesToRead != INFINITE && bytesRead < totalBytesToRead && !m_stop)) {
            size_t read = 0;
            if(!readFromPcm(buffer, samplesPerPeriod, bytesPerSample, read)) {
                break;
            }
            bytesRead += read;

            if(!m_capture.write(buffer, read)) {
                TR_MSG("Failed to write.");
                break;
            }
        }
        m_isFinished = true;
    }

    bool readFromPcm(u_char* buff, snd_pcm_uframes_t samplesToRead, int bytesPerSample, size_t &read){
        size_t readCountTotal = 0;
        while(readCountTotal < samplesToRead) {
            ssize_t readCount = 0;
            buff += (readCount * bytesPerSample);
            snd_pcm_uframes_t toRead = samplesToRead - readCountTotal;
            readCount = snd_pcm_readi(m_handle.get(), buff, toRead);
            if (readCount == -EPIPE) {
                TR_MSG("pipe overrun occurred");
                snd_pcm_prepare(m_handle.get());
                continue;
            }
            if(readCount < 0){
                TR_MSG("General Error. Abort");
                return false;
            }
            readCountTotal += readCount;
        }
        read = readCountTotal * bytesPerSample ;
        return true;
    };
};

#endif