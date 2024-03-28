#include "config.hpp"
#include "recorder.hpp"

int main(){
    HwConfig config{};
    CaptureConfig cconfig{};
    cconfig.mode = CAPTURE_MODE::WAV;
    cconfig.wav_file_name = "/tmp/wav_record.wav";
    cconfig.overwriteExistingFiles = true;
    Recorder rec{config, cconfig};
    if(!rec.init()){
        fprintf(stderr, "could not init recorder\n");
        return 1;
    }

    rec.start();
    usleep(10000000);
    rec.stop();
    usleep(500000);
    fprintf(stderr, "Finished %s", rec.hasFinished() ? "true": "false" );

    return 0;
}
