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
 *   This file implements the CoAP server message dispatch.
 */

#include <coap/coap_server.hpp>
#include <common/code_utils.hpp>
#include <common/logging.hpp>

namespace Thread {
namespace Coap {

Server::Server(Ip6::Udp &aUdp, uint16_t aPort, SenderFunction aSender, ReceiverFunction aReceiver):
    CoapBase(aUdp, aSender, aReceiver)
{
    mPort = aPort;
    mResources = NULL;
}

ThreadError Server::Start(void)
{
    Ip6::SockAddr sockaddr;
    sockaddr.mPort = mPort;

    return CoapBase::Start(sockaddr);
}

ThreadError Server::Stop(void)
{
    return CoapBase::Stop();
}

ThreadError Server::AddResource(Resource &aResource)
{
    ThreadError error = kThreadError_None;
    char a[255];
    uint16_t idx = 0;

    for (Resource *cur = mResources; cur; cur = cur->GetNext())
    {
        for (uint16_t i = 0; i< strlen(cur->mUriPath); i++)
        {
            a[idx++] = cur->mUriPath[i];
        }
        a[idx++] = ' ';
        VerifyOrExit(cur != &aResource, error = kThreadError_Already);
    }
    a[idx++] = '\0';
    otLogCritMeshCoP("1. %s, 2.%s",a, aResource.mUriPath);

    aResource.mNext = mResources;
    mResources = &aResource;

exit:
    return error;
}

void Server::RemoveResource(Resource &aResource)
{
    if (mResources == &aResource)
    {
        mResources = aResource.GetNext();
    }
    else
    {
        for (Resource *cur = mResources; cur; cur = cur->GetNext())
        {
            if (cur->mNext == &aResource)
            {
                cur->mNext = aResource.mNext;
                ExitNow();
            }
        }
    }

exit:
    aResource.mNext = NULL;
}

Message *Server::NewMessage(uint16_t aReserved)
{
    return mSocket.NewMessage(aReserved);
}

Message *Server::NewMeshCoPMessage(uint16_t aReserved)
{
    Message *message = NULL;

    VerifyOrExit((message = NewMessage(aReserved)) != NULL, ;);

    message->SetPriority(kMeshCoPMessagePriority);

exit:
    return message;
}

ThreadError Server::SendMessage(Message &aMessage, const Ip6::MessageInfo &aMessageInfo)
{
    return mSender(this, aMessage, aMessageInfo);
}

ThreadError Server::SendEmptyAck(const Header &aRequestHeader, const Ip6::MessageInfo &aMessageInfo)
{
    ThreadError error = kThreadError_None;
    Coap::Header responseHeader;
    Message *message = NULL;

    VerifyOrExit(aRequestHeader.GetType() == kCoapTypeConfirmable, error = kThreadError_InvalidArgs);

    responseHeader.SetDefaultResponseHeader(aRequestHeader);

    VerifyOrExit((message = NewMessage(responseHeader)) != NULL, error = kThreadError_NoBufs);

    SuccessOrExit(error = SendMessage(*message, aMessageInfo));

exit:

    if (error != kThreadError_None && message != NULL)
    {
        message->Free();
    }

    return error;
}

void Server::ProcessReceivedMessage(Message &aMessage, const Ip6::MessageInfo &aMessageInfo)
{
    Header header;
    char uriPath[Resource::kMaxReceivedUriPath] = "";
    char *curUriPath = uriPath;
    const Header::Option *coapOption;
    SuccessOrExit(header.FromMessage(aMessage, false));
    aMessage.MoveOffset(header.GetLength());
    coapOption = header.GetCurrentOption();

    while (coapOption != NULL)
    {
        otLogCritMeshCoP(">>>>>>>>  00 %d", coapOption->mNumber);
        switch (coapOption->mNumber)
        {
        case kCoapOptionUriPath:
            if (curUriPath != uriPath)
            {
                *curUriPath++ = '/';
            }

            VerifyOrExit(coapOption->mLength < sizeof(uriPath) - static_cast<size_t>(curUriPath + 1 - uriPath), ;);
            memcpy(curUriPath, coapOption->mValue, coapOption->mLength);
            otLogCritMeshCoP(">>>>>>>>  33 %s", curUriPath);
            curUriPath += coapOption->mLength;
            break;

        case kCoapOptionContentFormat:
            break;

        case 17:
            break;

        default:
            otLogCritMeshCoP(">>>>>>>>  44");
            ExitNow();
        }

        coapOption = header.GetNextOption();
    }

    curUriPath[0] = '\0';

    otLogCritMeshCoP(">>>>>>>>uriPath is %s", uriPath);
    for (Resource *resource = mResources; resource; resource = resource->GetNext())
    {
        if (strcmp(resource->mUriPath, uriPath) == 0)
        {
            resource->HandleRequest(header, aMessage, aMessageInfo);
            ExitNow();
        }
    }

exit:
     otLogCritMeshCoP(">>>>>>>>>>>>>>>exit");
    {}
}

ThreadError Server::SetPort(uint16_t aPort)
{
    mPort = aPort;

    Ip6::SockAddr sockaddr;
    sockaddr.mPort = mPort;

    return mSocket.Bind(sockaddr);
}

}  // namespace Coap
}  // namespace Thread
