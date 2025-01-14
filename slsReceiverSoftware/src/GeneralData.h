// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file GeneralData.h
 * @short abstract for setting/getting properties of detector data
 ***********************************************/
/**
 *@short abstract for setting/getting properties of detector data
 */

#include "receiver_defs.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include <cmath> //ceil
#include <vector>

class GeneralData {

  public:
    slsDetectorDefs::detectorType myDetectorType{slsDetectorDefs::GENERIC};
    uint32_t nPixelsX{0};
    uint32_t nPixelsY{0};
    uint32_t headerSizeinPacket{0};
    /** Size of just data in 1 packet (in bytes) */
    uint32_t dataSize{0};
    uint32_t packetSize{0};
    /** Number of packets in an image (for each listening UDP port) */
    uint32_t packetsPerFrame{0};
    /** Image size (in bytes, for each listening UDP port) */
    uint32_t imageSize{0};
    uint64_t frameIndexMask{0};
    uint32_t frameIndexOffset{0};
    uint32_t packetIndexMask{0};
    uint32_t packetIndexOffset{0};
    uint32_t maxFramesPerFile{0};
    /** Header size of data saved into fifo buffer at a time*/
    uint32_t fifoBufferHeaderSize{0};
    uint32_t defaultFifoDepth{0};
    uint32_t numUDPInterfaces{1};
    uint32_t headerPacketSize{0};
    /** Streaming (for ROI - mainly short Gotthard)  */
    uint32_t nPixelsXComplete{0};
    /** Streaming (for ROI - mainly short Gotthard)  */
    uint32_t nPixelsYComplete{0};
    /** Streaming (for ROI - mainly short Gotthard) - Image size (in bytes) */
    uint32_t imageSizeComplete{0};
    /** if standard header implemented in firmware */
    bool standardheader{false};
    uint32_t defaultUdpSocketBufferSize{RECEIVE_SOCKET_BUFFER_SIZE};
    uint32_t vetoDataSize{0};
    uint32_t vetoPacketSize{0};
    uint32_t vetoImageSize{0};
    uint32_t vetoHsize{0};
    uint32_t maxRowsPerReadout{0};
    uint32_t dynamicRange{16};
    bool tengigaEnable{false};
    uint32_t nAnalogSamples{0};
    uint32_t nDigitalSamples{0};
    slsDetectorDefs::readoutMode readoutType{slsDetectorDefs::ANALOG_ONLY};
    uint32_t adcEnableMaskOneGiga{BIT32_MASK};
    uint32_t adcEnableMaskTenGiga{BIT32_MASK};
    slsDetectorDefs::ROI roi{};

    GeneralData(){};
    virtual ~GeneralData(){};

    // Returns the pixel depth in byte, 4 bits being 0.5 byte
    float GetPixelDepth() { return float(dynamicRange) / 8; }

    void ThrowGenericError(std::string msg) const {
        throw sls::RuntimeError(
            msg + std::string("SetROI is a generic function that should be "
                              "overloaded by a derived class"));
    }

    /**
     * Get Header Infomation (frame number, packet number)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @param oddStartingPacket odd starting packet (gotthard)
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    virtual void GetHeaderInfo(int index, char *packetData,
                               bool oddStartingPacket, uint64_t &frameNumber,
                               uint32_t &packetNumber,
                               uint64_t &bunchId) const {
        frameNumber = ((uint32_t)(*((uint32_t *)(packetData))));
        frameNumber++;
        packetNumber = frameNumber & packetIndexMask;
        frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
        bunchId = -1;
    }

    virtual void SetROI(slsDetectorDefs::ROI i) {
        ThrowGenericError("SetROI");
    };

    /**@returns adc configured */
    virtual int GetAdcConfigured(int index, slsDetectorDefs::ROI i) const {
        ThrowGenericError("GetAdcConfigured");
        return 0;
    };

    virtual void SetDynamicRange(int dr) {
        ThrowGenericError("SetDynamicRange");
    };

    virtual void SetTenGigaEnable(bool tgEnable) {
        ThrowGenericError("SetTenGigaEnable");
    };

    virtual bool SetOddStartingPacket(int index, char *packetData) {
        ThrowGenericError("SetOddStartingPacket");
        return false;
    };

    virtual void SetNumberofInterfaces(const int n) {
        ThrowGenericError("SetNumberofInterfaces");
    };

    virtual void SetNumberofCounters(const int n) {
        ThrowGenericError("SetNumberofCounters");
    };

    virtual int GetNumberOfAnalogDatabytes() {
        ThrowGenericError("GetNumberOfAnalogDatabytes");
        return 0;
    };

    virtual void SetNumberOfAnalogSamples(int n) {
        ThrowGenericError("SetNumberOfAnalogSamples");
    };

    virtual void SetNumberOfDigitalSamples(int n) {
        ThrowGenericError("SetNumberOfDigitalSamples");
    };

    virtual void SetOneGigaAdcEnableMask(int n) {
        ThrowGenericError("SetOneGigaAdcEnableMask");
    };

    virtual void SetTenGigaAdcEnableMask(int n) {
        ThrowGenericError("SetTenGigaAdcEnableMask");
    };

    virtual void SetReadoutMode(slsDetectorDefs::readoutMode r) {
        ThrowGenericError("SetReadoutMode");
    };
};

class GotthardData : public GeneralData {

  private:
    const int nChan = 128;
    const int nChipsPerAdc = 2;

  public:
    GotthardData() {
        myDetectorType = slsDetectorDefs::GOTTHARD;
        nPixelsY = 1;
        headerSizeinPacket = 6;
        maxFramesPerFile = MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        UpdateImageSize();
    };

    /**
     * Get Header Infomation (frame number, packet number)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @param oddStartingPacket odd starting packet (gotthard)
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    void GetHeaderInfo(int index, char *packetData, bool oddStartingPacket,
                       uint64_t &frameNumber, uint32_t &packetNumber,
                       uint64_t &bunchId) const {
        if (nPixelsX == 1280) {
            frameNumber = *reinterpret_cast<uint32_t *>(packetData);
            if (oddStartingPacket)
                frameNumber++;
            packetNumber = frameNumber & packetIndexMask;
            frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
        } else {
            frameNumber = *reinterpret_cast<uint32_t *>(packetData);
            packetNumber = 0;
        }
        bunchId = -1;
    }

    /** @returns adc configured */
    int GetAdcConfigured(int index, slsDetectorDefs::ROI i) const {
        int adc = -1;
        // single adc
        if (i.xmin != -1) {
            // gotthard can have only one adc per detector enabled (or all)
            // adc = mid value/numchans also for only 1 roi
            adc = ((((i.xmax) + (i.xmin)) / 2) / (nChan * nChipsPerAdc));
            if ((adc < 0) || (adc > 4)) {
                LOG(logWARNING) << index
                                << ": Deleting ROI. "
                                   "Adc value should be between 0 and 4";
                adc = -1;
            }
        }
        LOG(logINFO) << "Adc Configured: " << adc;
        return adc;
    };

    /**
     * Set odd starting packet (gotthard)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @returns true or false for odd starting packet number
     */
    bool SetOddStartingPacket(int index, char *packetData) {
        bool oddStartingPacket = true;
        // care only if no roi
        if (nPixelsX == 1280) {
            uint32_t fnum = ((uint32_t)(*((uint32_t *)(packetData))));
            uint32_t firstData = ((uint32_t)(*((uint32_t *)(packetData + 4))));
            // first packet
            if (firstData == 0xCACACACA) {
                // packet number should be 0, but is 1 => so odd starting packet
                if (fnum & packetIndexMask) {
                    oddStartingPacket = true;
                } else {
                    oddStartingPacket = false;
                }
            }
            // second packet
            else {
                // packet number should be 1, but is 0 => so odd starting packet
                if (!(fnum & packetIndexMask)) {
                    oddStartingPacket = true;
                } else {
                    oddStartingPacket = false;
                }
            }
        }
        return oddStartingPacket;
    };

    void SetROI(slsDetectorDefs::ROI i) {
        roi = i;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {

        // all adcs
        if (roi.xmin == -1) {
            nPixelsX = 1280;
            dataSize = 1280;
            packetsPerFrame = 2;
            frameIndexMask = 0xFFFFFFFE;
            frameIndexOffset = 1;
            packetIndexMask = 1;
            maxFramesPerFile = MAX_FRAMES_PER_FILE;
            nPixelsXComplete = 0;
            nPixelsYComplete = 0;
            imageSizeComplete = 0;
            defaultFifoDepth = 50000;
        } else {
            nPixelsX = 256;
            dataSize = 512;
            packetsPerFrame = 1;
            frameIndexMask = 0xFFFFFFFF;
            frameIndexOffset = 0;
            packetIndexMask = 0;
            maxFramesPerFile = SHORT_MAX_FRAMES_PER_FILE;
            nPixelsXComplete = 1280;
            nPixelsYComplete = 1;
            imageSizeComplete = 1280 * 2;
            defaultFifoDepth = 75000;
        }
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetSize = headerSizeinPacket + dataSize;
        packetsPerFrame = imageSize / dataSize;
    };
};

class EigerData : public GeneralData {

  public:
    EigerData() {
        myDetectorType = slsDetectorDefs::EIGER;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        maxFramesPerFile = EIGER_MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        numUDPInterfaces = 2;
        headerPacketSize = 40;
        standardheader = true;
        maxRowsPerReadout = 256;
        UpdateImageSize();
    };

    void SetDynamicRange(int dr) {
        dynamicRange = dr;
        UpdateImageSize();
    }

    void SetTenGigaEnable(bool tgEnable) {
        tengigaEnable = tgEnable;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (256 * 4) / numUDPInterfaces;
        nPixelsY = 256;
        dataSize = (tengigaEnable ? 4096 : 1024);
        packetSize = headerSizeinPacket + dataSize;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        defaultFifoDepth = (dynamicRange == 32 ? 100 : 1000);
    };
};

class JungfrauData : public GeneralData {

  public:
    JungfrauData() {
        myDetectorType = slsDetectorDefs::JUNGFRAU;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        dataSize = 8192;
        packetSize = headerSizeinPacket + dataSize;
        maxFramesPerFile = JFRAU_MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        defaultFifoDepth = 2500;
        standardheader = true;
        maxRowsPerReadout = 512;
        UpdateImageSize();
    };

    void SetNumberofInterfaces(const int n) {
        numUDPInterfaces = n;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (256 * 4);
        nPixelsY = (256 * 2) / numUDPInterfaces;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        defaultUdpSocketBufferSize = (1000 * 1024 * 1024) / numUDPInterfaces;
    };
};

class Mythen3Data : public GeneralData {
  private:
    int ncounters;
    const int NCHAN = 1280;

  public:
    Mythen3Data() {
        myDetectorType = slsDetectorDefs::MYTHEN3;
        ncounters = 3;
        nPixelsY = 1;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        maxFramesPerFile = MYTHEN3_MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        defaultFifoDepth = 50000;
        standardheader = true;
        defaultUdpSocketBufferSize = (1000 * 1024 * 1024);
        dynamicRange = 32;
        tengigaEnable = true;
        UpdateImageSize();
    };

    void SetDynamicRange(int dr) {
        dynamicRange = dr;
        UpdateImageSize();
    };

    void SetTenGigaEnable(bool tg) {
        tengigaEnable = tg;
        UpdateImageSize();
    };

    virtual void SetNumberofCounters(const int n) {
        ncounters = n;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (NCHAN * ncounters); // max 1280 channels x 3 counters
        LOG(logINFO) << "nPixelsX: " << nPixelsX;
        imageSize = nPixelsX * nPixelsY * GetPixelDepth();

        // 10g
        if (tengigaEnable) {
            if (dynamicRange == 32 && ncounters > 1) {
                packetsPerFrame = 2;
            } else {
                packetsPerFrame = 1;
            }
            dataSize = imageSize / packetsPerFrame;
        }
        // 1g
        else {
            if (ncounters == 3) {
                dataSize = 768;
            } else {
                dataSize = 1280;
            }
            packetsPerFrame = imageSize / dataSize;
        }

        LOG(logINFO) << "Packets Per Frame: " << packetsPerFrame;
        packetSize = headerSizeinPacket + dataSize;
        LOG(logINFO) << "PacketSize: " << packetSize;
    };
};

class Gotthard2Data : public GeneralData {
  public:
    Gotthard2Data() {
        myDetectorType = slsDetectorDefs::GOTTHARD2;
        nPixelsX = 128 * 10;
        nPixelsY = 1;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        dataSize = 2560; // 1280 channels * 2 bytes
        maxFramesPerFile = GOTTHARD2_MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        defaultFifoDepth = 50000;
        standardheader = true;
        vetoDataSize = 160;
        vetoHsize = 16;
        UpdateImageSize();
    };

    void SetNumberofInterfaces(const int n) {
        numUDPInterfaces = n;
        UpdateImageSize();
    };

    /**
     * Get Header Infomation (frame number, packet number) for veto packets
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @param oddStartingPacket odd starting packet (gotthard)
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    void GetHeaderInfo(int index, char *packetData, bool oddStartingPacket,
                       uint64_t &frameNumber, uint32_t &packetNumber,
                       uint64_t &bunchId) const {
        frameNumber = *reinterpret_cast<uint64_t *>(packetData);
        bunchId = *reinterpret_cast<uint64_t *>(packetData + 8);
        packetNumber = 0;
    };

  private:
    void UpdateImageSize() {
        packetSize = headerSizeinPacket + dataSize;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        vetoPacketSize = vetoHsize + vetoDataSize;
        vetoImageSize = vetoDataSize * packetsPerFrame;
        defaultUdpSocketBufferSize = (1000 * 1024 * 1024) / numUDPInterfaces;
    };
};

class ChipTestBoardData : public GeneralData {
  private:
    const int NCHAN_DIGITAL = 64;
    const int NUM_BYTES_PER_ANALOG_CHANNEL = 2;
    int nAnalogBytes = 0;

  public:
    /** Constructor */
    ChipTestBoardData() {
        myDetectorType = slsDetectorDefs::CHIPTESTBOARD;
        nPixelsY = 1; // number of samples
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        frameIndexMask = 0xFFFFFF; // 10g
        frameIndexOffset = 8;      // 10g
        packetIndexMask = 0xFF;    // 10g
        maxFramesPerFile = CTB_MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        defaultFifoDepth = 2500;
        standardheader = true;
        UpdateImageSize();
    };

  public:
    int GetNumberOfAnalogDatabytes() { return nAnalogBytes; };

    void SetNumberOfAnalogSamples(int n) {
        nAnalogSamples = n;
        UpdateImageSize();
    };

    void SetNumberOfDigitalSamples(int n) {
        nDigitalSamples = n;
        UpdateImageSize();
    };

    void SetOneGigaAdcEnableMask(int n) {
        adcEnableMaskOneGiga = n;
        UpdateImageSize();
    };

    void SetTenGigaAdcEnableMask(int n) {
        adcEnableMaskTenGiga = n;
        UpdateImageSize();
    };

    void SetReadoutMode(slsDetectorDefs::readoutMode r) {
        readoutType = r;
        UpdateImageSize();
    };

    void SetTenGigaEnable(bool tg) {
        tengigaEnable = tg;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nAnalogBytes = 0;
        int nDigitalBytes = 0;
        int nAnalogChans = 0, nDigitalChans = 0;

        // analog channels (normal, analog/digital readout)
        if (readoutType == slsDetectorDefs::ANALOG_ONLY ||
            readoutType == slsDetectorDefs::ANALOG_AND_DIGITAL) {
            uint32_t adcEnableMask =
                (tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga);
            nAnalogChans = __builtin_popcount(adcEnableMask);

            nAnalogBytes =
                nAnalogChans * NUM_BYTES_PER_ANALOG_CHANNEL * nAnalogSamples;
            LOG(logDEBUG1) << " Number of Analog Channels:" << nAnalogChans
                           << " Databytes: " << nAnalogBytes;
        }
        // digital channels
        if (readoutType == slsDetectorDefs::DIGITAL_ONLY ||
            readoutType == slsDetectorDefs::ANALOG_AND_DIGITAL) {
            nDigitalChans = NCHAN_DIGITAL;
            nDigitalBytes = (sizeof(uint64_t) * nDigitalSamples);
            LOG(logDEBUG1) << "Number of Digital Channels:" << nDigitalChans
                           << " Databytes: " << nDigitalBytes;
        }

        nPixelsX = nAnalogChans + nDigitalChans;
        dataSize = tengigaEnable ? 8144 : UDP_PACKET_DATA_BYTES;
        packetSize = headerSizeinPacket + dataSize;
        imageSize = nAnalogBytes + nDigitalBytes;
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);

        LOG(logDEBUG1) << "Total Number of Channels:" << nPixelsX
                       << " Databytes: " << imageSize;
    };
};

class MoenchData : public GeneralData {

  private:
    const int NUM_BYTES_PER_ANALOG_CHANNEL = 2;

  public:
    MoenchData() {
        myDetectorType = slsDetectorDefs::MOENCH;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        frameIndexMask = 0xFFFFFF;
        maxFramesPerFile = MOENCH_MAX_FRAMES_PER_FILE;
        fifoBufferHeaderSize =
            FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
        defaultFifoDepth = 2500;
        standardheader = true;
        UpdateImageSize();
    };

    void SetNumberOfAnalogSamples(int n) {
        nAnalogSamples = n;
        UpdateImageSize();
    };

    void SetOneGigaAdcEnableMask(int n) {
        adcEnableMaskOneGiga = n;
        UpdateImageSize();
    };

    void SetTenGigaAdcEnableMask(int n) {
        adcEnableMaskTenGiga = n;
        UpdateImageSize();
    };

    void SetTenGigaEnable(bool tg) {
        tengigaEnable = tg;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        uint32_t adcEnableMask =
            (tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga);

        // count number of channels in x, each adc has 25 channels each
        int nchanTop = __builtin_popcount(adcEnableMask & 0xF0F0F0F0) * 25;
        int nchanBot = __builtin_popcount(adcEnableMask & 0x0F0F0F0F) * 25;
        nPixelsX = nchanTop > 0 ? nchanTop : nchanBot;

        // if both top and bottom adcs enabled, rows = 2
        int nrows = 1;
        if (nchanTop > 0 && nchanBot > 0) {
            nrows = 2;
        }
        nPixelsY = nAnalogSamples / 25 * nrows;
        LOG(logINFO) << "Number of Pixels: [" << nPixelsX << ", " << nPixelsY
                     << "]";

        dataSize = tengigaEnable ? 8144 : UDP_PACKET_DATA_BYTES;
        packetSize = headerSizeinPacket + dataSize;
        imageSize = nPixelsX * nPixelsY * NUM_BYTES_PER_ANALOG_CHANNEL;
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);

        LOG(logDEBUG) << "Databytes: " << imageSize;
    };
};
