/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the diagnostics module.
 */
#define _GNU_SOURCE
#include <string.h>

#include "factory_diags.hpp"

#if OPENTHREAD_CONFIG_DIAG_ENABLE

#include <stdio.h>
#include <stdlib.h>

#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>

#include "common/as_core_type.hpp"
#include "common/code_utils.hpp"
#include "common/instance.hpp"
#include "common/locator_getters.hpp"
#include "radio/radio.hpp"
#include "utils/parse_cmdline.hpp"

OT_TOOL_WEAK
otError otPlatDiagProcess(otInstance *aInstance,
                          uint8_t     aArgsLength,
                          char       *aArgs[],
                          char       *aOutput,
                          size_t      aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aOutput);
    OT_UNUSED_VARIABLE(aOutputMaxLen);

    return ot::kErrorInvalidCommand;
}

namespace ot {
namespace FactoryDiags {

#if OPENTHREAD_RADIO && !OPENTHREAD_RADIO_CLI

const struct Diags::Command Diags::sCommands[] = {
    {"channel", &Diags::ProcessChannel},
    {"cw", &Diags::ProcessContinuousWave},
    {"echo", &Diags::ProcessEcho},
    {"gpio", &Diags::ProcessGpio},
    {"power", &Diags::ProcessPower},
    {"powersettings", &Diags::ProcessPowerSettings},
    {"rawpowersetting", &Diags::ProcessRawPowerSetting},
    {"start", &Diags::ProcessStart},
    {"stop", &Diags::ProcessStop},
    {"stream", &Diags::ProcessStream},
};

Diags::Diags(Instance &aInstance)
    : InstanceLocator(aInstance)
{
}

Error Diags::ProcessChannel(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;
    long  value;

    VerifyOrExit(aArgsLength == 1, error = kErrorInvalidArgs);

    SuccessOrExit(error = ParseLong(aArgs[0], value));
    VerifyOrExit(value >= Radio::kChannelMin && value <= Radio::kChannelMax, error = kErrorInvalidArgs);

    otPlatDiagChannelSet(static_cast<uint8_t>(value));

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessPower(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;
    long  value;

    VerifyOrExit(aArgsLength == 1, error = kErrorInvalidArgs);

    SuccessOrExit(error = ParseLong(aArgs[0], value));

    otPlatDiagTxPowerSet(static_cast<int8_t>(value));

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessEcho(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    if (aArgsLength == 1)
    {
        snprintf(aOutput, aOutputMaxLen, "%s\r\n", aArgs[0]);
    }
    else if ((aArgsLength == 2) && (strcmp(aArgs[0], "-n") == 0))
    {
        const uint8_t kReservedLen = 3; // 1 byte '\r', 1 byte '\n' and 1 byte '\0'
        uint32_t      outputMaxLen = static_cast<uint32_t>(aOutputMaxLen) - kReservedLen;
        long          value;
        uint32_t      i;
        uint32_t      number;

        SuccessOrExit(error = ParseLong(aArgs[1], value));
        number = Min(static_cast<uint32_t>(value), outputMaxLen);

        for (i = 0; i < number; i++)
        {
            aOutput[i] = '0' + i % 10;
        }

        snprintf(&aOutput[i], aOutputMaxLen - i, "\r\n");
    }
    else
    {
        error = kErrorInvalidArgs;
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessStart(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    OT_UNUSED_VARIABLE(aOutput);
    OT_UNUSED_VARIABLE(aOutputMaxLen);

    otPlatDiagModeSet(true);

    return kErrorNone;
}

Error Diags::ProcessStop(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    OT_UNUSED_VARIABLE(aOutput);
    OT_UNUSED_VARIABLE(aOutputMaxLen);

    otPlatDiagModeSet(false);

    return kErrorNone;
}

extern "C" void otPlatDiagAlarmFired(otInstance *aInstance) { otPlatDiagAlarmCallback(aInstance); }

#else // OPENTHREAD_RADIO && !OPENTHREAD_RADIO_CLI
// For OPENTHREAD_FTD, OPENTHREAD_MTD, OPENTHREAD_RADIO_CLI
const struct Diags::Command Diags::sCommands[] = {
    {"channel", &Diags::ProcessChannel},
    {"cw", &Diags::ProcessContinuousWave},
    {"gpio", &Diags::ProcessGpio},
    {"power", &Diags::ProcessPower},
    {"powersettings", &Diags::ProcessPowerSettings},
    {"rawpowersetting", &Diags::ProcessRawPowerSetting},
    {"radio", &Diags::ProcessRadio},
    {"repeat", &Diags::ProcessRepeat},
    {"send", &Diags::ProcessSend},
    {"start", &Diags::ProcessStart},
    {"stats", &Diags::ProcessStats},
    {"stop", &Diags::ProcessStop},
    {"stream", &Diags::ProcessStream},
};

Diags::Diags(Instance &aInstance)
    : InstanceLocator(aInstance)
    , mTxPacket(&Get<Radio>().GetTransmitBuffer())
    , mTxPeriod(0)
    , mTxPackets(0)
    , mChannel(20)
    , mTxPower(0)
    , mTxLen(0)
    , mRepeatActive(false)
    , mDiagSendOn(false)
{
    mStats.Clear();
}

Error Diags::ProcessChannel(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);

    if (aArgsLength == 0)
    {
        snprintf(aOutput, aOutputMaxLen, "channel: %d\r\n", mChannel);
    }
    else
    {
        long value;

        SuccessOrExit(error = ParseLong(aArgs[0], value));
        VerifyOrExit(value >= Radio::kChannelMin && value <= Radio::kChannelMax, error = kErrorInvalidArgs);

        mChannel = static_cast<uint8_t>(value);
        IgnoreError(Get<Radio>().Receive(mChannel));
        otPlatDiagChannelSet(mChannel);

        snprintf(aOutput, aOutputMaxLen, "set channel to %d\r\nstatus 0x%02x\r\n", mChannel, error);
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessPower(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);

    if (aArgsLength == 0)
    {
        snprintf(aOutput, aOutputMaxLen, "tx power: %d dBm\r\n", mTxPower);
    }
    else
    {
        long value;

        SuccessOrExit(error = ParseLong(aArgs[0], value));

        mTxPower = static_cast<int8_t>(value);
        SuccessOrExit(error = Get<Radio>().SetTransmitPower(mTxPower));
        otPlatDiagTxPowerSet(mTxPower);

        snprintf(aOutput, aOutputMaxLen, "set tx power to %d dBm\r\nstatus 0x%02x\r\n", mTxPower, error);
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessRepeat(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);
    VerifyOrExit(aArgsLength > 0, error = kErrorInvalidArgs);

    if (strcmp(aArgs[0], "stop") == 0)
    {
        otPlatAlarmMilliStop(&GetInstance());
        mRepeatActive = false;
        snprintf(aOutput, aOutputMaxLen, "repeated packet transmission is stopped\r\nstatus 0x%02x\r\n", error);
    }
    else
    {
        long value;

        VerifyOrExit(aArgsLength == 2, error = kErrorInvalidArgs);

        SuccessOrExit(error = ParseLong(aArgs[0], value));
        mTxPeriod = static_cast<uint32_t>(value);

        SuccessOrExit(error = ParseLong(aArgs[1], value));
        VerifyOrExit(value <= OT_RADIO_FRAME_MAX_SIZE, error = kErrorInvalidArgs);
        VerifyOrExit(value >= OT_RADIO_FRAME_MIN_SIZE, error = kErrorInvalidArgs);
        mTxLen = static_cast<uint8_t>(value);

        mRepeatActive = true;
        uint32_t now  = otPlatAlarmMilliGetNow();
        otPlatAlarmMilliStartAt(&GetInstance(), now, mTxPeriod);
        snprintf(aOutput, aOutputMaxLen, "sending packets of length %#x at the delay of %#x ms\r\nstatus 0x%02x\r\n",
                 static_cast<int>(mTxLen), static_cast<int>(mTxPeriod), error);
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessSend(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;
    long  value;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);
    VerifyOrExit(aArgsLength == 2, error = kErrorInvalidArgs);

    SuccessOrExit(error = ParseLong(aArgs[0], value));
    mTxPackets = static_cast<uint32_t>(value);

    SuccessOrExit(error = ParseLong(aArgs[1], value));
    VerifyOrExit(value <= OT_RADIO_FRAME_MAX_SIZE, error = kErrorInvalidArgs);
    VerifyOrExit(value >= OT_RADIO_FRAME_MIN_SIZE, error = kErrorInvalidArgs);
    mTxLen = static_cast<uint8_t>(value);

    snprintf(aOutput, aOutputMaxLen, "sending %#x packet(s), length %#x\r\nstatus 0x%02x\r\n",
             static_cast<int>(mTxPackets), static_cast<int>(mTxLen), error);
    TransmitPacket();

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessStart(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    Error error = kErrorNone;

#if OPENTHREAD_FTD || OPENTHREAD_MTD
    VerifyOrExit(!Get<ThreadNetif>().IsUp(), error = kErrorInvalidState);
#endif

    otPlatDiagChannelSet(mChannel);
    otPlatDiagTxPowerSet(mTxPower);

    IgnoreError(Get<Radio>().Enable());
    Get<Radio>().SetPromiscuous(true);
    otPlatAlarmMilliStop(&GetInstance());
    SuccessOrExit(error = Get<Radio>().Receive(mChannel));
    SuccessOrExit(error = Get<Radio>().SetTransmitPower(mTxPower));
    otPlatDiagModeSet(true);
    mStats.Clear();
    snprintf(aOutput, aOutputMaxLen, "start diagnostics mode\r\nstatus 0x%02x\r\n", error);

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessStats(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);

    if ((aArgsLength == 1) && (strcmp(aArgs[0], "clear") == 0))
    {
        mStats.Clear();
        snprintf(aOutput, aOutputMaxLen, "stats cleared\r\n");
    }
    else
    {
        VerifyOrExit(aArgsLength == 0, error = kErrorInvalidArgs);
        snprintf(aOutput, aOutputMaxLen,
                 "received packets: %d\r\nsent packets: %d\r\n"
                 "first received packet: rssi=%d, lqi=%d\r\n"
                 "last received packet: rssi=%d, lqi=%d\r\n",
                 static_cast<int>(mStats.mReceivedPackets), static_cast<int>(mStats.mSentPackets),
                 static_cast<int>(mStats.mFirstRssi), static_cast<int>(mStats.mFirstLqi),
                 static_cast<int>(mStats.mLastRssi), static_cast<int>(mStats.mLastLqi));
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessStop(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    Error error = kErrorNone;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);

    otPlatAlarmMilliStop(&GetInstance());
    otPlatDiagModeSet(false);
    Get<Radio>().SetPromiscuous(false);

    snprintf(aOutput, aOutputMaxLen,
             "received packets: %d\r\nsent packets: %d\r\n"
             "first received packet: rssi=%d, lqi=%d\r\n"
             "last received packet: rssi=%d, lqi=%d\r\n"
             "\nstop diagnostics mode\r\nstatus 0x%02x\r\n",
             static_cast<int>(mStats.mReceivedPackets), static_cast<int>(mStats.mSentPackets),
             static_cast<int>(mStats.mFirstRssi), static_cast<int>(mStats.mFirstLqi),
             static_cast<int>(mStats.mLastRssi), static_cast<int>(mStats.mLastLqi), error);

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

void Diags::TransmitPacket(void)
{
    mTxPacket->mLength  = mTxLen;
    mTxPacket->mChannel = mChannel;

    for (uint8_t i = 0; i < mTxLen; i++)
    {
        mTxPacket->mPsdu[i] = i;
    }

    mDiagSendOn = true;
    IgnoreError(Get<Radio>().Transmit(*static_cast<Mac::TxFrame *>(mTxPacket)));
}

Error Diags::ProcessRadio(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorInvalidArgs;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);
    VerifyOrExit(aArgsLength > 0, error = kErrorInvalidArgs);

    if (strcmp(aArgs[0], "sleep") == 0)
    {
        SuccessOrExit(error = Get<Radio>().Sleep());
        snprintf(aOutput, aOutputMaxLen, "set radio from receive to sleep \r\nstatus 0x%02x\r\n", error);
    }
    else if (strcmp(aArgs[0], "receive") == 0)
    {
        SuccessOrExit(error = Get<Radio>().Receive(mChannel));
        SuccessOrExit(error = Get<Radio>().SetTransmitPower(mTxPower));
        otPlatDiagChannelSet(mChannel);
        otPlatDiagTxPowerSet(mTxPower);

        snprintf(aOutput, aOutputMaxLen, "set radio from sleep to receive on channel %d\r\nstatus 0x%02x\r\n", mChannel,
                 error);
    }
    else if (strcmp(aArgs[0], "state") == 0)
    {
        otRadioState state = Get<Radio>().GetState();

        error = kErrorNone;

        switch (state)
        {
        case OT_RADIO_STATE_DISABLED:
            snprintf(aOutput, aOutputMaxLen, "disabled\r\n");
            break;

        case OT_RADIO_STATE_SLEEP:
            snprintf(aOutput, aOutputMaxLen, "sleep\r\n");
            break;

        case OT_RADIO_STATE_RECEIVE:
            snprintf(aOutput, aOutputMaxLen, "receive\r\n");
            break;

        case OT_RADIO_STATE_TRANSMIT:
            snprintf(aOutput, aOutputMaxLen, "transmit\r\n");
            break;

        default:
            snprintf(aOutput, aOutputMaxLen, "invalid\r\n");
            break;
        }
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

extern "C" void otPlatDiagAlarmFired(otInstance *aInstance) { AsCoreType(aInstance).Get<Diags>().AlarmFired(); }

void Diags::AlarmFired(void)
{
    if (mRepeatActive)
    {
        uint32_t now = otPlatAlarmMilliGetNow();

        TransmitPacket();
        otPlatAlarmMilliStartAt(&GetInstance(), now, mTxPeriod);
    }
    else
    {
        otPlatDiagAlarmCallback(&GetInstance());
    }
}

void Diags::ReceiveDone(otRadioFrame *aFrame, Error aError)
{
    if (aError == kErrorNone)
    {
        // for sensitivity test, only record the rssi and lqi for the first and last packet
        if (mStats.mReceivedPackets == 0)
        {
            mStats.mFirstRssi = aFrame->mInfo.mRxInfo.mRssi;
            mStats.mFirstLqi  = aFrame->mInfo.mRxInfo.mLqi;
        }

        mStats.mLastRssi = aFrame->mInfo.mRxInfo.mRssi;
        mStats.mLastLqi  = aFrame->mInfo.mRxInfo.mLqi;

        mStats.mReceivedPackets++;
    }

    otPlatDiagRadioReceived(&GetInstance(), aFrame, aError);
}

void Diags::TransmitDone(Error aError)
{
    VerifyOrExit(mDiagSendOn);
    mDiagSendOn = false;

    if (aError == kErrorNone)
    {
        mStats.mSentPackets++;

        if (mTxPackets > 1)
        {
            mTxPackets--;
        }
        else
        {
            ExitNow();
        }
    }

    VerifyOrExit(!mRepeatActive);
    TransmitPacket();

exit:
    return;
}

#endif // OPENTHREAD_RADIO

Error Diags::ProcessContinuousWave(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorInvalidArgs;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);
    VerifyOrExit(aArgsLength > 0, error = kErrorInvalidArgs);

    if (strcmp(aArgs[0], "start") == 0)
    {
        SuccessOrExit(error = otPlatDiagRadioTransmitCarrier(&GetInstance(), true));
    }
    else if (strcmp(aArgs[0], "stop") == 0)
    {
        SuccessOrExit(error = otPlatDiagRadioTransmitCarrier(&GetInstance(), false));
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessStream(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorInvalidArgs;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);
    VerifyOrExit(aArgsLength > 0, error = kErrorInvalidArgs);

    if (strcmp(aArgs[0], "start") == 0)
    {
        error = otPlatDiagRadioTransmitStream(&GetInstance(), true);
    }
    else if (strcmp(aArgs[0], "stop") == 0)
    {
        error = otPlatDiagRadioTransmitStream(&GetInstance(), false);
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::GetPowerSettings(uint8_t aChannel, PowerSettings &aPowerSettings)
{
    aPowerSettings.mRawPowerSetting.mLength = RawPowerSetting::kMaxDataSize;
    return otPlatDiagRadioGetPowerSettings(&GetInstance(), aChannel, &aPowerSettings.mTargetPower,
                                           &aPowerSettings.mActualPower, aPowerSettings.mRawPowerSetting.mData,
                                           &aPowerSettings.mRawPowerSetting.mLength);
}

Error Diags::ProcessPowerSettings(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error         error = kErrorInvalidArgs;
    uint8_t       channel;
    PowerSettings powerSettings;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);

    if (aArgsLength == 0)
    {
        bool          isPrePowerSettingsValid = false;
        uint8_t       preChannel              = 0;
        PowerSettings prePowerSettings;
        int           n;

        n = snprintf(aOutput, aOutputMaxLen,
                     "| StartCh | EndCh | TargetPower | ActualPower | RawPowerSetting |\r\n"
                     "+---------+-------+-------------+-------------+-----------------+\r\n");
        VerifyOrExit((n > 0) && (n < static_cast<int>(aOutputMaxLen)), error = kErrorNoBufs);
        aOutput += n;
        aOutputMaxLen -= static_cast<size_t>(n);

        for (channel = Radio::kChannelMin; channel <= Radio::kChannelMax + 1; channel++)
        {
            error = (channel == Radio::kChannelMax + 1) ? kErrorNotFound : GetPowerSettings(channel, powerSettings);

            if (isPrePowerSettingsValid && ((powerSettings != prePowerSettings) || (error != kErrorNone)))
            {
                n = snprintf(aOutput, aOutputMaxLen, "| %7u | %5u | %11d | %11d | %15s |\r\n", preChannel, channel - 1,
                             prePowerSettings.mTargetPower, prePowerSettings.mActualPower,
                             prePowerSettings.mRawPowerSetting.ToString().AsCString());
                VerifyOrExit((n > 0) && (n < static_cast<int>(aOutputMaxLen)), error = kErrorNoBufs);
                aOutput += n;
                aOutputMaxLen -= static_cast<size_t>(n);
                isPrePowerSettingsValid = false;
            }

            if ((error == kErrorNone) && (!isPrePowerSettingsValid))
            {
                preChannel              = channel;
                prePowerSettings        = powerSettings;
                isPrePowerSettingsValid = true;
            }
        }

        error = kErrorNone;
    }
    else if (aArgsLength == 1)
    {
        SuccessOrExit(error = Utils::CmdLineParser::ParseAsUint8(aArgs[0], channel));
        VerifyOrExit(channel >= Radio::kChannelMin && channel <= Radio::kChannelMax, error = kErrorInvalidArgs);

        SuccessOrExit(error = GetPowerSettings(channel, powerSettings));
        snprintf(aOutput, aOutputMaxLen,
                 "TargetPower(0.01dBm): %d\r\nActualPower(0.01dBm): %d\r\nRawPowerSetting: %s\r\n",
                 powerSettings.mTargetPower, powerSettings.mActualPower,
                 powerSettings.mRawPowerSetting.ToString().AsCString());
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::GetRawPowerSetting(RawPowerSetting &aRawPowerSetting)
{
    aRawPowerSetting.mLength = RawPowerSetting::kMaxDataSize;
    return otPlatDiagRadioGetRawPowerSetting(&GetInstance(), aRawPowerSetting.mData, &aRawPowerSetting.mLength);
}

Error Diags::ProcessRawPowerSetting(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error           error = kErrorInvalidArgs;
    RawPowerSetting setting;

    VerifyOrExit(otPlatDiagModeGet(), error = kErrorInvalidState);

    if (aArgsLength == 0)
    {
        SuccessOrExit(error = GetRawPowerSetting(setting));
        snprintf(aOutput, aOutputMaxLen, "%s\r\n", setting.ToString().AsCString());
    }
    else if (strcmp(aArgs[0], "enable") == 0)
    {
        SuccessOrExit(error = otPlatDiagRadioRawPowerSettingEnable(&GetInstance(), true));
    }
    else if (strcmp(aArgs[0], "disable") == 0)
    {
        SuccessOrExit(error = otPlatDiagRadioRawPowerSettingEnable(&GetInstance(), false));
    }
    else
    {
        setting.mLength = RawPowerSetting::kMaxDataSize;
        SuccessOrExit(error = Utils::CmdLineParser::ParseAsHexString(aArgs[0], setting.mLength, setting.mData));
        SuccessOrExit(error = otPlatDiagRadioSetRawPowerSetting(&GetInstance(), setting.mData, setting.mLength));
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

Error Diags::ProcessGpio(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error      error = kErrorInvalidArgs;
    long       value;
    uint32_t   gpio;
    bool       level;
    otGpioMode mode;

    if ((aArgsLength == 2) && (strcmp(aArgs[0], "get") == 0))
    {
        SuccessOrExit(error = ParseLong(aArgs[1], value));
        gpio = static_cast<uint32_t>(value);
        SuccessOrExit(error = otPlatDiagGpioGet(gpio, &level));
        snprintf(aOutput, aOutputMaxLen, "%d\r\n", level);
    }
    else if ((aArgsLength == 3) && (strcmp(aArgs[0], "set") == 0))
    {
        SuccessOrExit(error = ParseLong(aArgs[1], value));
        gpio = static_cast<uint32_t>(value);
        SuccessOrExit(error = ParseBool(aArgs[2], level));
        SuccessOrExit(error = otPlatDiagGpioSet(gpio, level));
    }
    else if ((aArgsLength >= 2) && (strcmp(aArgs[0], "mode") == 0))
    {
        SuccessOrExit(error = ParseLong(aArgs[1], value));
        gpio = static_cast<uint32_t>(value);

        if (aArgsLength == 2)
        {
            SuccessOrExit(error = otPlatDiagGpioGetMode(gpio, &mode));
            if (mode == OT_GPIO_MODE_INPUT)
            {
                snprintf(aOutput, aOutputMaxLen, "in\r\n");
            }
            else if (mode == OT_GPIO_MODE_OUTPUT)
            {
                snprintf(aOutput, aOutputMaxLen, "out\r\n");
            }
        }
        else if ((aArgsLength == 3) && (strcmp(aArgs[2], "in") == 0))
        {
            SuccessOrExit(error = otPlatDiagGpioSetMode(gpio, OT_GPIO_MODE_INPUT));
        }
        else if ((aArgsLength == 3) && (strcmp(aArgs[2], "out") == 0))
        {
            SuccessOrExit(error = otPlatDiagGpioSetMode(gpio, OT_GPIO_MODE_OUTPUT));
        }
    }

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

void Diags::AppendErrorResult(Error aError, char *aOutput, size_t aOutputMaxLen)
{
    if (aError != kErrorNone)
    {
        snprintf(aOutput, aOutputMaxLen, "failed\r\nstatus %#x\r\n", aError);
    }
}

Error Diags::ParseLong(char *aString, long &aLong)
{
    char *endptr;
    aLong = strtol(aString, &endptr, 0);
    return (*endptr == '\0') ? kErrorNone : kErrorParse;
}

Error Diags::ParseBool(char *aString, bool &aBool)
{
    Error error;
    long  value;

    SuccessOrExit(error = ParseLong(aString, value));
    VerifyOrExit((value == 0) || (value == 1), error = kErrorParse);
    aBool = static_cast<bool>(value);

exit:
    return error;
}

Error Diags::ParseCmd(char *aString, uint8_t &aArgsLength, char *aArgs[])
{
    Error                     error;
    Utils::CmdLineParser::Arg args[kMaxArgs + 1];

    SuccessOrExit(error = Utils::CmdLineParser::ParseCmd(aString, args));
    aArgsLength = Utils::CmdLineParser::Arg::GetArgsLength(args);
    Utils::CmdLineParser::Arg::CopyArgsToStringArray(args, aArgs);

exit:
    return error;
}

Error Diags::ProcessLine(const char *aString, char *aOutput, size_t aOutputMaxLen)
{
    constexpr uint16_t kMaxCommandBuffer = OPENTHREAD_CONFIG_DIAG_CMD_LINE_BUFFER_SIZE;

    Error   error = kErrorNone;
    char    buffer[kMaxCommandBuffer];
    char   *args[kMaxArgs];
    uint8_t argCount = 0;

    VerifyOrExit(StringLength(aString, kMaxCommandBuffer) < kMaxCommandBuffer, error = kErrorNoBufs);

    strlcpy(buffer, aString, kMaxCommandBuffer);
    error = ParseCmd(buffer, argCount, args);

exit:

    switch (error)
    {
    case kErrorNone:
        aOutput[0] = '\0'; // In case there is no output.
        error      = ProcessCmd(argCount, &args[0], aOutput, aOutputMaxLen);
        break;

    case kErrorNoBufs:
        snprintf(aOutput, aOutputMaxLen, "failed: command string too long\r\n");
        break;

    case kErrorInvalidArgs:
        snprintf(aOutput, aOutputMaxLen, "failed: command string contains too many arguments\r\n");
        break;

    default:
        snprintf(aOutput, aOutputMaxLen, "failed to parse command string\r\n");
        break;
    }

    return error;
}

Error Diags::ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    // This `rcp` command is for debugging and testing only, building only when NDEBUG is not defined
    // so that it will be excluded from release build.
#if !defined(NDEBUG) && defined(OPENTHREAD_RADIO)
    if (aArgsLength > 0 && !strcmp(aArgs[0], "rcp"))
    {
        aArgs++;
        aArgsLength--;
    }
#endif

    if (aArgsLength == 0)
    {
        snprintf(aOutput, aOutputMaxLen, "diagnostics mode is %s\r\n", otPlatDiagModeGet() ? "enabled" : "disabled");
        ExitNow();
    }
    else
    {
        aOutput[0] = '\0';
    }

    for (const Command &command : sCommands)
    {
        if (strcmp(aArgs[0], command.mName) == 0)
        {
            error = (this->*command.mCommand)(aArgsLength - 1, (aArgsLength > 1) ? &aArgs[1] : nullptr, aOutput,
                                              aOutputMaxLen);
            ExitNow();
        }
    }

    // more platform specific features will be processed under platform layer
    error = otPlatDiagProcess(&GetInstance(), aArgsLength, aArgs, aOutput, aOutputMaxLen);

exit:
    // Add more platform specific diagnostics features here.
    if (error == kErrorInvalidCommand && aArgsLength > 1)
    {
        snprintf(aOutput, aOutputMaxLen, "diag feature '%s' is not supported\r\n", aArgs[0]);
    }

    return error;
}

bool Diags::IsEnabled(void) { return otPlatDiagModeGet(); }

} // namespace FactoryDiags
} // namespace ot

OT_TOOL_WEAK otError otPlatDiagGpioSet(uint32_t aGpio, bool aValue)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aValue);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagGpioGet(uint32_t aGpio, bool *aValue)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aValue);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagGpioSetMode(uint32_t aGpio, otGpioMode aMode)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aMode);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagGpioGetMode(uint32_t aGpio, otGpioMode *aMode)
{
    OT_UNUSED_VARIABLE(aGpio);
    OT_UNUSED_VARIABLE(aMode);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagRadioSetRawPowerSetting(otInstance    *aInstance,
                                                       const uint8_t *aRawPowerSetting,
                                                       uint16_t       aRawPowerSettingLength)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aRawPowerSetting);
    OT_UNUSED_VARIABLE(aRawPowerSettingLength);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagRadioGetRawPowerSetting(otInstance *aInstance,
                                                       uint8_t    *aRawPowerSetting,
                                                       uint16_t   *aRawPowerSettingLength)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aRawPowerSetting);
    OT_UNUSED_VARIABLE(aRawPowerSettingLength);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagRadioRawPowerSettingEnable(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnable);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagRadioTransmitCarrier(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnable);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagRadioTransmitStream(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aEnable);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatDiagRadioGetPowerSettings(otInstance *aInstance,
                                                     uint8_t     aChannel,
                                                     int16_t    *aTargetPower,
                                                     int16_t    *aActualPower,
                                                     uint8_t    *aRawPowerSetting,
                                                     uint16_t   *aRawPowerSettingLength)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aChannel);
    OT_UNUSED_VARIABLE(aTargetPower);
    OT_UNUSED_VARIABLE(aActualPower);
    OT_UNUSED_VARIABLE(aRawPowerSetting);
    OT_UNUSED_VARIABLE(aRawPowerSettingLength);

    return OT_ERROR_NOT_IMPLEMENTED;
}
#endif // OPENTHREAD_CONFIG_DIAG_ENABLE
