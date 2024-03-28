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
#ifndef _CONFIG_H_
#define _CONFIG_H_
extern "C"{
#include <alsa/asoundlib.h>
}
#include <string>

struct HwConfig{
  std::string pcm_name = "default";
  unsigned int channels = 2;
  unsigned int rate = 48000;
  snd_pcm_access_t access_mode = SND_PCM_ACCESS_RW_INTERLEAVED;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
  snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
  // optional will be set internaly if -1
  int size_near = -1;
};

enum CAPTURE_MODE{
  STDOUT = 0x1,
  RAW    = 0x2,
  WAV    = 0x4
};

struct CaptureConfig{
  std::string raw_file_name = "";
  std::string wav_file_name = "";
  CAPTURE_MODE mode = CAPTURE_MODE::STDOUT;
  bool overwriteExistingFiles = true;
};

/*
struct ConfigParams{
    std::string capture_file_name;
    std::string pcm_name = "hw:1,0";
    int pcm_open_mode = 0;
    snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
    snd_pcm_access_t access_mode = SND_PCM_ACCESS_RW_INTERLEAVED;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    snd_pcm_subformat_t subformat = SND_PCM_SUBFORMAT_STD;
    snd_pcm_uframes_t buff_frames = 24064;
    unsigned int buffer_time = 0;
    snd_pcm_uframes_t period_frames = 0;
    unsigned int period_time = 125333;
    snd_pcm_uframes_t start_threshold = 1;
    snd_pcm_uframes_t stop_threshold = 24064;
    snd_pcm_uframes_t chunk_size = 1024;
    size_t chunk_bytes = 0;
    snd_pcm_uframes_t buffer_size = 0;
    unsigned int channels = 2;
    unsigned int rate = 48000;
    unsigned int avail_min = 6016;
    size_t bits_per_sample = 0;
    size_t bits_per_frame = 0;

    // Record specific
    // take either duration or samples. If both are set samples has priority
    int duration = -1;
    int samples = 16384;
};
*/

#endif