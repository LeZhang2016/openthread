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
 *  This file includes definitions for the border agent role.
 */

#ifndef BORDER_AGENT_HPP_
#define BORDER_AGENT_HPP_

#include <openthread-types.h>
#include <openthread-core-config.h>

#include <coap/coap_header.hpp>
#include <coap/coap_client.hpp>
#include <coap/coap_server.hpp>
#include <coap/secure_coap_server.hpp>
#include <common/message.hpp>
#include <net/udp6.hpp>
#include <meshcop/dtls.hpp>
#include <meshcop/tlvs.hpp>
#include <thread/mle.hpp>
#include <net/ip6_address.hpp>

namespace Thread {

class ThreadNetif;

namespace MeshCoP {

class BorderAgent
{
public:
    /**
     * This constructor initializes the Border Agent object.
     *
     * @param[in]  aThreadNetif  A reference to the Thread network interface.
     *
     */
    BorderAgent(ThreadNetif &aThreadNetif);

    /**
     * This method starts the Border Agent service.
     *
     * @retval kThreadError_None  Successfully started the Border Agent service.
     *
     */
    ThreadError Start(void);

    /*
     * This method stops the Border Agent service.
     *
     * @retval kThreadError_None  Successfully stopped the Border Agent service.
     */
     
    ThreadError Stop(void);

    ThreadError SetPSKc(const char *aPSKc, const int aPSKcLength);

private:

    static void HandleCommissionerPetition(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                           const otMessageInfo *aMessageInfo);
    void HandleCommissionerPetition(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    static void HandleLeaderPetitionResponse(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                             const otMessageInfo *aMessageInfo, ThreadError aResult);
    void HandleLeaderPetitionResponse(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo,
                                      ThreadError aResult);

    static void HandleCommisionerKeepAlive(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                           const otMessageInfo *aMessageInfo);
    void HandleCommisionerKeepAlive(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    static void HandleLeaderKeepAliveResponse(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                              const otMessageInfo *aMessageInfo, ThreadError aResult);
    void HandleLeaderKeepAliveResponse(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo,
                                       ThreadError aResult);

    static void HandleRelayTransmit(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                    const otMessageInfo *aMessageInfo);
    void HandleRelayTransmit(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    static void HandleMgmtCommissionerSet(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                          const otMessageInfo *aMessageInfo);
    void HandleMgmtCommissionerSet(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    static void HandleLeaderMgmtCommissionerSetResponse(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                                        const otMessageInfo *aMessageInfo, ThreadError aResult);
    void HandleLeaderMgmtCommissionerSetResponse(Coap::Header &aHeader, Message &aMessage,
                                                 const Ip6::MessageInfo &aMessageInfo, ThreadError aResult);

    static void HandleRelayReceive(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
                                   const otMessageInfo *aMessageInfo);
    void HandleRelayReceive(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    ThreadError SendLeaderPetition(Message &aMessage, const Ip6::MessageInfo &aMessageInfo);
    ThreadError SendLeaderKeepAlive(Message &aMessage, const Ip6::MessageInfo &aMessageInfo);
    ThreadError SendLeaderMgmtCommissionerSet(Message &aMessage, const Ip6::MessageInfo &aMessageInfo);
    ThreadError SendCommissionerRelayReceive(Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    Coap::Resource mRelayReceive;
    Coap::Resource mRelayTransmit;
    Coap::Resource mCommissionerPetition;
    Coap::Resource mCommissionerKeepAlive;
    Coap::Resource mMgmtCommissionerSet;

    uint8_t *mPSKc;
    uint8_t mPSKcLength;
    Ip6::Address mCommissionerAddr;
    uint16_t mCommissionerUdpPort;
    Ip6::Address mJoinerRouterAddr;
    uint16_t mJoinerRouterUdpPort;
    ThreadNetif &mNetif;
    uint8_t mToken[8];
    uint8_t mTokenLength;


};

}  // namespace MeshCoP
}  // namespace Thread

#endif  // BORDER_AGENT_HPP_