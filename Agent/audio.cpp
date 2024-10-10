//#include <iostream>
//#include <vector>
//#include <portaudio.h>
//#include <sndfile.h>
//
//#define SAMPLE_RATE 44100
//#define FRAMES_PER_BUFFER 512
//#define NUM_CHANNELS 2
//#define DURATION 60  // 录音时长（秒）
//
//struct RecordingData {
//    std::vector<float> recordedSamples;
//    int maxFrameIndex;
//    int frameIndex;
//};
//
//static int recordCallback(const void* inputBuffer, void* outputBuffer,
//    unsigned long framesPerBuffer,
//    const PaStreamCallbackTimeInfo* timeInfo,
//    PaStreamCallbackFlags statusFlags,
//    void* userData) {
//    RecordingData* data = (RecordingData*)userData;
//    const float* in = (const float*)inputBuffer;
//    float* out = data->recordedSamples.data() + data->frameIndex * NUM_CHANNELS;
//
//    if (inputBuffer == nullptr) {
//        for (unsigned long i = 0; i < framesPerBuffer; i++) {
//            *out++ = 0;  // 左声道
//            if (NUM_CHANNELS == 2) *out++ = 0;  // 右声道
//        }
//    }
//    else {
//        for (unsigned long i = 0; i < framesPerBuffer; i++) {
//            *out++ = *in++;  // 左声道
//            if (NUM_CHANNELS == 2) *out++ = *in++;  // 右声道
//        }
//    }
//
//    data->frameIndex += framesPerBuffer;
//    return paContinue;
//}
//
//int main() {
//    PaError err;
//    PaStream* stream;
//
//    // 初始化PortAudio
//    err = Pa_Initialize();
//    if (err != paNoError) {
//        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
//        return -1;
//    }
//
//    // 计算总帧数
//    int totalFrames = DURATION * SAMPLE_RATE;
//    RecordingData data;
//    data.maxFrameIndex = totalFrames;
//    data.frameIndex = 0;
//    data.recordedSamples.resize(totalFrames * NUM_CHANNELS);
//
//    // 打开音频流
//    err = Pa_OpenDefaultStream(&stream,
//        NUM_CHANNELS,  // 输入通道数
//        0,             // 输出通道数
//        paFloat32,     // 采样格式
//        SAMPLE_RATE,
//        FRAMES_PER_BUFFER,
//        recordCallback,  // 回调函数
//        &data);         // 用户数据
//    if (err != paNoError) {
//        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
//        return -1;
//    }
//
//    // 开始音频流
//    err = Pa_StartStream(stream);
//    if (err != paNoError) {
//        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
//        return -1;
//    }
//
//    // 录音DURATION秒
//    Pa_Sleep(DURATION * 1000);
//
//    // 停止音频流
//    err = Pa_StopStream(stream);
//    if (err != paNoError) {
//        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
//        return -1;
//    }
//
//    // 关闭音频流
//    err = Pa_CloseStream(stream);
//    if (err != paNoError) {
//        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
//        return -1;
//    }
//
//    // 终止PortAudio
//    Pa_Terminate();
//
//    // 使用libsndfile保存录制的音频到文件
//    SF_INFO sfinfo;
//    sfinfo.samplerate = SAMPLE_RATE;
//    sfinfo.frames = totalFrames;
//    sfinfo.channels = NUM_CHANNELS;
//    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//
//    SNDFILE* sndfile = sf_open("D:\\Code\\Visual_Studio\\Mike_C2\\Mike_C2\\x64\\Debug\\recorded_audio.wav", SFM_WRITE, &sfinfo);
//    if (!sndfile) {
//        std::cerr << "Error: could not open file for writing." << std::endl;
//        return -1;
//    }
//
//    sf_writef_float(sndfile, data.recordedSamples.data(), totalFrames);
//    sf_close(sndfile);
//
//    std::cout << "Recording completed and saved to 'recorded_audio.wav'" << std::endl;
//
//    return 0;
//}
