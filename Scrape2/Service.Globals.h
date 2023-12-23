// Copyright © 2013 Brian Spanton

#pragma once

#include <Windows.h>
#include "Basic.IJobEventHandler.h"
#include "Basic.LogStream.h"
#include "Basic.ListenSocket.h"
#include "Basic.Cng.h"
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

    class Globals : public StateMachine, public IThread, public IJobEventHandler, public ICompletionQueue, public Tls::ICertificate, public std::enable_shared_from_this<Globals>
    {
    private:
        enum State
        {
            cert_pending_state = Start_State,
            encodings_pending_state,
            named_character_references_pending_state,
            accepts_pending_state,
            done_state = Succeeded_State,
        };

        typedef std::vector<HANDLE> ThreadList;

        HANDLE queue;
        HANDLE stopEvent;
        HANDLE consoleThread;
        HANDLE pfx_file;
        ThreadList threads;
        Basic::PCCERT_CONTEXT cert;
        Basic::NCRYPT_KEY_HANDLE private_key;
        Tls::Certificates certificates;

        static DWORD WINAPI Thread(void* param);

        bool ParseCert(ByteString* bytes, uint32 count, uint32 error);
        bool ReadCertificate();
        bool CreateSelfSignCert();
        bool ExtractPrivateKey();

        void consider_event(void* event);

    public:
        typedef Basic::StringMapCaseInsensitive<std::shared_ptr<Basic::ListenSocket> > Listeners;
        typedef std::vector<Basic::UnicodeStringRef> CommandList;

        std::shared_ptr<Basic::Console> console;
        std::shared_ptr<Basic::FileLog> file_log;
        std::shared_ptr<Basic::MemoryLog> memory_log;
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
		Basic::UnicodeStringRef command_amazon;
		Basic::UnicodeStringRef command_netflix;

        CommandList command_list;

        Basic::UnicodeStringRef root_admin;
        Basic::UnicodeStringRef root_echo;
        Basic::UnicodeStringRef root_question;
        Basic::UnicodeStringRef root_log;

        std::shared_ptr<WebServerEndpoint> http_endpoint;
        std::shared_ptr<WebServerEndpoint> https_endpoint;
        std::shared_ptr<FtpServerEndpoint> ftp_control_endpoint;

        Globals();
        ~Globals();

        virtual void IJobEventHandler::job_completed(std::shared_ptr<void> context, uint32 count, uint32 error);
        virtual bool IThread::thread();

        bool Initialize();
        void WaitForStopSignal();
        void Cleanup();
        bool SendStopSignal();
        bool SetThreadCount(uint32 count);

        template <int value_count>        
        void GetFilePath(const char* name, char (&value)[value_count])
        {
            char exe_path[MAX_PATH + 0x100];

            DWORD exe_path_count = GetModuleFileNameA(0, exe_path, sizeof(exe_path));
            if (exe_path_count == sizeof(exe_path))
                throw FatalError("GetModuleFileNameA", GetLastError());

            char drive[MAX_PATH + 0x100];
            char directory[MAX_PATH + 0x100];

            _splitpath_s(exe_path, drive, sizeof(drive), directory, sizeof(directory), 0, 0, 0, 0);

            strcpy_s(value, drive);
            strcat_s(value, directory);
            strcat_s(value, name);
        }

        virtual void ICompletionQueue::BindToCompletionQueue(Socket* socket);
        virtual void ICompletionQueue::BindToCompletionQueue(FileLog* log_file);
        virtual void ICompletionQueue::QueueJob(std::shared_ptr<Job> job);

        virtual bool ICertificate::CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult);
        virtual Tls::Certificates* ICertificate::Certificates();
    };

    extern Globals* globals;
}