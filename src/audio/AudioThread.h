// Copyright (c) Charles J. Cliffe
// SPDX-License-Identifier: GPL-2.0+

#pragma once

#include <queue>
#include <vector>
#include <map>
#include <string>
#include <atomic>
#include <memory>
#include "ThreadBlockingQueue.h"
#include "RtAudio.h"
#include "DemodDefs.h"

class AudioThreadInput {
public:
    long long frequency;
    int inputRate;
    int sampleRate;
    int channels;
    float peak;
    int type;
    std::vector<float> data;

    AudioThreadInput() :
            frequency(0), sampleRate(0), channels(0), peak(0) {

    }

    virtual ~AudioThreadInput() {
       
    }
};

typedef std::shared_ptr<AudioThreadInput> AudioThreadInputPtr;

typedef ThreadBlockingQueue<AudioThreadInputPtr> DemodulatorThreadOutputQueue;

typedef std::shared_ptr<DemodulatorThreadOutputQueue> DemodulatorThreadOutputQueuePtr;

class AudioThreadCommand {
public:
    enum AudioThreadCommandEnum {
        AUDIO_THREAD_CMD_NULL, AUDIO_THREAD_CMD_SET_DEVICE, AUDIO_THREAD_CMD_SET_SAMPLE_RATE
    };

    AudioThreadCommand() :
            cmd(AUDIO_THREAD_CMD_NULL), int_value(0) {
    }

    AudioThreadCommandEnum cmd;
    int int_value;
};

typedef ThreadBlockingQueue<AudioThreadInputPtr> AudioThreadInputQueue;
typedef ThreadBlockingQueue<AudioThreadCommand> AudioThreadCommandQueue;

typedef std::shared_ptr<AudioThreadInputQueue> AudioThreadInputQueuePtr;
typedef std::shared_ptr<AudioThreadCommandQueue> AudioThreadCommandQueuePtr;

class AudioThread : public IOThread {

public:
   
    AudioThread();
    virtual ~AudioThread();

    static void enumerateDevices(std::vector<RtAudio::DeviceInfo> &devs);

    void setInitOutputDevice(int deviceId, int sampleRate=-1);
    int getOutputDevice();
   
    int getSampleRate();

    virtual void run();
    virtual void terminate();

    bool isActive();
    void setActive(bool state);

    void setGain(float gain_in);
   
    static std::map<int, int> deviceSampleRate;

    AudioThreadCommandQueue *getCommandQueue();

    //give access to the this AudioThread lock
    std::recursive_mutex& getMutex();

    static void deviceCleanup();
    static void setDeviceSampleRate(int deviceId, int sampleRate);

    //fields below, only to be used by other AudioThreads !
    size_t underflowCount;
    //protected by m_mutex
    std::vector<AudioThread *> boundThreads;
    AudioThreadInputQueuePtr inputQueue;
    AudioThreadInputPtr currentInput;
    size_t audioQueuePtr;
    float gain;

private:

    std::atomic_bool active;
    std::atomic_int outputDevice;
    
    RtAudio dac;
    unsigned int nBufferFrames;
    RtAudio::StreamOptions opts;
    RtAudio::StreamParameters parameters;
    AudioThreadCommandQueue cmdQueue;
    int sampleRate;

    //The own m_mutex protecting this AudioThread, in particular boundThreads
    std::recursive_mutex m_mutex;

    void setupDevice(int deviceId);
    void setSampleRate(int sampleRate);

    void bindThread(AudioThread *other);
    void removeThread(AudioThread *other);

    static std::map<int, AudioThread *> deviceController;
   
    static std::map<int, std::thread *> deviceThread;
};

