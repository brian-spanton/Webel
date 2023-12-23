// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Console.h"
#include "Basic.TextWriter.h"
#include "Basic.Hold.h"
#include "Basic.Globals.h"

namespace Basic
{
    Console::Console() :
        output(INVALID_HANDLE_VALUE),
        input(INVALID_HANDLE_VALUE)
    {
    }

    Console::~Console()
    {
        if (output != INVALID_HANDLE_VALUE)
            CloseHandle(output);

        if (input != INVALID_HANDLE_VALUE)
            CloseHandle(output);
    }

    void Console::Initialize(std::shared_ptr<ITransportEventHandler<Codepoint> > event_handler)
    {
        this->output = GetStdHandle(STD_OUTPUT_HANDLE);
        if (this->output == INVALID_HANDLE_VALUE)
        {
            // $$$ probably we are running as a service and have no console
            Basic::globals->HandleError("GetStdHandle", GetLastError());
            return;
        }

        this->input = GetStdHandle(STD_INPUT_HANDLE);
        if (this->input == INVALID_HANDLE_VALUE)
            throw FatalError("GetStdHandle", GetLastError());

        bool success = (bool)GetConsoleMode(this->input, &this->originalMode);
        if (!success)
            throw FatalError("GetConsoleMode", GetLastError());

        success = (bool)SetConsoleMode(this->input, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
        if (!success)
            throw FatalError("SetConsoleMode", GetLastError());

        this->event_handler = event_handler;
        this->event_handler->transport_connected();
    }

    bool Console::thread()
    {
        // if we don't have a console, don't read from it.
        if (this->input == INVALID_HANDLE_VALUE)
            return false;

        while (true)
        {
            INPUT_RECORD inputs[0x100];
            uint32 count;

            BOOL success = ReadConsoleInputW(this->input, inputs, _countof(inputs), &count);
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
                            Basic::globals->CRLF->write_to_stream(this);
                            this->event_handler->transport_received(&b, 1);
                            break;

                        default:
                            write_element(b);
                            this->event_handler->transport_received(&b, 1);
                            break;
                        }
                    }
                }
            }
        }

        return true;
    }

    void Console::write_element(Codepoint element)
    {
        // if we don't have a console, don't write to it.
        if (this->output == INVALID_HANDLE_VALUE)
            return;

        // $ hacky conversion from utf-32 to utf-16
        uint16 utf_16 = (uint16)element;

        BOOL success = WriteConsoleW(this->output, &utf_16, 1, 0, 0);
        if (success == FALSE)
            throw FatalError("WriteConsoleW", GetLastError());
    }
}