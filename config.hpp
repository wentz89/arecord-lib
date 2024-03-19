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

/*
Hardware PCM card 1 'HD-Audio Generic' device 0 subdevice 0
Its setup is:
  stream       : CAPTURE
  access       : RW_INTERLEAVED
  format       : S16_LE
  subformat    : STD
  channels     : 2
  rate         : 48000
  exact rate   : 48000 (48000/1)
  msbits       : 16
  buffer_size  : 24064
  period_size  : 6016
  period_time  : 125333
  tstamp_mode  : NONE
  tstamp_type  : MONOTONIC
  period_step  : 1
  avail_min    : 6016
  period_event : 0
  start_threshold  : 1
  stop_threshold   : 24064
  silence_threshold: 0
  silence_size : 0
  boundary     : 6773413839565225984
  appl_ptr     : 0
  hw_ptr       : 0
^CAborted by signal Interrupt...
*/

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

#endif