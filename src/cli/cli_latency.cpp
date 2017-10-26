/*
 *  Copyright (c) 2017, The OpenThread Authors.
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
 *   This file implements a simple CLI for the CoAP service.
 */

#include "cli_latency.hpp"

// #include <openthread/config.h>
#include <openthread/message.h>
#include <openthread/udp.h>

#include "cli/cli.hpp"
#include "cli_uart.hpp"
#include "common/encoding.hpp"

#define MONITOR_PIN 3
#define CLEAR_PIN_INTERVAL 3

using ot::Encoding::BigEndian::HostSwap16;

namespace ot {
namespace Cli {

CliLatency *CliLatency::sCliLatency;


const struct CliLatency::Command CliLatency::sCommands[] =
{
    { "help", &CliLatency::ProcessHelp },
    { "bind", &CliLatency::ProcessBind },
    { "close", &CliLatency::ProcessClose },
    { "connect", &CliLatency::ProcessConnect },
    { "open", &CliLatency::ProcessOpen },
    { "send", &CliLatency::ProcessSend },
    { "test", &CliLatency::ProcessTest },
    { "result", &CliLatency::ProcessResult },
};

otError CliLatency::ProcessHelp(int argc, char *argv[])
{
    for (unsigned int i = 0; i < sizeof(sCommands) / sizeof(sCommands[0]); i++)
    {
        mInterpreter.mServer->OutputFormat("%s\r\n", sCommands[i].mName);
    }

    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return OT_ERROR_NONE;
}

CliLatency::CliLatency(Interpreter &aInterpreter):
    mInterpreter(aInterpreter),
    mLength(1232),
    mCount(0),
    mInterval(1),
    mPingTimer(*aInterpreter.mInstance, &CliLatency::s_HandlePingTimer, this),
    mGpioTimer(*aInterpreter.mInstance, &CliLatency::s_HandleGpioTimer, this)
{
}

void CliLatency::Init(void)
{
    mCount = 0;
    mLossNum = 0;
    mLatency = 0;
    mTimestamp = 0;
    mTimeElapse = 0;
    mJitter = 0;
    mAcceptTimestamp = 0;
    mIsRun = true;

    otPlatGpioCfgOutput(13);
    otPlatGpioWrite(13, 0);

    // otPlatGpioClear(MONITOR_PIN + 1);
    otPlatGpioCfgOutput(MONITOR_PIN + 1);
    otPlatGpioWrite(MONITOR_PIN + 1, 1);

    otPlatGpioCfgInput(MONITOR_PIN);
    otPlatGpioClear(MONITOR_PIN);
    otPlatGpioWrite(MONITOR_PIN, 0);

    for (int i = 0; i < 1000; i++)
    {
        mSendTimer[i] = 0;
        mReceiveTimer[i] = 0;
    }
}

otError CliLatency::SendUdpPacket(void)
{
    otError error;
    uint32_t timestamp = 0;
    otMessage *message;

    timestamp = TimerMilli::GetNow();
    memset(mPayload, 0, sizeof(mPayload));
    
    mPayload[0] = timestamp >> 24;
    mPayload[1] = timestamp >> 16;
    mPayload[2] = timestamp >> 8;
    mPayload[3] = timestamp;
    mPayload[4] = mCount >> 24;
    mPayload[5] = mCount >> 16;
    mPayload[6] = mCount >> 8;
    mPayload[7] = mCount;

    for (uint16_t i = 8; i < mLength; i++)
    {
        mPayload[i] = 'T';
    }

    message = otUdpNewMessage(mInterpreter.mInstance, true);

    VerifyOrExit(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, mPayload, static_cast<uint16_t>(mLength));

    SuccessOrExit(error);

    error = otUdpSend(&mSocket, message, &mMessageInfo);

    SuccessOrExit(error);
    otPlatGpioWrite(MONITOR_PIN, 1);
    mGpioTimer.Start(CLEAR_PIN_INTERVAL);

    mCount++;

exit:

    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }

    return error;

}

uint32_t CliLatency::GetAcceptedTimestamp(otMessage *aMessage)
{
    uint8_t buf[1500];
    int length;
    uint32_t timestamp;

    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';
    timestamp = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    return timestamp;
}

uint32_t CliLatency::GetAcceptedCount(otMessage *aMessage)
{
    uint8_t buf[1500];
    int length;
    uint32_t count;

    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';
    count = buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7];
    return count;
}

otError CliLatency::ProcessBind(int argc, char *argv[])
{
    otError error;
    otSockAddr sockaddr;
    long value;

    VerifyOrExit(argc == 2, error = OT_ERROR_PARSE);

    memset(&sockaddr, 0, sizeof(sockaddr));

    error = otIp6AddressFromString(argv[0], &sockaddr.mAddress);
    SuccessOrExit(error);

    error = Interpreter::ParseLong(argv[1], value);
    SuccessOrExit(error);

    sockaddr.mPort = static_cast<uint16_t>(value);

    error = otUdpBind(&mSocket, &sockaddr);

exit:
    return error;
}

otError CliLatency::ProcessConnect(int argc, char *argv[])
{
    otError error;
    otSockAddr sockaddr;
    long value;

    VerifyOrExit(argc == 2, error = OT_ERROR_PARSE);

    memset(&sockaddr, 0, sizeof(sockaddr));

    error = otIp6AddressFromString(argv[0], &sockaddr.mAddress);
    SuccessOrExit(error);

    error = Interpreter::ParseLong(argv[1], value);
    SuccessOrExit(error);

    sockaddr.mPort = static_cast<uint16_t>(value);

    error = otUdpConnect(&mSocket, &sockaddr);

exit:
    return error;
}

otError CliLatency::ProcessClose(int argc, char *argv[])
{
    mIsRun = false;
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);
    
    return otUdpClose(&mSocket);
}

otError CliLatency::ProcessOpen(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);
    Init();
    return otUdpOpen(mInterpreter.mInstance, &mSocket, HandleUdpReceive, this);
}

otError CliLatency::ProcessSend(int argc, char *argv[])
{
    otError error;
    otMessageInfo messageInfo;
    otMessage *message = NULL;
    int curArg = 0;

    memset(&messageInfo, 0, sizeof(messageInfo));

    VerifyOrExit(argc == 1 || argc == 3, error = OT_ERROR_PARSE);

    if (argc == 3)
    {
        long value;

        error = otIp6AddressFromString(argv[curArg++], &messageInfo.mPeerAddr);
        SuccessOrExit(error);

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        messageInfo.mPeerPort = static_cast<uint16_t>(value);
    }

    message = otUdpNewMessage(mInterpreter.mInstance, true);
    VerifyOrExit(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, argv[curArg], static_cast<uint16_t>(strlen(argv[curArg])));
    SuccessOrExit(error);

    error = otUdpSend(&mSocket, message, &messageInfo);

exit:

    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }

    return error;
}

otError CliLatency::Process(int argc, char *argv[])
{
    otError error = OT_ERROR_PARSE;

    for (size_t i = 0; i < sizeof(sCommands) / sizeof(sCommands[0]); i++)
    {
        if (strcmp(argv[0], sCommands[i].mName) == 0)
        {
            error = (this->*sCommands[i].mCommand)(argc - 1, argv + 1);
            break;
        }
    }

    return error;
}

void CliLatency::HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static_cast<CliLatency *>(aContext)->HandleUdpReceive(aMessage, aMessageInfo);
}

void CliLatency::HandleUdpReceive(otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    otMessageInfo messageInfo;
    otMessage *message;
    uint32_t timestamp = 0;
    // uint32_t sendTimestamp = 0;
    uint16_t count;

    timestamp = TimerMilli::GetNow();
    // otPlatGpioWrite(MONITOR_PIN, 1);
    // mGpioTimer.Start(CLEAR_PIN_INTERVAL);

    memset(&messageInfo, 0, sizeof(messageInfo));
    memset(&message, 0, sizeof(message));
    
    //Get sequence number from the packet
    count = GetAcceptedCount(aMessage);

    //compare the sequence number with current number recorded.

    // sendTimestamp = GetAcceptedTimestamp(aMessage);

    //new test and reinitialize the receive node
    //but the node will send gpio signal firstly
    if (count == 0)
    {
        Init();
    }
    else
    {
        // record the receive packet timestamp
        if (count >= mCount)
        {
            mReceiveTimer[count] = timestamp;
        }
    }

    // if (mTimestamp < timestamp)
    // {
    //     mTimeElapse = mTimeElapse + timestamp - mTimestamp;
    //     // mTimeElapse = timestamp - mTimestamp;
    //     mTimestamp = timestamp;
    // }

    // if (mAcceptTimestamp < sendTimestamp)
    // {
    //     mJitter = sendTimestamp - mAcceptTimestamp;
    //     mAcceptTimestamp = sendTimestamp;
    // }

    if (count != 0)
    {
        mInterpreter.mServer->OutputFormat("%u, %d, %d, %u, %u from ", timestamp, count, otMessageGetLength(aMessage) - otMessageGetOffset(aMessage), mTimeElapse, mJitter);
        mInterpreter.mServer->OutputFormat("%x:%x:%x:%x:%x:%x:%x:%x %d \r\n",
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[0]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[1]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[2]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[3]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[4]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[5]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[6]),
                                           HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[7]),
                                           aMessageInfo->mPeerPort);
    }
    //TODO: count the maximum throughput after the following test
}

otError CliLatency::ProcessResult(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    for (int i = 1; i < 1000; i++)
    {
        mInterpreter.mServer->OutputFormat("%d, %d \r\n", mSendTimer[i], mReceiveTimer[i]);
    }

    return OT_ERROR_NONE;
} 

CliLatency &CliLatency::GetOwner(const Context &aContext)
{
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    CliLatency &udp = *static_cast<CliLatency *>(aContext.GetContext());
#else
    CliLatency &udp = Uart::sUartServer->GetInterpreter().mCliLatency;
    OT_UNUSED_VARIABLE(aContext);
#endif
    return udp;
}

void CliLatency::s_HandlePingTimer(Timer &aTimer)
{
    GetOwner(aTimer).HandlePingTimer();
}

void CliLatency::s_HandleGpioTimer(Timer &aTimer)
{
    GetOwner(aTimer).HandleGpioTimer();
}

void CliLatency::HandleGpioTimer()
{
    otPlatGpioWrite(MONITOR_PIN, 0);
}

void CliLatency::HandlePingTimer()
{
    otError error = OT_ERROR_NONE;
    uint32_t interval = 0;

    if (mIsRun)
    {
        error = SendUdpPacket();
        SuccessOrExit(error);
    }
    else
    {
        ExitNow();
    }
exit:
    if (error == OT_ERROR_NONE)
    {
        if (mCount <= mInitialCount)
        {
            if(mInterval == 0)
            {
                interval = otPlatRandomGet() % 100 + 500;
            }
            else
            {
                interval = mInterval;
            }
            mPingTimer.Start(interval);
        }
        else
        {
            Init();
        }   
    }
    else
    {
        if(mInterval == 0)
        {
            interval = otPlatRandomGet() % 100 + 500;
        }
        else
        {
            interval = 50;
        }
        mPingTimer.Start(mInterval);
    }

}

otError CliLatency::ProcessTest(int argc, char *argv[])
{
    otError error = OT_ERROR_NONE;
    int curArg = 0;

    memset(&mMessageInfo, 0, sizeof(mMessageInfo));

    VerifyOrExit(argc == 2 || argc == 5, error = OT_ERROR_PARSE);

    long value;


    if (argc == 5)
    {

        error = otIp6AddressFromString(argv[curArg++], &mMessageInfo.mPeerAddr);
        SuccessOrExit(error);

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        mMessageInfo.mPeerPort = static_cast<uint16_t>(value);

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        mLength = value;

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);
             
        mInitialCount = value;

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        mInterval = value;

        mMessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
        HandlePingTimer();
    }
    else if (argc == 2)
    {
        Init();
    }

exit:

    if (error != OT_ERROR_NONE && mMessage != NULL)
    {
        otMessageFree(mMessage);
    }

    return error;
}

void CliLatency::platGpioResponse(void)
{
    //record timestamp
    otPlatGpioToggle(13);
    // mInterpreter.mServer->OutputFormat("receive the packet %d gpio signal\r\n", mCount);
    if (mCount > 0)
    {
        if (mSendTimer[mCount] == 0)
        {
            mSendTimer[mCount] = TimerMilli::GetNow();
            mCount++;
        }
    }
}

extern "C" void otPlatGpioSignalEvent(uint32_t aPinIndex)
{
    (void)aPinIndex;
    CliLatency::sCliLatency->platGpioResponse();

}

}  // namespace Cli
}  // namespace ot
