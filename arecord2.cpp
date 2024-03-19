#include "config.hpp"
#include "recorder.hpp"

int main(){
    ConfigParams config{};
    // arecord -D hw:1,0 -c2 -fS16_LE -r48000 test.wav
    config.pcm_name = "hw:1,0";
    config.capture_file_name = "/home/alex/capture_test";
    fprintf(stderr, "Main\n");
    Recorder rec{config};
    if(!rec.init()){
        fprintf(stderr, "could not init recorder\n");
        return 1;
    }

    if(!rec.doRecord()){
        fprintf(stderr, "could not record\n");
        return 1;
    }

    return 0;
}
