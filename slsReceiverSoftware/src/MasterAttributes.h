// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "receiver_defs.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#ifdef HDF5C
#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif
#endif

#include <chrono>
using ns = std::chrono::nanoseconds;

struct MasterAttributes {
    // (before acquisition)
    slsDetectorDefs::detectorType detType{slsDetectorDefs::GENERIC};
    slsDetectorDefs::timingMode timingMode{slsDetectorDefs::AUTO_TIMING};
    uint32_t imageSize{0};
    slsDetectorDefs::xy nPixels{};
    uint32_t maxFramesPerFile{0};
    slsDetectorDefs::frameDiscardPolicy frameDiscardMode{
        slsDetectorDefs::NO_DISCARD};
    int framePadding{1};
    slsDetectorDefs::scanParameters scanParams{};
    uint64_t totalFrames{0};
    ns exptime{0};
    ns period{0};
    slsDetectorDefs::burstMode burstMode{slsDetectorDefs::BURST_INTERNAL};
    int numUDPInterfaces{0};
    uint32_t dynamicRange{0};
    uint32_t tenGiga{0};
    int thresholdEnergyeV{0};
    std::array<int, 3> thresholdAllEnergyeV = {{0, 0, 0}};
    ns subExptime{0};
    ns subPeriod{0};
    uint32_t quad{0};
    uint32_t readNRows;
    std::vector<int64_t> ratecorr;
    uint32_t adcmask{0};
    uint32_t analog{0};
    uint32_t analogSamples{0};
    uint32_t digital{0};
    uint32_t digitalSamples{0};
    uint32_t dbitoffset{0};
    uint64_t dbitlist{0};
    slsDetectorDefs::ROI roi{};
    uint32_t counterMask{0};
    ns exptime1{0};
    ns exptime2{0};
    ns exptime3{0};
    ns gateDelay1{0};
    ns gateDelay2{0};
    ns gateDelay3{0};
    uint32_t gates;
    std::map<std::string, std::string> additionalJsonHeader;

    // Final Attributes (after acquisition)
    uint64_t framesInFile{0};

    MasterAttributes(){};
    virtual ~MasterAttributes(){};

    virtual void WriteMasterBinaryAttributes(FILE *fd) {
        LOG(logERROR) << "WriteMasterBinaryAttributes should have been called "
                         "by a child class";
    }

    std::string GetBinaryMasterAttributes() {
        time_t t = time(nullptr);
        std::ostringstream oss;
        oss << "Version                    : " << std::setprecision(2)
            << BINARY_WRITER_VERSION << '\n'
            << "TimeStamp                  : " << ctime(&t) << '\n'
            << "Detector Type              : " << sls::ToString(detType) << '\n'
            << "Timing Mode                : " << sls::ToString(timingMode)
            << '\n'
            << "Image Size                 : " << imageSize << " bytes" << '\n'
            << "Pixels                     : " << sls::ToString(nPixels) << '\n'
            << "Max Frames Per File        : " << maxFramesPerFile << '\n'
            << "Frame Discard Policy       : "
            << sls::ToString(frameDiscardMode) << '\n'
            << "Frame Padding              : " << framePadding << '\n'
            << "Scan Parameters            : " << sls::ToString(scanParams)
            << '\n'
            << "Total Frames               : " << totalFrames << '\n';
        return oss.str();
    };

    void WriteBinaryAttributes(FILE *fd, std::string message) {
        if (fwrite((void *)message.c_str(), 1, message.length(), fd) !=
            message.length()) {
            throw sls::RuntimeError(
                "Master binary file incorrect number of bytes written to file");
        }
    };

    void WriteFinalBinaryAttributes(FILE *fd) {
        // adding few common parameters to the end
        std::ostringstream oss;

        if (!additionalJsonHeader.empty()) {
            oss << "Additional Json Header     : "
                << sls::ToString(additionalJsonHeader) << '\n';
        }
        oss << "Frames in File             : " << framesInFile << '\n';

        // adding sls_receiver header format
        oss << '\n'
            << "#Frame Header" << '\n'
            << "Frame Number               : 8 bytes" << '\n'
            << "SubFrame Number/ExpLength  : 4 bytes" << '\n'
            << "Packet Number              : 4 bytes" << '\n'
            << "Bunch ID                   : 8 bytes" << '\n'
            << "Timestamp                  : 8 bytes" << '\n'
            << "Module Id                  : 2 bytes" << '\n'
            << "Row                        : 2 bytes" << '\n'
            << "Column                     : 2 bytes" << '\n'
            << "Reserved                   : 2 bytes" << '\n'
            << "Debug                      : 4 bytes" << '\n'
            << "Round Robin Number         : 2 bytes" << '\n'
            << "Detector Type              : 1 byte" << '\n'
            << "Header Version             : 1 byte" << '\n'
            << "Packets Caught Mask        : 64 bytes" << '\n';

        std::string message = oss.str();

        // writing to file
        if (fwrite((void *)message.c_str(), 1, message.length(), fd) !=
            message.length()) {
            throw sls::RuntimeError(
                "Master binary file incorrect number of bytes written to file");
        }
    };

#ifdef HDF5C
    virtual void WriteMasterHDF5Attributes(H5File *fd, Group *group) {
        LOG(logERROR) << "WriteMasterHdf5Attributes should have been called "
                         "by a child class";
    };

    void WriteHDF5Attributes(H5File *fd, Group *group) {
        char c[1024];
        memset(c, 0, sizeof(c));
        // clang-format off
        // version
        {
            double version = BINARY_WRITER_VERSION;
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            Attribute attribute = fd->createAttribute(
                "Version", PredType::NATIVE_DOUBLE, dataspace);
            attribute.write(PredType::NATIVE_DOUBLE, &version);
        }
        // timestamp
        {
            time_t t = time(nullptr);
            StrType strdatatype(PredType::C_S1, 256);
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
            group->createDataSet("Timestamp", strdatatype, dataspace);
            sls::strcpy_safe(c, std::string(ctime(&t)));
            dataset.write(c, strdatatype);
        }
        // detector type
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Detector Type", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(detType));
            dataset.write(c, strdatatype);
        }
        // timing mode
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Timing Mode", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(timingMode));
            dataset.write(c, strdatatype);
        }
        // Image Size
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Image Size", PredType::NATIVE_INT, dataspace);
            dataset.write(&imageSize, PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
            dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
            sls::strcpy_safe(c, "bytes");
            attribute.write(strdatatype, c);
        }
        //TODO: make this into an array?
        // x
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of pixels in x axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&nPixels.x, PredType::NATIVE_INT);
        }
        // y
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of pixels in y axis", PredType::NATIVE_INT, dataspace);
            dataset.write(&nPixels.y, PredType::NATIVE_INT);
        }
        // Maximum frames per file
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Maximum frames per file", PredType::NATIVE_INT, dataspace);
            dataset.write(&maxFramesPerFile, PredType::NATIVE_INT);
        }
        // Frame Discard Policy
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Frame Discard Policy", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(frameDiscardMode));
            dataset.write(c, strdatatype);
        }        
        // Frame Padding
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Frame Padding", PredType::NATIVE_INT, dataspace);
            dataset.write(&framePadding, PredType::NATIVE_INT);
        }
        // Scan Parameters
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
            group->createDataSet("Scan Parameters", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(scanParams));
            dataset.write(c, strdatatype);
        }   
        // Total Frames
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Total Frames", PredType::STD_U64LE, dataspace);
            dataset.write(&totalFrames, PredType::STD_U64LE);
        }
    };

    void WriteFinalHDF5Attributes(H5File *fd, Group *group) {
        char c[1024];
        memset(c, 0, sizeof(c));
        // Total Frames in file
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Frames in File", PredType::STD_U64LE, dataspace);
            dataset.write(&framesInFile, PredType::STD_U64LE);
        }
        // additional json header
        if (!additionalJsonHeader.empty()) {
            std::string json = sls::ToString(additionalJsonHeader);
            StrType strdatatype(PredType::C_S1, json.length());
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
            group->createDataSet("Additional JSON Header", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(additionalJsonHeader));
            dataset.write(c, strdatatype);
        }
    };

    void WriteHDF5Exptime(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
        group->createDataSet("Exposure Time", strdatatype, dataspace);
        char c[1024];
        memset(c, 0, sizeof(c));
        sls::strcpy_safe(c, sls::ToString(exptime));
        dataset.write(c, strdatatype);
    };

    void WriteHDF5Period(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        DataSet dataset =
        group->createDataSet("Acquisition Period", strdatatype, dataspace);
        char c[1024];
        memset(c, 0, sizeof(c));
        sls::strcpy_safe(c, sls::ToString(period));
        dataset.write(c, strdatatype);
    };

    void WriteHDF5DynamicRange(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet(
            "Dynamic Range", PredType::NATIVE_INT, dataspace);
        dataset.write(&dynamicRange, PredType::NATIVE_INT);
        DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
        StrType strdatatype(PredType::C_S1, 256);
        Attribute attribute =
        dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
        char c[1024] = "bits";
        attribute.write( strdatatype, c);
    };

    void WriteHDF5TenGiga(H5File *fd, Group *group) {
        DataSpace dataspace = DataSpace(H5S_SCALAR);
        DataSet dataset = group->createDataSet(
            "Ten Giga Enable", PredType::NATIVE_INT, dataspace);
        dataset.write(&tenGiga, PredType::NATIVE_INT);
    };
#endif
};
// clang-format on

class GotthardMasterAttributes : public MasterAttributes {
  public:
    GotthardMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Roi (xmin, xmax)           : " << sls::ToString(roi) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        // Roi xmin
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "roi xmin", PredType::NATIVE_INT, dataspace);
            dataset.write(&roi.xmin, PredType::NATIVE_INT);
        }
        // Roi xmax
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "roi xmax", PredType::NATIVE_INT, dataspace);
            dataset.write(&roi.xmax, PredType::NATIVE_INT);
        }
    };
#endif
};

class JungfrauMasterAttributes : public MasterAttributes {
  public:
    JungfrauMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Number of UDP Interfaces   : " << numUDPInterfaces << '\n'
            << "Number of rows             : " << readNRows << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of UDP Interfaces", PredType::NATIVE_INT, dataspace);
            dataset.write(&numUDPInterfaces, PredType::NATIVE_INT);
        }
        // readNRows
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of rows", PredType::NATIVE_INT, dataspace);
            dataset.write(&readNRows, PredType::NATIVE_INT);
        }
    };
#endif
};

class EigerMasterAttributes : public MasterAttributes {
  public:
    EigerMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Dynamic Range              : " << dynamicRange << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Threshold Energy           : " << thresholdEnergyeV << '\n'
            << "SubExptime                 : " << sls::ToString(subExptime)
            << '\n'
            << "SubPeriod                  : " << sls::ToString(subPeriod)
            << '\n'
            << "Quad                       : " << quad << '\n'
            << "Number of rows             : " << readNRows << '\n'
            << "Rate Corrections           : " << sls::ToString(ratecorr)
            << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5DynamicRange(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        char c[1024];
        memset(c, 0, sizeof(c));
        // threshold
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Threshold Energy", PredType::NATIVE_INT, dataspace);
            dataset.write(&thresholdEnergyeV, PredType::NATIVE_INT);
            DataSpace dataspaceAttr = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            Attribute attribute =
                dataset.createAttribute("Unit", strdatatype, dataspaceAttr);
            sls::strcpy_safe(c, "eV");
            attribute.write(strdatatype, c);
        }
        // SubExptime
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset = group->createDataSet("Sub Exposure Time",
                                                   strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(subExptime));
            dataset.write(c, strdatatype);
        }
        // SubPeriod
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Sub Period", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(subPeriod));
            dataset.write(c, strdatatype);
        }
        // Quad
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("Quad", PredType::NATIVE_INT, dataspace);
            dataset.write(&quad, PredType::NATIVE_INT);
        }
        // readNRows
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Number of rows", PredType::NATIVE_INT, dataspace);
            dataset.write(&readNRows, PredType::NATIVE_INT);
        }
        // Rate corrections
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 1024);
            DataSet dataset = group->createDataSet("Rate Corrections",
                                                   strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(ratecorr));
            dataset.write(c, strdatatype);
        }
    };
#endif
};

class Mythen3MasterAttributes : public MasterAttributes {
  public:
    Mythen3MasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Dynamic Range              : " << dynamicRange << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Counter Mask               : " << sls::ToStringHex(counterMask)
            << '\n'
            << "Exptime1                   : " << sls::ToString(exptime1)
            << '\n'
            << "Exptime2                   : " << sls::ToString(exptime2)
            << '\n'
            << "Exptime3                   : " << sls::ToString(exptime3)
            << '\n'
            << "GateDelay1                 : " << sls::ToString(gateDelay1)
            << '\n'
            << "GateDelay2                 : " << sls::ToString(gateDelay2)
            << '\n'
            << "GateDelay3                 : " << sls::ToString(gateDelay3)
            << '\n'
            << "Gates                      : " << gates << '\n'
            << "Threshold Energies         : "
            << sls::ToString(thresholdAllEnergyeV) << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5DynamicRange(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        char c[1024];
        memset(c, 0, sizeof(c));
        // Counter Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Counter Mask", PredType::STD_U32LE, dataspace);
            dataset.write(&counterMask, PredType::STD_U32LE);
        }
        // Exptime1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Exposure Time1", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(exptime1));
            dataset.write(c, strdatatype);
        }
        // Exptime2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Exposure Time2", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(exptime2));
            dataset.write(c, strdatatype);
        }
        // Exptime3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Exposure Time3", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(exptime3));
            dataset.write(c, strdatatype);
        }
        // GateDelay1
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Gate Delay1", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(gateDelay1));
            dataset.write(c, strdatatype);
        }
        // GateDelay2
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Gate Delay2", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(gateDelay2));
            dataset.write(c, strdatatype);
        }
        // GateDelay3
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Gate Delay3", strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(gateDelay3));
            dataset.write(c, strdatatype);
        }
        // Gates
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset =
                group->createDataSet("Gates", PredType::STD_U32LE, dataspace);
            dataset.write(&gates, PredType::STD_U32LE);
        }
        // Threshold Energies
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 1024);
            DataSet dataset = group->createDataSet("Threshold Energies",
                                                   strdatatype, dataspace);
            sls::strcpy_safe(c, sls::ToString(thresholdAllEnergyeV));
            dataset.write(c, strdatatype);
        }
    };
#endif
};

class Gotthard2MasterAttributes : public MasterAttributes {
  public:
    Gotthard2MasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Burst Mode                 : " << sls::ToString(burstMode)
            << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        // burst mode
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            StrType strdatatype(PredType::C_S1, 256);
            DataSet dataset =
                group->createDataSet("Burst Mode", strdatatype, dataspace);
            char c[1024];
            memset(c, 0, sizeof(c));
            sls::strcpy_safe(c, sls::ToString(burstMode));
            dataset.write(c, strdatatype);
        }
    };
#endif
};

class MoenchMasterAttributes : public MasterAttributes {
  public:
    MoenchMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "ADC Mask                   : " << sls::ToStringHex(adcmask)
            << '\n'
            << "Analog Samples             : " << analogSamples << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "ADC Mask", PredType::NATIVE_INT, dataspace);
            dataset.write(&adcmask, PredType::NATIVE_INT);
        }
        // Analog Samples
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Analog Samples", PredType::NATIVE_INT, dataspace);
            dataset.write(&analogSamples, PredType::NATIVE_INT);
        }
    };
#endif
};

class CtbMasterAttributes : public MasterAttributes {
  public:
    CtbMasterAttributes(){};

    void WriteMasterBinaryAttributes(FILE *fd) override {
        std::ostringstream oss;
        oss << MasterAttributes::GetBinaryMasterAttributes()
            << "Exptime                    : " << sls::ToString(exptime) << '\n'
            << "Period                     : " << sls::ToString(period) << '\n'
            << "Ten Giga                   : " << tenGiga << '\n'
            << "ADC Mask                   : " << sls::ToStringHex(adcmask)
            << '\n'
            << "Analog Flag                : " << analog << '\n'
            << "Analog Samples             : " << analogSamples << '\n'
            << "Digital Flag               : " << digital << '\n'
            << "Digital Samples            : " << digitalSamples << '\n'
            << "Dbit Offset                : " << dbitoffset << '\n'
            << "Dbit Bitset                : " << dbitlist << '\n';
        std::string message = oss.str();
        MasterAttributes::WriteBinaryAttributes(fd, message);
    };

#ifdef HDF5C
    void WriteMasterHDF5Attributes(H5File *fd, Group *group) override {
        MasterAttributes::WriteHDF5Attributes(fd, group);
        MasterAttributes::WriteHDF5Exptime(fd, group);
        MasterAttributes::WriteHDF5Period(fd, group);
        MasterAttributes::WriteHDF5TenGiga(fd, group);
        // ADC Mask
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "ADC mMsk", PredType::NATIVE_INT, dataspace);
            dataset.write(&adcmask, PredType::NATIVE_INT);
        }
        // Analog Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Analog Flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&analog, PredType::NATIVE_INT);
        }
        // Analog Samples
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Analog Samples", PredType::NATIVE_INT, dataspace);
            dataset.write(&analogSamples, PredType::NATIVE_INT);
        }
        // Digital Flag
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Digital Flag", PredType::NATIVE_INT, dataspace);
            dataset.write(&digital, PredType::NATIVE_INT);
        }
        // Digital Samples
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Digital Samples", PredType::NATIVE_INT, dataspace);
            dataset.write(&digitalSamples, PredType::NATIVE_INT);
        }
        // Dbit Offset
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Dbit Offset", PredType::NATIVE_INT, dataspace);
            dataset.write(&dbitoffset, PredType::NATIVE_INT);
        }
        // Dbit List
        {
            DataSpace dataspace = DataSpace(H5S_SCALAR);
            DataSet dataset = group->createDataSet(
                "Dbit Bitset List", PredType::STD_U64LE, dataspace);
            dataset.write(&dbitlist, PredType::STD_U64LE);
        }
    };
#endif
};
