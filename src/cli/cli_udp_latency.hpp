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
 *   This file contains definitions for a simple CLI CoAP server and client.
 */

#ifndef CLI_UDP_LATENCY_HPP_
#define CLI_UDP_LATENCY_HPP_
// #include "cli.hpp"
#include <openthread/udp.h>
#include <openthread/types.h>

#include "common/timer.hpp"

namespace ot {
namespace Cli {

class Interpreter;

/**
 * This class implements a CLI-based UDP example.
 *
 */
class UdpLatency
{
public:
    /**
     * Constructor
     *
     * @param[in]  aInterpreter  The CLI interpreter.
     *
     */
    UdpLatency(Interpreter &aInterpreter);

    /**
     * This method interprets a list of CLI arguments.
     *
     * @param[in]  argc  The number of elements in argv.
     * @param[in]  argv  A pointer to an array of command line arguments.
     *
     */
    otError Process(int argc, char *argv[]);

private:
    struct Command
    {
        const char *mName;
        otError(UdpLatency::*mCommand)(int argc, char *argv[]);
    };

    otError ProcessHelp(int argc, char *argv[]);
    otError ProcessBind(int argc, char *argv[]);
    otError ProcessClose(int argc, char *argv[]);
    otError ProcessConnect(int argc, char *argv[]);
    otError ProcessOpen(int argc, char *argv[]);
    otError ProcessSend(int argc, char *argv[]);
    otError ProcessTest(int argc, char *argv[]);
    
    void Init(void);
    otError SendUDPPacket(void);
    uint32_t ReadSequence(otMessage *aMessage);
    uint32_t ReadTimestamp(otMessage *aMessage);

    static void s_HandlePingTimer(Timer &aTimer);
    void HandlePingTimer();
    static UdpLatency &GetOwner(const Context &aContext);

    static void HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
    void HandleUdpReceive(otMessage *aMessage, const otMessageInfo *aMessageInfo);

    static const Command sCommands[];
    Interpreter &mInterpreter;

    otUdpSocket mSocket;

    uint16_t mLength;
    uint16_t mCount;
    uint16_t mSequence;
    uint32_t mInterval;
    char mPayload[2048];
    otMessageInfo mMessageInfo;
    otMessage *mMessage = NULL;
    TimerMilli mPingTimer;
    uint32_t mTimestamp;
    uint8_t mRole;
    uint32_t mLossNum;
    bool mIsReceived;
    uint32_t mLatency;
    uint32_t mTimeElapse;
};

}  // namespace Cli
}  // namespace ot

#endif  // CLI_UDP_EXAMPLE_HPP_
