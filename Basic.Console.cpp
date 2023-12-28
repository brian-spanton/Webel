// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Console.h"
#include "Basic.TextWriter.h"
#include "Basic.Hold.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"
#include "Basic.String.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    Console::Console()
    {
    }

    Console::~Console()
    {
        if (output != INVALID_HANDLE_VALUE)
            CloseHandle(output);

        if (input != INVALID_HANDLE_VALUE)
            CloseHandle(input);
    }

    void Console::Initialize(std::shared_ptr<IProcess> protocol, HANDLE* createdThread)
    {
        TryInitialize(protocol, createdThread);
    }

    bool Console::TryInitialize(std::shared_ptr<IProcess> protocol, HANDLE* createdThread)
    {
        (*createdThread) = 0;

        this->protocol = protocol;

        output = GetStdHandle(STD_OUTPUT_HANDLE);
        if (output == INVALID_HANDLE_VALUE)
            return Basic::globals->HandleError("GetStdHandle", GetLastError());

        input = GetStdHandle(STD_INPUT_HANDLE);
        if (input == INVALID_HANDLE_VALUE)
            return Basic::globals->HandleError("GetStdHandle", GetLastError());

        bool success = (bool)GetConsoleMode(input, &originalMode);
        if (!success)
            return Basic::globals->HandleError("GetConsoleMode", GetLastError());

        success = (bool)SetConsoleMode(input, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
        if (!success)
            return Basic::globals->HandleError("SetConsoleMode", GetLastError());

        CanSendCodepointsEvent event;
        event.Initialize(&this->protocol_element_source);
        produce_event(this->protocol.get(), &event);

        HANDLE thread = ::CreateThread(0, 0, Thread, this, 0, 0);
        if (thread == 0)
            throw FatalError("CreateThread", GetLastError());

        (*createdThread) = thread;
        return true;
    }

    DWORD WINAPI Console::Thread(void* param)
    {
        Console* console = reinterpret_cast<Console*>(param);
        bool success = console->Thread();
        if (!success)
            return ERROR_ERRORS_ENCOUNTERED;

        return ERROR_SUCCESS;
    }

    bool Console::Thread()
    {
        while (true)
        {
            INPUT_RECORD inputs[0x100];
            uint32 count;

            BOOL success = ReadConsoleInputW(input, inputs, _countof(inputs), &count);
            if (success == FALSE)
                return Basic::globals->HandleError("ReadConsoleInputA", GetLastError());

            for (uint32 i = 0; i < count; i++)
            {
                if (inputs[i].EventType == KEY_EVENT && inputs[i].Event.KeyEvent.bKeyDown == TRUE)
                {
                    for (uint16 x = 0; x < inputs[i].Event.KeyEvent.wRepeatCount; x++)
                    {
                        Codepoint b = inputs[x].Event.KeyEvent.uChar.UnicodeChar;

                        switch (b)
                        {
                        case 0:
                            // just a control key probably
                            break;

                        case '\r':
                            {
                                write_elements(Basic::globals->CRLF->address(), Basic::globals->CRLF->size());

                                this->protocol_element_source.Initialize(&b, 1);

                                ReceivedCodepointsEvent event;
                                event.Initialize(&this->protocol_element_source);
                                produce_event(this->protocol.get(), &event);
                            }
                            break;

                        default:
                            {
                                write_element(b);

                                this->protocol_element_source.Initialize(&b, 1);

                                ReceivedCodepointsEvent event;
                                event.Initialize(&this->protocol_element_source);
                                produce_event(this->protocol.get(), &event);
                            }
                            break;
                        }
                    }
                }
            }
        }

        return true;
    }

    void Console::write_elements(const Codepoint* elements, uint32 count)
    {
        // if we don't have a console, don't log it.
        if (output == INVALID_HANDLE_VALUE)
            return;

        // $$$ switch to support UTF-16 using WriteConsoleW

        ByteString ascii_elements;

        SingleByteEncoder encoder;
        encoder.Initialize(Basic::globals->ascii_index, &ascii_elements);
        encoder.write_elements(elements, count);

        BOOL success = WriteConsoleA(output, (char*)ascii_elements.c_str(), ascii_elements.size(), 0, 0);
        if (success == FALSE)
            throw FatalError("WriteConsoleW", GetLastError());
    }

    void Console::write_eof()
    {
        HandleError("unexpected eof");
    }
}