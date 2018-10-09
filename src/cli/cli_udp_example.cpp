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

#include <openthread/message.h>
#include <openthread/udp.h>
#include <openthread/network_time.h>
#include "cli/cli.hpp"
#include "common/encoding.hpp"

using ot::Encoding::BigEndian::HostSwap16;

namespace ot {
namespace Cli {

const struct UdpExample::Command UdpExample::sCommands[] = {
    {"help", &UdpExample::ProcessHelp},       {"bind", &UdpExample::ProcessBind}, {"close", &UdpExample::ProcessClose},
    {"connect", &UdpExample::ProcessConnect}, {"open", &UdpExample::ProcessOpen}, {"send", &UdpExample::ProcessSend},
    {"result", &UdpExample::ProcessResult}};

UdpExample::UdpExample(Interpreter &aInterpreter)
    : mInterpreter(aInterpreter)
{
    memset(&mSocket, 0, sizeof(mSocket));
}

otError UdpExample::ProcessHelp(int argc, char *argv[])
{
    for (unsigned int i = 0; i < OT_ARRAY_LENGTH(sCommands); i++)
    {
        mInterpreter.mServer->OutputFormat("%s\r\n", sCommands[i].mName);
    }

    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return OT_ERROR_NONE;
}

otError UdpExample::ProcessResult(int argc, char *argv[])
{
    mInterpreter.mServer->OutputFormat("%d\r\n", mLatency);

    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return OT_ERROR_NONE;
}

otError UdpExample::ProcessBind(int argc, char *argv[])
{
    otError    error;
    otSockAddr sockaddr;
    long       value;

    VerifyOrExit(argc == 2, error = OT_ERROR_INVALID_ARGS);

    memset(&sockaddr, 0, sizeof(sockaddr));

    error = otIp6AddressFromString(argv[0], &sockaddr.mAddress);
    SuccessOrExit(error);

    error = Interpreter::ParseLong(argv[1], value);
    SuccessOrExit(error);

    sockaddr.mPort    = static_cast<uint16_t>(value);
    sockaddr.mScopeId = OT_NETIF_INTERFACE_ID_THREAD;

    error = otUdpBind(&mSocket, &sockaddr);

exit:
    return error;
}

otError UdpExample::ProcessConnect(int argc, char *argv[])
{
    otError    error;
    otSockAddr sockaddr;
    long       value;

    VerifyOrExit(argc == 2, error = OT_ERROR_INVALID_ARGS);

    memset(&sockaddr, 0, sizeof(sockaddr));

    error = otIp6AddressFromString(argv[0], &sockaddr.mAddress);
    SuccessOrExit(error);

    error = Interpreter::ParseLong(argv[1], value);
    SuccessOrExit(error);

    sockaddr.mPort    = static_cast<uint16_t>(value);
    sockaddr.mScopeId = OT_NETIF_INTERFACE_ID_THREAD;

    error = otUdpConnect(&mSocket, &sockaddr);

exit:
    return error;
}

otError UdpExample::ProcessClose(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return otUdpClose(&mSocket);
}

otError UdpExample::ProcessOpen(int argc, char *argv[])
{
    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return otUdpOpen(mInterpreter.mInstance, &mSocket, HandleUdpReceive, this);
}

otError UdpExample::GetPayload(void)
{
    uint64_t timestamp = 0;

    mInterpreter.mServer->OutputFormat("1.+++++++++++++++++++++++++++++");
    otNetworkTimeGet(mInterpreter.mInstance, timestamp);

    memset(mPayload, 0, sizeof(mPayload));

    memcpy(mPayload, &timestamp, sizeof(timestamp));
    mInterpreter.mServer->OutputFormat("%d,%d,%d,%d,%d,%d,%d,%d\r\n", mPayload[0], mPayload[1],mPayload[2],mPayload[3],mPayload[4],mPayload[5],mPayload[6],mPayload[7]);

    for (uint16_t i = 8; i < mLength; i++)
    {
        mPayload[i] = 'T';
    }

    mPayload[mLength] = '\0';

    mInterpreter.mServer->OutputFormat("%d\r\n", timestamp);
    mInterpreter.mServer->OutputFormat("2.+++++++++++++++++++++++++++++");
    return OT_ERROR_NONE;

}

otError UdpExample::ProcessSend(int argc, char *argv[])
{
    otError       error;
    otMessageInfo messageInfo;
    otMessage *   message = NULL;
    int           curArg  = 0;

    memset(&messageInfo, 0, sizeof(messageInfo));

    VerifyOrExit(argc == 1 || argc == 3, error = OT_ERROR_INVALID_ARGS);
    // if send command with 3 parameters
    if (argc == 3)
    {
        long value;

        error = otIp6AddressFromString(argv[curArg++], &messageInfo.mPeerAddr);
        SuccessOrExit(error);

        error = Interpreter::ParseLong(argv[curArg++], value);
        SuccessOrExit(error);

        messageInfo.mPeerPort    = static_cast<uint16_t>(value);
        messageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
    }
    // create a message
    message = otUdpNewMessage(mInterpreter.mInstance, true);
    VerifyOrExit(message != NULL, error = OT_ERROR_NO_BUFS);

    // fill the message with data

    error = Interpreter::ParseLong(argv[curArg], mLength);

    SuccessOrExit(GetPayload());

    mInterpreter.mServer->OutputFormat("%s\r\n", mPayload);
    // error = otMessageAppend(message, argv[curArg], static_cast<uint16_t>(strlen(argv[curArg])));
    error = otMessageAppend(message, mPayload, static_cast<uint16_t>(mLength));
    SuccessOrExit(error);

    error = otUdpSend(&mSocket, message, &messageInfo);

exit:

    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }

    return error;
}

otError UdpExample::Process(int argc, char *argv[])
{
    otError error = OT_ERROR_PARSE;

    if (argc < 1)
    {
        ProcessHelp(0, NULL);
        error = OT_ERROR_INVALID_ARGS;
    }
    else
    {
        for (size_t i = 0; i < OT_ARRAY_LENGTH(sCommands); i++)
        {
            if (strcmp(argv[0], sCommands[i].mName) == 0)
            {
                error = (this->*sCommands[i].mCommand)(argc - 1, argv + 1);
                break;
            }
        }
    }
    return error;
}

void UdpExample::HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static_cast<UdpExample *>(aContext)->HandleUdpReceive(aMessage, aMessageInfo);
}

void UdpExample::HandleUdpReceive(otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    uint8_t buf[1500];
    int     length;
    uint64_t receiveTimestamp = 0;
    uint64_t sendTimestamp = 0;

    otNetworkTimeGet(mInterpreter.mInstance, receiveTimestamp);

    mInterpreter.mServer->OutputFormat("%d bytes from ", otMessageGetLength(aMessage) - otMessageGetOffset(aMessage));
    mInterpreter.mServer->OutputFormat(
        "%x:%x:%x:%x:%x:%x:%x:%x %d ", HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[0]),
        HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[1]), HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[2]),
        HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[3]), HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[4]),
        HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[5]), HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[6]),
        HostSwap16(aMessageInfo->mPeerAddr.mFields.m16[7]), aMessageInfo->mPeerPort);

    length      = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';

    // for (size_t i = mLength-8; i < mLength; i++)
    // {
    //     sendTimestamp |= (buf[i] << (mLength-i)*8);
    // }
    memcpy(&sendTimestamp, buf, sizeof(sendTimestamp));

    mLatency = receiveTimestamp - sendTimestamp;

    mInterpreter.mServer->OutputFormat("----------%d\r\n", mLatency);
    mInterpreter.mServer->OutputFormat("+++++++++++%d\r\n", receiveTimestamp);
    mInterpreter.mServer->OutputFormat("@@@@@@@@@@@%d\r\n", sendTimestamp);

    mInterpreter.mServer->OutputFormat("%s\r\n", buf);
    mInterpreter.mServer->OutputFormat("Done\r\n");
}

} // namespace Cli
} // namespace ot
