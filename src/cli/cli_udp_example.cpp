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

#include "cli_udp_example.hpp"

#include <openthread/config.h>
#include <openthread/message.h>
#include <openthread/udp.h>

#include "cli/cli.hpp"
#include "cli_uart.hpp"
#include "common/encoding.hpp"

using ot::Encoding::BigEndian::HostSwap16;

namespace ot {
namespace Cli {

Udp::Udp(Interpreter &aInterpreter):
    mInterpreter(aInterpreter),
    mLength(1232),
    mCount(0),
    mInterval(1),
    mPingTimer(*aInterpreter.mInstance, &Udp::s_HandlePingTimer, this)
{
}

void Udp::Init(void)
{
    mCount = 0;
    mLossNum = 0;
    mLatency = 0;
    mTimestamp = 0;
    mTimeElapse = 0;
    mJitter = 0;
    mAcceptTimestamp = 0;
}

otError Udp::SendUdpPacket(void)
{
    otError error;
    uint32_t timestamp = 0;
    otMessage *message;

    timestamp = TimerMilli::GetNow();
    mInterpreter.mServer->OutputFormat("the timestamp is %u and count is %d \r\n", static_cast<unsigned int>(timestamp), mCount);

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

    mCount++;

exit:

    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }

    return error;

}

uint32_t Udp::GetAcceptedTimestamp(otMessage *aMessage)
{
    uint8_t buf[1500];
    int length;
    uint32_t timestamp;

    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';
    timestamp = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    return timestamp;
}

uint32_t Udp::GetAcceptedCount(otMessage *aMessage)
{
    uint8_t buf[1500];
    int length;
    uint32_t count;

    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';
    count = buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7];
    return count;
}

const struct Udp::Command Udp::sCommands[] =
{
    { "help", &Udp::ProcessHelp },
    { "bind", &Udp::ProcessBind },
    { "close", &Udp::ProcessClose },
    { "connect", &Udp::ProcessConnect },
    { "open", &Udp::ProcessOpen },
    { "send", &Udp::ProcessSend },
    { "test", &Udp::ProcessTest }
};

otError Udp::ProcessHelp(int argc, char *argv[])
{
    for (unsigned int i = 0; i < sizeof(sCommands) / sizeof(sCommands[0]); i++)
    {
        mInterpreter.mServer->OutputFormat("%s\r\n", sCommands[i].mName);
    }

    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return OT_ERROR_NONE;
}

otError Udp::ProcessBind(int argc, char *argv[])
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

otError Udp::ProcessConnect(int argc, char *argv[])
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

otError Udp::ProcessClose(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return otUdpClose(&mSocket);
}

otError Udp::ProcessOpen(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);
    return otUdpOpen(mInterpreter.mInstance, &mSocket, HandleUdpReceive, this);
}

otError Udp::ProcessSend(int argc, char *argv[])
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

otError Udp::Process(int argc, char *argv[])
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

void Udp::HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static_cast<Udp *>(aContext)->HandleUdpReceive(aMessage, aMessageInfo);
}

void Udp::HandleUdpReceive(otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    otMessageInfo messageInfo;
    otMessage *message;
    uint32_t timestamp = 0;
    uint32_t sendTimestamp = 0;
    uint16_t count;

    timestamp = TimerMilli::GetNow();

    memset(&messageInfo, 0, sizeof(messageInfo));
    memset(&message, 0, sizeof(message));

    count = GetAcceptedCount(aMessage);
    sendTimestamp = GetAcceptedTimestamp(aMessage);

    if (count == 0)
    {
        Init();
        mTimestamp = TimerMilli::GetNow();
        mAcceptTimestamp = sendTimestamp;
    }

    if (count - mCount > 0)
    {
        mLossNum = count - mCount;
        mCount = count + 1;
    }
    else
    {
        mCount++;
    }

    if (mTimestamp < timestamp)
    {
//        mTimeElapse = mTimeElapse + timestamp - mTimestamp;
        mTimeElapse = timestamp - mTimestamp;
        mTimestamp = timestamp;
    }

    if (mAcceptTimestamp < sendTimestamp)
    {
        mJitter = sendTimestamp - mAcceptTimestamp;
        mAcceptTimestamp = sendTimestamp;
    }

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
    //TODO: count the maximum throughput after the following test



}

Udp &Udp::GetOwner(const Context &aContext)
{
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    Udp &udp = *static_cast<Udp *>(aContext.GetContext());
#else
    Udp &udp = Uart::sUartServer->GetInterpreter().mUdp;
    OT_UNUSED_VARIABLE(aContext);
#endif
    return udp;
}

void Udp::s_HandlePingTimer(Timer &aTimer)
{
    GetOwner(aTimer).HandlePingTimer();
}


void Udp::HandlePingTimer()
{
    otError error;

    error = SendUdpPacket();
    SuccessOrExit(error);
exit:
    if (mCount < mInitialCount)
    {
        mPingTimer.Start(mInterval);
    }
    else
    {
        Init();
    }

}

otError Udp::ProcessTest(int argc, char *argv[])
{
    otError error;
    int curArg = 0;

    memset(&mMessageInfo, 0, sizeof(mMessageInfo));

    VerifyOrExit(argc == 2 || argc == 5, error = OT_ERROR_PARSE);

    long value;

    error = otIp6AddressFromString(argv[curArg++], &mMessageInfo.mPeerAddr);
    SuccessOrExit(error);

    error = Interpreter::ParseLong(argv[curArg++], value);
    SuccessOrExit(error);

    mMessageInfo.mPeerPort = static_cast<uint16_t>(value);

    error = Interpreter::ParseLong(argv[curArg++], value);
    SuccessOrExit(error);

    if (argc == 5)
    {

        mLength = value;

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);
             
        mInitialCount = value;

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        mInterval = value;
    }

    HandlePingTimer();

exit:

    if (error != OT_ERROR_NONE && mMessage != NULL)
    {
        otMessageFree(mMessage);
    }

    return error;
}


}  // namespace Cli
}  // namespace ot
