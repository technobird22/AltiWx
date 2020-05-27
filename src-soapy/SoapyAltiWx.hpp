#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>
#include <rtl-sdr.h>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include "zmq.hpp"

typedef enum rtlsdrRXFormat
{
    RTL_RX_FORMAT_FLOAT32,
    RTL_RX_FORMAT_INT16,
    RTL_RX_FORMAT_INT8
} rtlsdrRXFormat;

#define DEFAULT_BUFFER_LENGTH (16 * 32 * 512)
#define DEFAULT_NUM_BUFFERS 15
#define BYTES_PER_SAMPLE 2

class SoapyAltiWx : public SoapySDR::Device
{
public:
    SoapyAltiWx(const SoapySDR::Kwargs &args);

    ~SoapyAltiWx(void);

    std::ifstream fileIn;

    zmq::context_t zmqContext{1};
    zmq::socket_t zmqSocket{zmqContext, zmq::socket_type::dealer};

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

    /*******************************************************************
     * Channels API
     ******************************************************************/

    size_t getNumChannels(const int) const;

    bool getFullDuplex(const int direction, const size_t channel) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/

    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

    SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

    SoapySDR::Stream *setupStream(const int direction, const std::string &format, const std::vector<size_t> &channels = std::vector<size_t>(), const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    void closeStream(SoapySDR::Stream *stream);

    size_t getStreamMTU(SoapySDR::Stream *stream) const;

    int activateStream(
        SoapySDR::Stream *stream,
        const int flags = 0,
        const long long timeNs = 0,
        const size_t numElems = 0);

    int deactivateStream(SoapySDR::Stream *stream, const int flags = 0, const long long timeNs = 0);

    int readStream(
        SoapySDR::Stream *stream,
        void *const *buffs,
        const size_t numElems,
        int &flags,
        long long &timeNs,
        const long timeoutUs = 100000);

    /*******************************************************************
     * Direct buffer access API
     ******************************************************************/

    size_t getNumDirectAccessBuffers(SoapySDR::Stream *stream);

    int getDirectAccessBufferAddrs(SoapySDR::Stream *stream, const size_t handle, void **buffs);

    int acquireReadBuffer(
        SoapySDR::Stream *stream,
        size_t &handle,
        const void **buffs,
        int &flags,
        long long &timeNs,
        const long timeoutUs = 100000);

    void releaseReadBuffer(
        SoapySDR::Stream *stream,
        const size_t handle);

    /*******************************************************************
     * Antenna API
     ******************************************************************/

    std::vector<std::string> listAntennas(const int direction, const size_t channel) const;

    void setAntenna(const int direction, const size_t channel, const std::string &name);

    std::string getAntenna(const int direction, const size_t channel) const;

    /*******************************************************************
     * Frontend corrections API
     ******************************************************************/

    bool hasDCOffsetMode(const int direction, const size_t channel) const;

    bool hasFrequencyCorrection(const int direction, const size_t channel) const;

    void setFrequencyCorrection(const int direction, const size_t channel, const double value);

    double getFrequencyCorrection(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/

    std::vector<std::string> listGains(const int direction, const size_t channel) const;

    bool hasGainMode(const int direction, const size_t channel) const;

    void setGainMode(const int direction, const size_t channel, const bool automatic);

    bool getGainMode(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const double value);

    void setGain(const int direction, const size_t channel, const std::string &name, const double value);

    double getGain(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/

    void setFrequency(
        const int direction,
        const size_t channel,
        const std::string &name,
        const double frequency,
        const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    double getFrequency(const int direction, const size_t channel, const std::string &name) const;

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;

    /*******************************************************************
     * Sample Rate API
     ******************************************************************/

    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    std::vector<double> listSampleRates(const int direction, const size_t channel) const;

    void setBandwidth(const int direction, const size_t channel, const double bw);

    double getBandwidth(const int direction, const size_t channel) const;

    std::vector<double> listBandwidths(const int direction, const size_t channel) const;

    /*******************************************************************
     * Time API
     ******************************************************************/

    std::vector<std::string> listTimeSources(void) const;

    std::string getTimeSource(void) const;

    bool hasHardwareTime(const std::string &what = "") const;

    long long getHardwareTime(const std::string &what = "") const;

    void setHardwareTime(const long long timeNs, const std::string &what = "");

    /*******************************************************************
     * Settings API
     ******************************************************************/

    SoapySDR::ArgInfoList getSettingInfo(void) const;

    void writeSetting(const std::string &key, const std::string &value);

    std::string readSetting(const std::string &key) const;

private:
    //device handle
    int deviceId;
    rtlsdr_dev_t *dev;

    //cached settings
    rtlsdrRXFormat rxFormat;
    rtlsdr_tuner tunerType;
    uint32_t sampleRate, centerFrequency;
    int ppm, directSamplingMode;
    size_t numBuffers, bufferLength, asyncBuffs;
    bool iqSwap, gainMode, offsetMode, digitalAGC, biasTee;
    double IFGain[6], tunerGain;
    std::atomic<long long> ticks;

    std::vector<std::complex<float>> _lut_32f;
    std::vector<std::complex<float>> _lut_swap_32f;
    std::vector<std::complex<int16_t>> _lut_16i;
    std::vector<std::complex<int16_t>> _lut_swap_16i;

public:
    struct Buffer
    {
        unsigned long long tick;
        std::vector<signed char> data;
    };

    //async api usage
    std::thread _rx_async_thread;
    void rx_async_operation(void);
    void rx_callback(unsigned char *buf, uint32_t len);

    std::mutex _buf_mutex;
    std::condition_variable _buf_cond;

    std::vector<Buffer> _buffs;
    size_t _buf_head;
    size_t _buf_tail;
    std::atomic<size_t> _buf_count;
    signed char *_currentBuff;
    std::atomic<bool> _overflowEvent;
    size_t _currentHandle;
    size_t bufferedElems;
    long long bufTicks;
    std::atomic<bool> resetBuffer;

    double gainMin, gainMax;
};