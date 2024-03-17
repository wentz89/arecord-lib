#include "config.hpp"
#include "recorder.hpp"

int main(){
    ConfigParams config{};
    config.pcm_name = "hw:1,0";
    config.capture_file_name = "~/capture_test";
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
