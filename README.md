# Alsa Recorder (and in future Player) Refactored

A refactored cpp-version for alsa sound recording and playing(future). This is a work in progress.
In the moment it offers a library for recording sound via a fairly simple interface. To use it:
- create config structs HwConfig, CaptureConfig
- create object of class recorder and call init() function
- start recording (either with max duration, max samples or empty for infinite recording time)
- wait for recorder to be finished (bool hasFinished()) or stop it manually (void stop())
- profit


TODO:
- separate into public(recorder.hpp, config.hpp) and private interface
- create unit tests
- improve docu
- create exec with commandline options (similar to arecord)
- adapt cmakelists to build lib and exec
- setup debian folder
- improve logging
- clangformat file
- create sound analyzer (lenght, frequency, amplitude) of sound data
- work on player library and exec 