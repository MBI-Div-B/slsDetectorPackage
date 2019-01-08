#pragma once

#include "commonServerFunctions.h" // blackfin.h, ansi.h
#include "common.h"

#include "math.h"
#include <string.h>

/* LTC2620 DAC DEFINES */
// first 4 bits are 0 as this is a 12 bit dac
#define LTC2620_DAC_DATA_OFST               (4)
#define LTC2620_DAC_DATA_MSK                (0x00000FFF << LTC2620_DAC_DATA_OFST)
#define LTC2620_DAC_ADDR_OFST               (16)
#define LTC2620_DAC_ADDR_MSK                (0x0000000F << LTC2620_DAC_ADDR_OFST)
#define LTC2620_DAC_CMD_OFST                (20)
#define LTC2620_DAC_CMD_MSK                 (0x0000000F << LTC2620_DAC_CMD_OFST)

#define LTC2620_DAC_CMD_WR_IN_VAL           ((0x0 << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK) // write to input register
#define LTC2620_DAC_CMD_UPDTE_DAC_VAL       ((0x1 << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK) // update dac (power up)
#define LTC2620_DAC_CMD_WR_IN_UPDTE_DAC_VAL ((0x2 << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK) // write to input register and update dac (power up)
#define LTC2620_DAC_CMD_WR_UPDTE_DAC_VAL    ((0x3 << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK) // write to and update dac (power up)
#define LTC2620_DAC_CMD_PWR_DWN_VAL         ((0x4 << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK)
#define LTC2620_DAC_CMD_NO_OPRTN_VAL        ((0xF << LTC2620_DAC_CMD_OFST) & LTC2620_DAC_CMD_MSK)

#define LTC2620_NUMBITS                     (24)
#define LTC2620_DAISY_CHAIN_NUMBITS         (32) // due to shift register FIXME: was 33 earlier
#define LTC2620_NUMCHANNELS                 (8)
#define LTC2620_MIN_MV                      (0)
#define LTC2620_MAX_STEPS                   (pow(2,12)) // 4096
#define LTC2620_PWR_DOWN_VAL                (-100)

uint32_t LTC2620_Reg = 0x0;
uint32_t LTC2620_CsMask = 0x0;
uint32_t LTC2620_ClkMask = 0x0;
uint32_t LTC2620_DigMask = 0x0;
int LTC2620_DigOffset = 0x0;
int LTC2620_Ndac = 0;
int LTC2620_MaxMV = 0;

/**
 * Set Defines
 * @param reg spi register
 * @param cmsk chip select mask
 * @param clkmsk clock output mask
 * @param dmsk digital output mask
 * @param dofst digital output offset
 * @param nd total number of dacs for this board (for dac channel and daisy chain chip id)
 * @param mv maximum voltage in mV
 */
void LTC2620_SetDefines(uint32_t reg, uint32_t cmsk, uint32_t clkmsk, uint32_t dmsk, int dofst, int nd, int mv) {
    LTC2620_Reg = reg;
    LTC2620_CsMask = cmsk;
    LTC2620_ClkMask = clkmsk;
    LTC2620_DigMask = dmsk;
    LTC2620_DigOffset = dofst;
    LTC2620_Ndac = nd;
    LTC2620_MaxMV = mv;
}


/**
 * Disable SPI
 */
void LTC2620_Disable() {
    bus_w(LTC2620_Reg, (bus_r(LTC2620_Reg)
            | LTC2620_CsMask
            | LTC2620_ClkMask)
            & ~(LTC2620_DigMask));
}


/**
 * Convert voltage to dac units
 * @param voltage value in mv
 * @param dacval pointer to value converted to dac units
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int LTC2620_VoltageToDac(int voltage, int* dacval) {
    return Common_VoltageToDac(voltage, dacval, LTC2620_MIN_MV, LTC2620_MaxMV, LTC2620_MAX_STEPS);
}


/**
 * Convert dac units to voltage
 * @param dacval dac units
 * @param voltage pointer to value converted to mV
 * @returns FAIL when voltage outside limits, OK if conversion successful
 */
int LTC2620_DacToVoltage(int dacval, int* voltage) {
    return Common_DacToVoltage(dacval, voltage, LTC2620_MIN_MV, LTC2620_MaxMV, LTC2620_MAX_STEPS);
}


/**
 * Set a single chip (all non ctb detectors use this)
 * when max dac is 8
 * @param cmd command
 * @param data dac value to be set
 * @param dacaddr dac channel number in chip
 */
void LTC2620_SetSingle(int cmd, int data, int dacaddr)  {
    FILE_LOG(logDEBUG1, ("\tdac addr:%d, dac value:%d, cmd:%d\n", dacaddr, data, cmd));

    uint32_t codata = (((data << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK) |
            ((dacaddr << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) |
            cmd);

    serializeToSPI (LTC2620_Reg, codata, LTC2620_CsMask, LTC2620_NUMBITS,
            LTC2620_ClkMask, LTC2620_DigMask, LTC2620_DigOffset);
}


/**
 * bit bang the data into all the chips daisy fashion
 * @param valw current value of register while bit banging
 * @param val data to be sent (data, dac addr and command)
 */
void LTC2620_SendDaisyData(uint32_t* valw, uint32_t val) {
    sendDataToSPI(valw, LTC2620_Reg, val, LTC2620_DAISY_CHAIN_NUMBITS,
            LTC2620_ClkMask, LTC2620_DigMask, LTC2620_DigOffset);
}


/**
 * Set a single chip (all non ctb detectors use this)
 * when max dac is 8
 * @param cmd command
 * @param data dac value to be set
 * @param dacaddr dac channel number in chip
 * @param chipIndex index of the chip
 */
void LTC2620_SetDaisy(int cmd, int data, int dacaddr, int chipIndex)  {

    int nchip = LTC2620_Ndac / LTC2620_NUMCHANNELS;
    uint32_t valw = 0;
    int ichip = 0;

    FILE_LOG(logDEBUG1, ("\tdesired chip index:%d, nchip:%d, dac channel:%d, dac value:%d, cmd:%d \n",
            chipIndex, nchip, dacaddr, data, cmd));

    // data to be bit banged
    uint32_t codata = (((data << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK) |
            ((dacaddr << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) |
            cmd);

    // select all chips (ctb daisy chain; others 1 chip)
    FILE_LOG(logDEBUG1, ("\tSelecting LTC2620\n"));
    SPIChipSelect (&valw, LTC2620_Reg, LTC2620_CsMask, LTC2620_ClkMask, LTC2620_DigMask);

    // send same data to all
    if (chipIndex < 0) {
        FILE_LOG(logDEBUG1, ("\tSend same data to all\n"));
        for (ichip = 0; ichip < nchip; ++ichip) {
            FILE_LOG(logDEBUG1, ("\tSend to ichip %d\n", ichip));
            LTC2620_SendDaisyData(&valw, codata);
        }
    }

    // send to one chip, nothing to others
    else {
        // send nothing to preceding ichips (daisy chain) (if any chips in front of desired chip)
        for (ichip = 0; ichip < chipIndex; ++ichip) {
            FILE_LOG(logDEBUG1, ("\tSend nothing to ichip %d\n", ichip));
            LTC2620_SendDaisyData(&valw, LTC2620_DAC_CMD_NO_OPRTN_VAL);
        }

        // send data to desired chip
        FILE_LOG(logDEBUG1, ("\tSend data to ichip %d\n", chipIndex));
        LTC2620_SendDaisyData(&valw, codata);

        // send nothing to subsequent ichips (daisy chain) (if any chips after desired chip)
        int ichip = 0;
        for (ichip = chipIndex + 1; ichip < nchip; ++ichip) {
            FILE_LOG(logDEBUG1, ("\tSend nothing to ichip %d\n", ichip));
            LTC2620_SendDaisyData(&valw, LTC2620_DAC_CMD_NO_OPRTN_VAL);
        }
    }

    // deselect all chips (ctb daisy chain; others 1 chip)
    FILE_LOG(logDEBUG1, ("\tDeselecting LTC2620\n"));
    SPIChipDeselect(&valw, LTC2620_Reg, LTC2620_CsMask, LTC2620_ClkMask);
}


/**
 * Sets a single chip (LTC2620_SetSingle) or multiple chip (LTC2620_SetDaisy)
 * multiple chip is only for ctb where the multiple chips are connected in daisy fashion
 * @param cmd command to send
 * @param data dac value to be set
 * @param dacaddr dac channel number for the chip
 * @param chipIndex the chip to be set
 */
void LTC2620_Set(int cmd, int data, int dacaddr, int chipIndex)  {
    FILE_LOG(logDEBUG1, ("\tcmd:%d data:%d dacaddr:%d chipIndex:%d\n", cmd, data, dacaddr, chipIndex));
    // ctb
    if (LTC2620_Ndac > LTC2620_NUMCHANNELS)
        LTC2620_SetDaisy(cmd, data, dacaddr, chipIndex);
    // others
    else
        LTC2620_SetSingle(cmd, data, dacaddr);
}


/**
 * Configure (obtains dacaddr, command and ichip and calls LTC2620_Set)
 */
void LTC2620_Configure(){
    FILE_LOG(logINFOBLUE, ("Configuring LTC2620\n"));

    // dac channel - all channels
    int addr = LTC2620_DAC_ADDR_MSK;

    // data (any random low value, just writing to power up)
    int data = 0x6;

    // command
    int cmd = LTC2620_DAC_CMD_WR_IN_VAL; //FIXME: should be command update and not write(does not power up)
    // also why do we need to power up (for jctb, we power down next)

    LTC2620_Set(data, addr, cmd, -1);
}


/**
 * Set Dac (obtains dacaddr, command and ichip and calls LTC2620_Set)
 * @param dacnum dac number
 * @param data dac value to set
 */
void LTC2620_SetDAC (int dacnum, int data) {
    FILE_LOG(logDEBUG1, ("\tSetting dac %d to %d\n", dacnum, data));
    // LTC2620 index
    int ichip =  dacnum / LTC2620_NUMCHANNELS;

    // dac channel
    int addr = dacnum % LTC2620_NUMCHANNELS;

    // command
    int cmd = LTC2620_DAC_CMD_WR_UPDTE_DAC_VAL;

    // power down mode, value is ignored
    if (data == LTC2620_PWR_DOWN_VAL) {
        cmd = LTC2620_DAC_CMD_PWR_DWN_VAL;
        FILE_LOG(logDEBUG1, ("\tPOWER DOWN\n"));
    } else {
        FILE_LOG(logDEBUG1,("\tWrite to Input Register and Update\n"));
    }

    LTC2620_Set(cmd, data, addr, ichip);
}

/**
 * Set dac in dac units or mV
 * @param dacnum dac index
 * @param val value in dac units or mV
 * @param mV 0 for dac units and 1 for mV unit
 * @param dacval pointer to value in dac units
 * @returns OK or FAIL for success of operation
 */
int LTC2620_SetDACValue (int dacnum, int val, int mV, int* dacval) {
    FILE_LOG(logDEBUG1, ("\tdacnum:%d, val:%d, mV:%d\n", dacnum, val, mV));
    // validate index
    if (dacnum < 0 || dacnum >= LTC2620_Ndac) {
        FILE_LOG(logERROR, ("Dac index %d is out of bounds (0 to %d)\n", dacnum, LTC2620_Ndac - 1));
        return FAIL;
    }

    // get
    if (val < 0 && val != LTC2620_PWR_DOWN_VAL)
        return FAIL;

    // convert to dac or get mV value
    *dacval = val;
    int dacmV = val;
    int ret = OK;
    if (mV) {
        ret = LTC2620_VoltageToDac(val, dacval);
    } else if (val >= 0) { // do not convert power down dac val
        ret = LTC2620_DacToVoltage(val, &dacmV);
    }

    // conversion out of bounds
    if (ret == FAIL) {
        FILE_LOG(logERROR, ("Setting Dac %d %s is out of bounds\n", dacnum, (mV ? "mV" : "dac units")));
        return FAIL;
    }

    // set
    if ( (*dacval >= 0) || (*dacval == LTC2620_PWR_DOWN_VAL)) {
        FILE_LOG(logINFO, ("Setting DAC %d: %d dac (%d mV)\n",dacnum, *dacval, dacmV));
        LTC2620_SetDAC(dacnum, *dacval);
    }
    return OK;
}