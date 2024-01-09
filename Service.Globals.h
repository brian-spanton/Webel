// Copyright © 2013 Brian Spanton

#pragma once

#include <Windows.h>
#include "Basic.ICompleter.h"
#include "Basic.LogStream.h"
#include "Basic.ListenSocket.h"
#include "Basic.Cng.h"
#include "Basic.Frame.h"
#include "Basic.Uri.h"
#include "Basic.Console.h"
#include "Service.AdminProtocol.h"
#include "Service.Types.h"
#include "Basic.ICompletionQueue.h"
#include "Tls.ICertificate.h"
#include "Service.WebServerEndpoint.h"
#include "Service.WebProxyEndpoint.h"
#include "Service.FtpServerEndpoint.h"
#include "Basic.ConsoleLog.h"
#include "Basic.DebugLog.h"
#include "Basic.FileLog.h"
#include "Basic.MemoryLog.h"
#include "Basic.TailLog.h"

namespace Service
{
    using namespace Basic;

    class Globals : public Frame, public ICompleter, public ILogger, public ICompletionQueue, public Tls::ICertificate, public std::enable_shared_from_this<Globals>
    {
    private:
        enum State
        {
            initialize_test_gzip_state = Start_State,
            pending_test_gzip_completion_state,
            initialize_read_certificate_state,
            pending_read_certificate_state,
            initialize_self_sign_certificate_state,
            initialize_encodings_state,
            pending_encodings_state,
            initialize_html_globals_state,
            pending_html_globals_state,
            accepts_pending_state,
            done_state = Succeeded_State,
        };

        typedef std::vector<HANDLE> ThreadList;

        HANDLE queue;
        HANDLE stopEvent;
        HANDLE consoleThread;
        HANDLE pfx_file;
        HANDLE gzip_test_file;
        ThreadList threads;
        Basic::PCCERT_CONTEXT cert = 0;
        Basic::NCRYPT_KEY_HANDLE private_key;
        Tls::Certificates certificates;

        static DWORD WINAPI Thread(void* param);
        bool ParseCert(ByteString* bytes, uint32 count, uint32 error);
        bool ReadCertificate();
        bool CreateSelfSignCert();
        bool ExtractPrivateKey();

        bool TestHuffman();
        bool TestGzip();

        virtual ProcessResult IProcess::process_event(IEvent* event);

    public:
        typedef Basic::StringMapCaseInsensitive<std::shared_ptr<Basic::ListenSocket> > Listeners;

        std::shared_ptr<Basic::Console> console;
        std::shared_ptr<Basic::FileLog> file_log;
        std::shared_ptr<Basic::TailLog> tail_log;
        std::shared_ptr<Basic::ConsoleLog> console_log;
        std::shared_ptr<Basic::DebugLog> debug_log;

        std::string service_name;
        std::string self_sign_domain;
        std::string certificate_file_name;
        std::wstring certificate_file_password;

        Basic::UnicodeStringRef command_stop;
        Basic::UnicodeStringRef command_log;
        Basic::UnicodeStringRef command_get;
        Basic::UnicodeStringRef command_follow_link;
        Basic::UnicodeStringRef command_select_form;
        Basic::UnicodeStringRef command_set_control_value;
        Basic::UnicodeStringRef command_submit;
        Basic::UnicodeStringRef command_render_links;
        Basic::UnicodeStringRef command_render_forms;
        Basic::UnicodeStringRef command_render_nodes;
        Basic::UnicodeStringRef command_search;

        Basic::UnicodeStringRef root_admin;
        Basic::UnicodeStringRef root_echo;
        Basic::UnicodeStringRef root_question;
        Basic::UnicodeStringRef root_log;

        UnicodeStringRef title_property;
        UnicodeStringRef as_of_property;
        UnicodeStringRef source_property;

        std::shared_ptr<Index> index;

        std::shared_ptr<WebServerEndpoint> http_endpoint;
        std::shared_ptr<WebServerEndpoint> https_endpoint;
        std::shared_ptr<FtpServerEndpoint> ftp_control_endpoint;

        Globals();
        ~Globals();

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);

        void Initialize();
        void WaitForStopSignal();
        void Cleanup();
        void SendStopSignal();
        bool Thread();
        void SetThreadCount(uint32 count);

        template <int value_count>        
        void GetFilePath(const char* name, char (&value)[value_count])
        {
            char exe_path[MAX_PATH + 0x100];

            DWORD exe_path_count = GetModuleFileNameA(0, exe_path, sizeof(exe_path));
            if (exe_path_count == sizeof(exe_path))
                throw FatalError("Service", "Globals::GetFilePath GetModuleFileNameA { exe_path_count == sizeof(exe_path) } failed", GetLastError());

            char drive[MAX_PATH + 0x100];
            char directory[MAX_PATH + 0x100];

            _splitpath_s(exe_path, drive, sizeof(drive), directory, sizeof(directory), 0, 0, 0, 0);

            strcpy_s(value, drive);
            strcat_s(value, directory);
            strcat_s(value, name);
        }

        virtual void ILogger::Log(LogLevel level, const char* component, const char* context, uint32 code);
        virtual Basic::IStream<Codepoint>* ILogger::LogStream();
        virtual Basic::TextWriter* ILogger::DebugWriter();

        virtual void ICompletionQueue::BindToCompletionQueue(HANDLE handle);
        virtual void ICompletionQueue::QueueJob(std::shared_ptr<Job> job);

        virtual bool ICertificate::CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult);
        virtual Tls::Certificates* ICertificate::Certificates();
    };

    extern Globals* globals;
}