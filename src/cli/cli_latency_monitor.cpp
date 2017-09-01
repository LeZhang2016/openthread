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

#include "cli_latency_monitor.hpp"

#include <openthread/config.h>
#include <openthread/message.h>
#include <openthread/udp.h>

#include "cli/cli.hpp"
#include "cli_uart.hpp"
#include "common/encoding.hpp"

using ot::Encoding::BigEndian::HostSwap16;

namespace ot {
namespace Cli {

// static bool sIsAccept = false;
LatencyMonitor::LatencyMonitor(Interpreter &aInterpreter):
    mInterpreter(aInterpreter)
    {
    }


const struct LatencyMonitor::Command LatencyMonitor::sCommands[] =
{
    { "help", &LatencyMonitor::ProcessHelp },
    { "open", &LatencyMonitor::ProcessOpen }
};


void LatencyMonitor::Init()
{
    for (uint8_t i = 0; i < sizeof(mPinNumbers)/mPinNumbers[0]; i++)
    {
        mPinNumbers[i] = 0;
    }
}

otError LatencyMonitor::ProcessHelp(int argc, char *argv[])
{
    for (unsigned int i = 0; i < sizeof(sCommands) / sizeof(sCommands[0]); i++)
    {
        mInterpreter.mServer->OutputFormat("%s\r\n", sCommands[i].mName);
    }

    OT_UNUSED_VARIABLE(argc);
    OT_UNUSED_VARIABLE(argv);

    return OT_ERROR_NONE;
}


otError LatencyMonitor::ProcessOpen(int argc, char *argv[])
{
    otError error;

    VerifyOrExit(argc >= 1, error = OT_ERROR_PARSE);
    otGpioInit();
    for (int i = 0; i < argc; i++)
    {
        long value;
        error = Interpreter::ParseLong(argv[i], value);
        mPinNumbers[i] = static_cast<uint32_t>(value);
        otPlatGpioCfgInput(mPinNumbers[i]);
    }


exit:
    return error;
}


otError LatencyMonitor::Process(int argc, char *argv[])
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


LatencyMonitor &LatencyMonitor::GetOwner(const Context &aContext)
{
#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
    LatencyMonitor &udp = *static_cast<LatencyMonitor *>(aContext.GetContext());
#else
    LatencyMonitor &udp = Uart::sUartServer->GetInterpreter().mLatencyMonitor;
    OT_UNUSED_VARIABLE(aContext);
#endif
    return udp;
}


extern "C" void otPlatGpioSignalEvent(uint32_t aPinIndex)
{
    aPinIndex++;
}

}  // namespace Cli
}  // namespace ot
