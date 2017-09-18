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

#include "cli_udp_latency.hpp"

#include <openthread/config.h>
#include <openthread/message.h>
#include <openthread/udp.h>

#include "cli/cli.hpp"
#include "cli_uart.hpp"
#include "common/encoding.hpp"

using ot::Encoding::BigEndian::HostSwap16;

namespace ot {
namespace Cli {

UdpLatency::UdpLatency(Interpreter &aInterpreter):
    mInterpreter(aInterpreter),
    mCount(10),
    mPingTimer(*aInterpreter.mInstance, &UdpLatency::s_HandlePingTimer, this)
    {
    }


const struct UdpLatency::Command UdpLatency::sCommands[] =
{
    { "help", &UdpLatency::ProcessHelp },
    { "bind", &UdpLatency::ProcessBind },
    { "close", &UdpLatency::ProcessClose },
    { "connect", &UdpLatency::ProcessConnect },
    { "open", &UdpLatency::ProcessOpen },
    { "send", &UdpLatency::ProcessSend },
    { "test", &UdpLatency::ProcessTest }
};


otError UdpLatency::SendUDPPacket()
{
    otMessage *message;
    otError error = OT_ERROR_NONE;
    uint32_t timestamp = 0;
    timestamp = TimerMilli::GetNow();

    mPayload[0] = timestamp >> 24;
    mPayload[1] = timestamp >> 16;
    mPayload[2] = timestamp >> 8;
    mPayload[3] = timestamp;
    mPayload[4] = mSequence >> 24;
    mPayload[5] = mSequence >> 16;
    mPayload[6] = mSequence >> 8;
    mPayload[7] = mSequence;

    for (uint16_t i = 8; i < mLength; i++)
    {
        mPayload[i] = 'T';
    }

    mSequence++;

    message = otUdpNewMessage(mInterpreter.mInstance, true);

//    printf("Send pkt sequence No is %u, and length is %d\n", mSequence, mLength);
    if (message == NULL)
    {
        ExitNow();
    }
    error = otMessageAppend(message, mPayload, mLength);

    error = otUdpSend(&mSocket, message, &mMessageInfo);

    mIsReceived = false;

exit:
    // mPingTimer.Start(mInterval);

    return error;
}

uint32_t UdpLatency::ReadSequence(otMessage *aMessage)
{
    uint8_t buf[1500];
    int length;
    uint32_t sequence;
    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';
    sequence = buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7];
    return sequence;
}

uint32_t UdpLatency::ReadTimestamp(otMessage *aMessage)
{
    uint8_t buf[1500];
    int length;
    uint32_t timestamp;
    length = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';
    timestamp = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    return timestamp;
}

void UdpLatency::Init()
{
    mSequence = 0;
    mLossNum = 0;
    mIsReceived = false;
    mLatency = 0;
    mTimestamp = 0;
    mTimeElapse = 0;
}

otError UdpLatency::ProcessHelp(int argc, char *argv[])
{
    for (unsigned int i = 0; i < sizeof(sCommands) / sizeof(sCommands[0]); i++)
    {
        mInterpreter.mServer->OutputFormat("%s\r\n", sCommands[i].mName);
    }

    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return OT_ERROR_NONE;
}

otError UdpLatency::ProcessBind(int argc, char *argv[])
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

otError UdpLatency::ProcessConnect(int argc, char *argv[])
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

otError UdpLatency::ProcessClose(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return otUdpClose(&mSocket);
}

otError UdpLatency::ProcessOpen(int argc, char *argv[])
{
    // OT_UNUSED_VARIABLE(argc);
    // OT_UNUSED_VARIABLE(argv);
    otError error;
    int curArg = 0;

    VerifyOrExit(argc == 1, error = OT_ERROR_PARSE);

    if(argc == 1)
    {
        long value;
        error = Interpreter::ParseLong(argv[curArg], value);
        mRole = value;
    }
    error = otUdpOpen(mInterpreter.mInstance, &mSocket, HandleUdpReceive, this);

exit:
    return error;
}

otError UdpLatency::ProcessSend(int argc, char *argv[])
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

otError UdpLatency::Process(int argc, char *argv[])
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

void UdpLatency::HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static_cast<UdpLatency *>(aContext)->HandleUdpReceive(aMessage, aMessageInfo);
}

void UdpLatency::HandleUdpReceive(otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    int length;
    otMessageInfo messageInfo;
    otMessage *message;
    uint32_t sequence;
    uint32_t timestamp;

    mIsReceived = true;

    memset(&messageInfo, 0, sizeof(messageInfo));
    memset(&message, 0, sizeof(message));

    sequence = ReadSequence(aMessage);
    timestamp = TimerMilli::GetNow();
    if (mRole == 2)
    {
        if (sequence <= 0)
        {
            Init();
            mTimestamp = TimerMilli::GetNow();
        }

        if (sequence - mSequence > 0)
        {
            mLossNum = sequence - mSequence;
            mSequence = sequence;
        }
        else
        {
            mSequence++;
        }
        if (mTimestamp < timestamp)
        {
            mTimeElapse = timestamp - mTimestamp;
            mTimestamp = timestamp;
        }

        memcpy(messageInfo.mPeerAddr.mFields.m8, aMessageInfo->mPeerAddr.mFields.m8, sizeof(aMessageInfo->mSockAddr.mFields.m8));
        messageInfo.mPeerPort = aMessageInfo->mPeerPort;
        memcpy(messageInfo.mSockAddr.mFields.m8, aMessageInfo->mSockAddr.mFields.m8, sizeof(aMessageInfo->mPeerAddr.mFields.m8));
        messageInfo.mSockPort = aMessageInfo->mSockPort;
        length = otMessageRead(aMessage, otMessageGetOffset(aMessage), mPayload, sizeof(mPayload) - 1);
        mPayload[length] = '\0';
        message = otUdpNewMessage(mInterpreter.mInstance, true);
        otMessageAppend(message, mPayload, static_cast<uint16_t>(length));
        otUdpSend(&mSocket, message, &messageInfo);
        mInterpreter.mServer->OutputFormat("SeqNo: %u, Elapse: %u, Lost: %u,\r\n", static_cast<unsigned int>(mSequence), static_cast<unsigned int>(mTimeElapse), static_cast<unsigned int>(mLossNum));
        mInterpreter.mServer->OutputFormat("%x:%x:%x:%x:%x:%x:%x:%x %d \r\n",
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[0]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[1]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[2]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[3]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[4]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[5]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[6]),
                                           HostSwap16(messageInfo.mPeerAddr.mFields.m16[7]),
                                           messageInfo.mPeerPort);

        //retrieve the data records the sequence
    }
    else if (mRole == 1)
    {
        //send the next packets
//        printf("Send received message and reply it to sender %u\n", mSequence);
        if (mSequence <= mCount)
        {
            uint32_t readTimestamp = ReadTimestamp(aMessage);
            mTimeElapse = timestamp - readTimestamp;
            mInterpreter.mServer->OutputFormat("SeqNo: %u, Elapse: %u,\r\n", static_cast<unsigned int>(mSequence), static_cast<unsigned int>(mTimeElapse));
            SendUDPPacket();
        }

    }
}

UdpLatency &UdpLatency::GetOwner(const Context &aContext)
{
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    UdpLatency &udp = *static_cast<UdpLatency *>(aContext.GetContext());
#else
    UdpLatency &udp = Uart::sUartServer->GetInterpreter().mUdpLatency;
    OT_UNUSED_VARIABLE(aContext);
#endif
    return udp;
}

void UdpLatency::s_HandlePingTimer(Timer &aTimer)
{
    GetOwner(aTimer).HandlePingTimer();
}


void UdpLatency::HandlePingTimer()
{
    if (mIsReceived == false)
    {
        SendUDPPacket();
    }
}

otError UdpLatency::ProcessTest(int argc, char *argv[])
{
    otError error;
    int curArg = 0;

    memset(&mMessageInfo, 0, sizeof(mMessageInfo));

    VerifyOrExit(argc == 1 || argc == 5, error = OT_ERROR_PARSE);

    if (argc == 5)
    {
        long value;

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
             
        mCount = value;

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        mInterval = value;
    }

    Init();
    // FillPayload();

    // message = otUdpNewMessage(mInterpreter.mInstance, true);
    // VerifyOrExit(message != NULL, error = OT_ERROR_NO_BUFS);

    // error = otMessageAppend(message, mPayload, mLength);
    // SuccessOrExit(error);

    // error = otUdpSend(&mSocket, message, &mMessageInfo);
    SendUDPPacket();
    // mPingTimer.Start(mInterval);

exit:

    if (error != OT_ERROR_NONE && mMessage != NULL)
    {
        otMessageFree(mMessage);
    }

    return error;
}


}  // namespace Cli
}  // namespace ot
