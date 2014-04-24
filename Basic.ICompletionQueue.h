// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IRefCounted.h"
#include "Basic.Socket.h"
#include "Basic.LogFile.h"
#include "Basic.ICompletion.h"
#include "Basic.IProcess.h"

namespace Basic
{
    __interface ICompletionQueue : public IRefCounted
    {
        void BindToCompletionQueue(Socket::Ref socket);
        void BindToCompletionQueue(LogFile::Ref socket);
        void PostCompletion(Basic::ICompletion* completion, LPOVERLAPPED overlapped);
        void QueueProcess(Basic::Ref<IProcess> process, ByteString::Ref cookie);
    };
}