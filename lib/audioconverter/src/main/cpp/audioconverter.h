//
// Created by test on 2022/11/6.
//

#ifndef FAAD2SAMPLE_AUDIOCONVERTER_H
#define FAAD2SAMPLE_AUDIOCONVERTER_H

#ifdef __cplusplus
extern "C" {
#endif
    int convertAac2MonoWav(const char *sourceAac, const char *destWave, const char *destMonoWave, int targetSampleRate);
#ifdef __cplusplus
}
#endif

#endif //FAAD2SAMPLE_AUDIOCONVERTER_H
