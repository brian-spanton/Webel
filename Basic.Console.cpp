// Copyright � 2013 Brian Spanton

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
        (*createdThread) = 0;

        this->protocol = protocol;

        output = GetStdHandle(STD_OUTPUT_HANDLE);
        if (output == INVALID_HANDLE_VALUE)
            throw FatalError("Basic", "Console::Initialize GetStdHandle failed", GetLastError());

        input = GetStdHandle(STD_INPUT_HANDLE);
        if (input == INVALID_HANDLE_VALUE)
            throw FatalError("Basic", "Console::Initialize GetStdHandle failed", GetLastError());

        bool success = (bool)GetConsoleMode(input, &originalMode);
        if (!success)
            throw FatalError("Basic", "Console::Initialize GetConsoleMode failed", GetLastError());

        success = (bool)SetConsoleMode(input, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);
        if (!success)
            throw FatalError("Basic", "Console::Initialize SetConsoleMode failed", GetLastError());

        CanSendCodepointsEvent event;
        event.Initialize(&this->protocol_element_source);
        process_event_ignore_failures(this->protocol.get(), &event);

        HANDLE thread = ::CreateThread(0, 0, Thread, this, 0, 0);
        if (thread == 0)
            throw FatalError("Basic", "Console::Initialize CreateThread failed", GetLastError());

        (*createdThread) = thread;
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
            INPUT_RECORD record;
            uint32 count;

            BOOL success = ReadConsoleInputW(input, &record, 1, &count);
            if (success == FALSE)
                throw FatalError("Basic", "Console::Thread ReadConsoleInputA failed", GetLastError());

            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown == TRUE)
            {
                for (uint16 x = 0; x < record.Event.KeyEvent.wRepeatCount; x++)
                {
                    Codepoint b = record.Event.KeyEvent.uChar.UnicodeChar;

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
                            process_event_ignore_failures(this->protocol.get(), &event);
                        }
                        break;

                    default:
                        {
                            write_element(b);

                            this->protocol_element_source.Initialize(&b, 1);

                            ReceivedCodepointsEvent event;
                            event.Initialize(&this->protocol_element_source);
                            process_event_ignore_failures(this->protocol.get(), &event);
                        }
                        break;
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

        // $$ switch to converting to UTF-16 and using WriteConsoleW

        ByteString ascii_elements;

        SingleByteEncoder encoder;
        encoder.Initialize(Basic::globals->ascii_index, &ascii_elements);
        encoder.write_elements(elements, count);

        BOOL success = WriteConsoleA(output, (char*)ascii_elements.c_str(), ascii_elements.size(), 0, 0);
        if (success == FALSE)
            throw FatalError("Basic", "Console::write_elements WriteConsoleA", GetLastError());
    }

    void Console::write_eof()
    {
        throw FatalError("Basic", "Console::write_eof");
    }
}