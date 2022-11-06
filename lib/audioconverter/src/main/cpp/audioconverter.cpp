#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <functional>
#include <string>
#include <fstream>
#include <iostream>

#include "audioconverter.h"
#include "faad2/include/neaacdec.h"

#define FRAME_MAX_LEN  1024*5
#define BUFFER_MAX_LEN 1024*1024

#if __ANDROID__
#include <android/log.h>
#define LOG(...)        __android_log_print(ANDROID_LOG_DEBUG, "LOG", __VA_ARGS__)
#else
//    #include <cstdio>
    #define LOG(fmt, ...)   do{printf(fmt, ##__VA_ARGS__); printf("\n");} while(0)

#endif

using namespace std;

struct WavFileHeader
{
    char        id[4];          // should always contain "RIFF"
    int     totallength;    // total file length minus 8
    char        wavefmt[8];     // should be "WAVEfmt "
    int     format;         // 16 for PCM format
    short     pcm;            // 1 for PCM format
    short     channels;       // channels
    int     frequency;      // sampling frequency
    int     bytes_per_second;
    short     bytes_by_capture;
    short     bits_per_sample;
    char        data[4];        // should always contain "data"
    int     bytes_in_data;
};


class WavHeader{
public:
    // RIFF Descriptor
    uint8_t  RIFF[4];
    uint32_t fileSize;
    uint8_t WAVE[4];

    // fmt sub-chunk
    uint8_t fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t noChannels;
    uint32_t sampleRate;
    uint32_t bytesPerSec;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    // data sub-chunk
    uint8_t data[4];
    uint32_t dataSize;

    WavHeader(uint32_t fileSize, uint16_t noChannels, uint32_t sampleRate,
              uint16_t bitsPerSample, uint32_t dataSize){

        // RIFF descriptor
        RIFF[0] = 'R';RIFF[1] = 'I';RIFF[2] = 'F';RIFF[3] = 'F';
        this->fileSize = fileSize;
        WAVE[0] = 'W';WAVE[1] = 'A';WAVE[2] = 'V';WAVE[3] = 'E';

        // fmt sub-chunk
        fmt[0] = 'f';fmt[1] = 'm';fmt[2] = 't';fmt[3] = 32;
        fmtSize = 16;
        audioFormat = 1;
        this->noChannels = noChannels;
        this->sampleRate = sampleRate;
        this->bitsPerSample = bitsPerSample;
        blockAlign = (bitsPerSample * noChannels) / 8;
        bytesPerSec = (sampleRate * bitsPerSample * noChannels) / 8;

        // data sub-chunk
        data[0] = 'd';data[1] = 'a';data[2] = 't';data[3] = 'a';
        this->dataSize = dataSize;

    }

    WavHeader(){}

};

void write_wav_header(FILE* file, int totalsamcnt_per_channel, int samplerate, int channels){
    struct WavFileHeader filler;
    memcpy(filler.id, "RIFF",4);
    filler.bits_per_sample = 16;
    filler.totallength = (totalsamcnt_per_channel * channels * filler.bits_per_sample/8) + sizeof(filler); //81956
    memcpy(filler.wavefmt, "WAVEfmt ",8);
    filler.format = 16;
    filler.pcm = 1;
    filler.channels = channels;
    filler.frequency = samplerate;
    filler.bytes_per_second = filler.channels * filler.frequency * filler.bits_per_sample/8;
    filler.bytes_by_capture = filler.channels*filler.bits_per_sample/8;
    filler.bytes_in_data = totalsamcnt_per_channel * filler.channels * filler.bits_per_sample/8;
    memcpy(filler.data, "data",4);
    fwrite(&filler, 1, sizeof(filler), file);
}



static float dataToFloat(uint16_t num){

    /*
     * converting channel data from 16-bit integer data to
     * fractional binary fixed-point numbers
     *
     * The most significant (16th) bit is used as the
     * sign bit. If sign bit is 1, it is a negative number
     * we need to find its 2's complement.
     *
     * fractional fixed-point numbers are obtained from
     * integer fixed-point numbers by dividing them by 2 ^ (N - 1)
     * here N = 16 (bitsPerSample)
    */

    uint16_t signBit = 32768;
    int multiplier = 1;

    // check if sign bit is 1
    if( signBit & num ){
        // 1's complement
        num ^= 65535;
        // +1 to get 2's complement
        num++;
        // setting multiplier to -1 to change final answer to negative
        multiplier = -1;
    }

    return multiplier * ((float)num) / pow(2, 15);

}


uint16_t floatToData(float num){

    /*
     * converting fractional fixed-point number
     * to 16-bit integer
     *
     * This is just the reverse process of dataToFloat(..)
    */

    // if num is negative, set signBit to 1
    uint16_t signBit = (num < 0) ? 32768 : 0;

    num = abs(num);
    num *= pow(2, 15);

    uint16_t ans = num;

    // if it is a negative number, get 2's complement
    if( signBit ){
        // 1's complement (invert all bits)
        ans ^= 65535;
        // +1 to get 2's complement
        ans++;
    }

    return ans;

}

/**
 * fetch one ADTS frame
 */
static int get_one_ADTS_frame(unsigned char* buffer, size_t buf_size, unsigned char* data ,size_t* data_size){
    size_t size = 0;
    if(!buffer || !data || !data_size ){
        return -1;
    }
    while(1){
        if(buf_size  < 7 ){
            return -1;
        }
        if((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0) ){
            size |= ((buffer[3] & 0x03) <<11);     //high 2 bit
            size |= buffer[4]<<3;                //middle 8 bit
            size |= ((buffer[5] & 0xe0)>>5);        //low 3bit
            break;
        }
        --buf_size;
        ++buffer;
    }
    if(buf_size < size){
        return -1;
    }
    memcpy(data, buffer, size);
    *data_size = size;
    return 0;
}


std::string steroToMono(std::string filePath, string outFilePath){

    std::ifstream inFile(filePath);

    // read input file wav header
    WavHeader header;
    inFile.read((char *) &header, sizeof(header));

    int a = sizeof(header);
    if( header.noChannels != 2 ){
        cout <<"This file is not stereo, it has " <<header.noChannels
             <<" channels\nPlease provide a stereo file";
        return filePath;
    }

    clock_t start, end;
    start = clock();

    // get unique output file name
    string outputFileName = outFilePath; //getOutputFileName(getFileBaseName(filePath));

    // change header data
    header.noChannels = 1;
    header.blockAlign = (header.bitsPerSample * header.noChannels) / 8;
    header.bytesPerSec = (header.sampleRate * header.bitsPerSample * header.noChannels) / 8;
    header.fileSize = header.dataSize = 0;

    // write wav header
    ofstream outFile(outputFileName, ios::binary);
    outFile.write((char*) &header, sizeof(header));

    uint16_t channel1, channel2;
    uint32_t dataSize = 0;
    while( ! inFile.eof() ){

        // read contents of both channel
        inFile.read((char*) &channel1, sizeof(channel1));
        inFile.read((char*) &channel2, sizeof(channel2));

        // if data on both channel is different, find average
        if( channel1 != channel2 ){
            channel1 = floatToData((dataToFloat(channel1) + dataToFloat(channel2)) / 2);
        }

        // write data of single channel to output file
        outFile.write((char*)&channel1, sizeof(channel1));

        // count the number of bytes written to file
        dataSize += sizeof(channel1);

    }

    // write new data size to wav header
    outFile.seekp(40, ios_base::beg);
    outFile.write((char*) &dataSize, sizeof(dataSize));

    // write new file size to wav header
    dataSize += 44;
    outFile.seekp(4, ios_base::beg);
    outFile.write((char*) &dataSize, sizeof(dataSize));

    inFile.close();
    outFile.close();

    end = clock();

    cout <<"Completed in " <<((float)(end - start)) / CLOCKS_PER_SEC <<" seconds\n";
    cout <<"File saved as : " <<outputFileName <<"\n\n";

    return outputFileName;
}


int convertAac2MonoWav(const char *sourceAac, const char *destWave, const char *destMonoWave, int targetSampleRate) {
    LOG("convertAac2MonoWav: sourceAac: %s, destWave: %s, sampleRate: %d, LC: %d\n", sourceAac, destWave, targetSampleRate, LC);

    static unsigned char frame[FRAME_MAX_LEN] = {0};
    static unsigned char buffer[BUFFER_MAX_LEN] = {0};

    int result = 1;

    FILE *ifile = NULL;
    unsigned long samplerate;
    unsigned char channels;

    NeAACDecHandle decoder = 0;
    size_t data_size = 0;
    size_t size = 0;

    NeAACDecFrameInfo frame_info;
    unsigned char *input_data = buffer;
    unsigned char *pcm_data = NULL;

    ifile = fopen(sourceAac, "rb");
    if(!ifile){
        printf("source or destination file");
        return -1;
    }
    data_size = fread(buffer, 1, BUFFER_MAX_LEN, ifile);

    //open decoder
    decoder = NeAACDecOpen();
    NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(decoder);
    if(!conf){
        printf("NeAACDecGetCurrentConfiguration failed\n");
    }
    conf->defObjectType = LC;
    conf->defSampleRate = targetSampleRate;
    conf->outputFormat = FAAD_FMT_16BIT;
    conf->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(decoder, conf);

    if(get_one_ADTS_frame(buffer, data_size, frame, &size) < 0){
        return -1;
    }

    NeAACDecInit(decoder, frame, size, &samplerate, &channels);
    printf("samplerate %lu, channels %d\n", samplerate, channels);

    const char* wavename = destWave;
    FILE* wavfile = fopen(wavename, "wb");
    if (wavfile) {
        int wavheadsize = sizeof(struct WavFileHeader);
        fseek(wavfile, wavheadsize, SEEK_SET);

        int totalsmp_per_chl = 0;
        while(get_one_ADTS_frame(input_data, data_size, frame, &size) == 0){
//            printf("frame size %zu\n", size);

            // decode ADTS frame
            pcm_data = (unsigned char*)NeAACDecDecode(decoder, &frame_info, frame, size);

            if(frame_info.error > 0){
                printf("%s\n",NeAACDecGetErrorMessage(frame_info.error));
            }else if(pcm_data && frame_info.samples > 0){

//                printf("frame info: bytesconsumed %lu, channels %d, header_type %d\
//                       object_type %d, samples %lu, samplerate %lu\n",
//                       frame_info.bytesconsumed,
//                       frame_info.channels, frame_info.header_type,
//                       frame_info.object_type, frame_info.samples,
//                       frame_info.samplerate);

                fwrite(pcm_data, 1, frame_info.samples*2, wavfile); // frameinfo.samples是所有声道数的样本总和；16bit位深
//                [audioPlay openAudioFromQueue:pcm_data dataSize:(frame_info.samples * frame_info.channels) samplerate:(int)frame_info.samplerate channels:frame_info.channels aBit:16];
                /*
                 pcm_data 为解码后的数据
                 (frame_info.samples * frame_info.channels)  为解码后的数据大小
                 */
                data_size -= size;
                input_data += size;
                totalsmp_per_chl += frame_info.samples / frame_info.channels;;
            }
        }
        printf("aac decode done, totalsmp_per_chl=%d\n", totalsmp_per_chl);
        fseek(wavfile, 0, SEEK_SET);
        write_wav_header(wavfile, totalsmp_per_chl, (int)samplerate, (int)channels);
        fclose(wavfile);
        result = 0;
    }

    NeAACDecClose(decoder);
    fclose(ifile);

    if (result == 0) {
        steroToMono(destWave, destMonoWave);
    }

    return 0;
}