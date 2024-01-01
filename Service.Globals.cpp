// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Service.Globals.h"
#include "Basic.Globals.h"
#include "Tls.Globals.h"
#include "Json.Globals.h"
#include "Html.Globals.h"
#include "Http.Globals.h"
#include "Basic.IStream.h"
#include "Service.AdminProtocol.h"
#include "Web.Server.h"
#include "Web.Proxy.h"
#include "Html.ElementNode.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.SuffixArray.h"
#include "Service.StandardEncodings.h"
#include "Tls.ICertificate.h"
#include "Service.WebServer.h"
#include "Service.HtmlNamedCharacterReferences.h"
#include "Web.Globals.h"
#include "Ftp.Globals.h"
#include "Scrape.Globals.h"

template <typename type>
void make_immortal(type** pointer, std::shared_ptr<type>* ref)
{
    // create a ref that will never destruct
    std::shared_ptr<type>* heap_ref = new std::shared_ptr<type>();

    // create the global instance held by the ref that never destructs
    (*heap_ref) = std::make_shared<type>();

    // return the address
    (*pointer) = heap_ref->get();

    if (ref != 0)
        (*ref) = (*heap_ref);
}

namespace Service
{
    Globals* globals = 0;

    __declspec(thread) Basic::IStream<Codepoint>* debug_stream = 0;
    __declspec(thread) Basic::TextWriter* debug_writer = 0;

    DWORD WINAPI Globals::Thread(void* param)
    {
        Globals* process = reinterpret_cast<Globals*>(param);
        bool success = process->Thread();
        if (!success)
            return ERROR_ERRORS_ENCOUNTERED;

        return ERROR_SUCCESS;
    }

    Globals::Globals() :
        queue(0),
        stopEvent(0)
    {
        // these get used before Globals::Initialize is called

        this->console_log = std::make_shared<Basic::ConsoleLog>();
        this->debug_log = std::make_shared<Basic::DebugLog>();
        this->memory_log = std::make_shared<Basic::MemoryLog>();
        this->tail_log = std::make_shared<Basic::TailLog>();
    }

    Globals::~Globals()
    {
        if (queue != 0)
            CloseHandle(queue);

        if (stopEvent != 0)
            CloseHandle(stopEvent);
    }

    bool Globals::Initialize()
    {
        make_immortal<Tls::Globals>(&Tls::globals, 0);
        Tls::globals->Initialize();

        make_immortal<Ftp::Globals>(&Ftp::globals, 0);
        Ftp::globals->Initialize();

        make_immortal<Http::Globals>(&Http::globals, 0);
        Http::globals->Initialize();

        make_immortal<Html::Globals>(&Html::globals, 0);
        Html::globals->Initialize();

        make_immortal<Json::Globals>(&Json::globals, 0);
        Json::globals->Initialize();

        make_immortal<Web::Globals>(&Web::globals, 0);
        Web::globals->Initialize();

        make_immortal<Scrape::Globals>(&Scrape::globals, 0);
        Scrape::globals->Initialize();

        initialize_unicode(&command_stop, "stop");
        command_list.push_back(command_stop);

        initialize_unicode(&command_log, "log");
        command_list.push_back(command_log);

        initialize_unicode(&command_get, "get");
        command_list.push_back(command_get);

        initialize_unicode(&command_follow_link, "link");
        command_list.push_back(command_follow_link);

        initialize_unicode(&command_select_form, "form");
        command_list.push_back(command_select_form);

        initialize_unicode(&command_set_control_value, "control");
        command_list.push_back(command_set_control_value);

        initialize_unicode(&command_submit, "submit");
        command_list.push_back(command_submit);

        initialize_unicode(&command_render_links, "links");
        command_list.push_back(command_render_links);

        initialize_unicode(&command_render_forms, "forms");
        command_list.push_back(command_render_forms);

        initialize_unicode(&command_render_nodes, "nodes");
        command_list.push_back(command_render_nodes);

        initialize_unicode(&command_search, "search");
        command_list.push_back(command_search);

		initialize_unicode(&command_amazon, "amazon");
		command_list.push_back(command_amazon);

		initialize_unicode(&command_netflix, "netflix");
		command_list.push_back(command_netflix);

        initialize_unicode(&root_admin, "admin");
        initialize_unicode(&root_echo, "echo");
        initialize_unicode(&root_question, "question");
        initialize_unicode(&root_log, "log");

        DebugWriter()->WriteLine("initializing io completion port");

        this->queue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
        if (this->queue == 0)
            return Basic::globals->HandleError("CreateIoCompletionPort", GetLastError());

        DebugWriter()->WriteLine("initializing log file");

        char log_path[MAX_PATH + 0x100];
        GetFilePath("service.log", log_path);

        this->file_log = std::make_shared<Basic::FileLog>();
        this->file_log->Initialize(log_path);

        DebugWriter()->WriteLine("initializing stop event");

        this->stopEvent = CreateEvent(0, TRUE, FALSE, 0);
        if (this->stopEvent == 0)
            return Basic::globals->HandleError("CreateEvent", GetLastError());

        DebugWriter()->WriteLine("initializing console");

        this->console = std::make_shared<Basic::Console>();
        this->console->Initialize(std::make_shared<AdminProtocol>(this->console), &this->consoleThread);

        DebugWriter()->WriteLine("initializing socket library");

        bool success = Basic::globals->InitializeSocketApi();
        if (!success)
            return false;

        DebugWriter()->WriteLine("initializing certificate");

        if (!Service::globals->certificate_file_name.empty())
        {
            success = ReadCertificate();
        }
        else
        {
            success = false;
        }

        if (!success)
        {
            success = CreateSelfSignCert();
            if (!success)
                return false;
        }

        SetThreadCount(0); // 1 is good for debugging, 0 is good for perf (matches CPU count)

        return true;
    }

    bool Globals::ReadCertificate()
    {
        char pfx_path[MAX_PATH + 0x100];
        GetFilePath(Service::globals->certificate_file_name.c_str(), pfx_path);

        DebugWriter()->write_literal("reading ");
        DebugWriter()->WriteLine(pfx_path);

        this->pfx_file = ::CreateFileA(
            pfx_path,
            GENERIC_READ,
            FILE_SHARE_READ,
            0,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            0);
        if (this->pfx_file == INVALID_HANDLE_VALUE)
            return Basic::globals->HandleError("CreateFileA", GetLastError());

        HANDLE result = CreateIoCompletionPort(this->pfx_file, this->queue, reinterpret_cast<ULONG_PTR>(static_cast<ICompleter*>(this)), 0);
        if (result == 0)
            return Basic::globals->HandleError("CreateIoCompletionPort", GetLastError());

        LARGE_INTEGER size;
        bool success = (bool)GetFileSizeEx(this->pfx_file, &size);
        if (!success)
            return Basic::globals->HandleError("GetFileSizeEx", GetLastError());

        if (size.HighPart > 0)
            return Basic::globals->HandleError("GetFileSizeEx returned unexpectedly large size", 0);

        std::shared_ptr<ByteString> pfx_data = std::make_shared<ByteString>();
        pfx_data->resize(size.LowPart);

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), pfx_data);

        success = (bool)ReadFile(this->pfx_file, pfx_data->address(), pfx_data->size(), 0, job.get());
        if (!success)
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING)
            {
                job->Internal = error;
                Service::globals->QueueJob(job);
            }
        }

        return true;
    }

    void Globals::WaitForStopSignal()
    {
        WaitForSingleObject(stopEvent, INFINITE);
    }

    void Globals::Cleanup()
    {
        DebugWriter()->WriteLine("exiting");

        WaitForMultipleObjectsEx(threads.size(), &threads.front(), true, 30000, false);

        for (ThreadList::iterator it = threads.begin(); it != threads.end(); it++)
        {
            TerminateThread(*it, 0);
            CloseHandle(*it);
        }

        CloseHandle(this->consoleThread);

        int error = WSACleanup();
        if (error == SOCKET_ERROR)
            Basic::globals->HandleError("WSACleanup", WSAGetLastError());
    }

    void Globals::complete(std::shared_ptr<void> context, uint32 count, uint32 error)
    {
        std::shared_ptr<ByteString> bytes = std::static_pointer_cast<ByteString>(context);

        bool success = ParseCert(bytes.get(), count, error);
        if (!success)
        {
            success = CreateSelfSignCert();
            if (!success)
            {
                SendStopSignal();
            }
        }
    }

    bool Globals::ExtractPrivateKey()
    {
        DWORD keySpec;
        BOOL free;

        bool success = (bool)CryptAcquireCertificatePrivateKey(
            this->cert,
            CRYPT_ACQUIRE_SILENT_FLAG | CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG,
            0,
            &this->private_key,
            &keySpec,
            &free);
        if (!success)
            return Basic::globals->HandleError("handle_event::main CryptAcquireCertificatePrivateKey", GetLastError());

        if (!free)
            return Basic::globals->HandleError("handle_event::main !free", 0);

        if (keySpec != CERT_NCRYPT_KEY_SPEC)
            return Basic::globals->HandleError("handle_event::main != CERT_NCRYPT_KEY_SPEC", 0);

        this->certificates.resize(1);

        this->certificates[0].resize(this->cert->cbCertEncoded);
        CopyMemory(this->certificates[0].address(), this->cert->pbCertEncoded, this->cert->cbCertEncoded);

        DebugWriter()->WriteLine("initializing encodings");

        switch_to_state(State::encodings_pending_state);

        std::shared_ptr<StandardEncodings> standard_encodings = std::make_shared<StandardEncodings>(this->shared_from_this(), ByteStringRef());
        standard_encodings->start();

        return true;
    }

    bool Globals::CreateSelfSignCert()
    {
        byte name[0x100];
        uint32 count = _countof(name);

        std::string x_500;

        x_500 = "CN=";
        x_500 += this->self_sign_domain;

        DebugWriter()->write_literal("creating transient self-sign certificate ");
        DebugWriter()->WriteLine(x_500.c_str());

        bool success = (bool)CertStrToNameA(X509_ASN_ENCODING, x_500.c_str(), 0, 0, name, &count, 0);
        if (!success)
            return Basic::globals->HandleError("handle_event::main CertStrToNameA", GetLastError());

        CERT_NAME_BLOB blob;
        blob.cbData = count;
        blob.pbData = name;

        this->cert = CertCreateSelfSignCertificate(0, &blob, 0, 0, 0, 0, 0, 0); // $ seems to spin up win32 thread pool?
        if (this->cert == 0)
            return Basic::globals->HandleError("handle_event::main CertCreateSelfSignCertificate", GetLastError());

        success = ExtractPrivateKey();
        if (!success)
            return false;

        return true;
    }

    bool Globals::ParseCert(ByteString* bytes, uint32 count, uint32 error)
    {
        if (error != ERROR_SUCCESS)
            return Basic::globals->HandleError("read cert from file failed", error);

        bytes->resize(count);

        DebugWriter()->WriteLine("initializing certificate from file");

        CRYPT_DATA_BLOB pfx_blob;
        pfx_blob.pbData = bytes->address();
        pfx_blob.cbData = bytes->size();

        Basic::HCERTSTORE store = PFXImportCertStore(&pfx_blob, this->certificate_file_password.c_str(), CRYPT_MACHINE_KEYSET | PKCS12_ALLOW_OVERWRITE_KEY);
        if (store == 0)
            return Basic::globals->HandleError("handle_event::main PFXImportCertStore", GetLastError());

        this->cert = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, 0, 0);
        if (this->cert == 0)
            return Basic::globals->HandleError("handle_event::main CertFindCertificateInStore", GetLastError());

        bool success = ExtractPrivateKey();
        if (!success)
            return false;

        return true;
    }

    void Globals::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::encodings_pending_state:
            {
                if (event->get_type() != Basic::EventType::encodings_complete_event)
                {
                    Basic::HandleError("unexpected event");
                    throw Yield("unexpected event");
                }

                DebugWriter()->WriteLine("initializing html globals");

                std::shared_ptr<HtmlNamedCharacterReferences> named_characters = std::make_shared<HtmlNamedCharacterReferences>(this->shared_from_this(), ByteStringRef());
                named_characters->start();

                switch_to_state(State::named_character_references_pending_state);
            }
            break;

        case State::named_character_references_pending_state:
            {
                if (event->get_type() != Service::EventType::characters_complete_event)
                    throw Yield("unexepected event");

                DebugWriter()->WriteLine("initializing endpoints");

                this->http_endpoint = std::make_shared<WebServerEndpoint>(Basic::ListenSocket::Face_Default, 81, std::shared_ptr<Tls::ICertificate>());
                this->http_endpoint->SpawnListeners(20);

                this->https_endpoint = std::make_shared<WebServerEndpoint>(Basic::ListenSocket::Face_Default, 82, this->shared_from_this());
                this->https_endpoint->SpawnListeners(20);

                this->ftp_control_endpoint = std::make_shared<FtpServerEndpoint>(Basic::ListenSocket::Face_Default, 21);
                this->ftp_control_endpoint->SpawnListeners(20);

                switch_to_state(State::accepts_pending_state);
                throw Yield("event consumed");
            }
            break;

        case State::accepts_pending_state:
            throw FatalError("was Yield... what is expected for this state?");

        default:
            throw FatalError("Html::Globals::Complete unexpected state");
        }
    }

    bool Globals::SetThreadCount(uint32 count)
    {
        DebugWriter()->WriteLine("initializing threads");

        if (count == 0)
        {
            SYSTEM_INFO info;
            GetSystemInfo(&info);

            count = info.dwNumberOfProcessors * 2;
        }

        for (uint32 i = 0; i < count; i++)
        {
            HANDLE thread = CreateThread(0, 0, Thread, static_cast<Globals*>(this), 0, 0);
            if (thread == 0)
                return Basic::globals->HandleError("CreateThread", GetLastError());

            threads.push_back(thread);
        }

        return true;
    }

    bool Globals::SendStopSignal()
    {
        BOOL success = SetEvent(stopEvent);
        if (success == FALSE)
            return Basic::globals->HandleError("SetEvent", GetLastError());

        return true;
    }

    bool Globals::Thread()
    {
        while (true)
        {
            OVERLAPPED_ENTRY entries[0x10]; // $
            ULONG count;

            BOOL success = GetQueuedCompletionStatusEx(queue, entries, _countof(entries), &count, 5000, false);
            if (success == FALSE)
            {
                DWORD error = GetLastError();
                if (error != WAIT_TIMEOUT)
                {
                    if (error != ERROR_ABANDONED_WAIT_0 && error != ERROR_INVALID_HANDLE)
                        throw Basic::FatalError("GetQueuedCompletionStatusEx", error);

                    return false;
                }
                else
                {
                    count = 0;
                }
            }

            for (uint32 i = 0; i < count; i++)
            {
                OVERLAPPED_ENTRY* entry = entries + i;
                Job::complete(entry);
            }

            DWORD result = WaitForSingleObject(stopEvent, 0);
            if (result == WAIT_OBJECT_0)
                break;
        }

        return true;
    }

    void Globals::QueueJob(std::shared_ptr<Job> job)
    {
        BOOL success = PostQueuedCompletionStatus(this->queue, 0, 0, job.get());
        if (success == 0)
            throw Basic::FatalError("Globals::QueueJob PostQueuedCompletionStatus", GetLastError());
    }

    void Globals::BindToCompletionQueue(FileLog* log_file)
    {
        HANDLE handle = log_file->file;

        HANDLE result = CreateIoCompletionPort(handle, this->queue, 0, 0);
        if (result == 0)
            throw Basic::FatalError("CreateIoCompletionPort", GetLastError());
    }

    void Globals::BindToCompletionQueue(Socket* socket)
    {
        HANDLE handle = reinterpret_cast<HANDLE>(socket->socket);

        HANDLE result = CreateIoCompletionPort(handle, this->queue, 0, 0);
        if (result == 0)
            throw Basic::FatalError("CreateIoCompletionPort", GetLastError());
    }

    bool Globals::CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult)
    {
        SECURITY_STATUS error = NCryptDecrypt(
            this->private_key,
            pbInput,
            cbInput,
            0,
            pbOutput,
            cbOutput,
            pcbResult,
            NCRYPT_PAD_PKCS1_FLAG);
        if (error != 0)
            return Basic::globals->HandleError("Globals::CertDecrypt NCryptDecrypt", error);

        return true;
    }

    Tls::Certificates* Globals::Certificates()
    {
        return &this->certificates;
    }

    Basic::IStream<Codepoint>* Globals::LogStream()
    {
        if (debug_stream == 0)
        {
            Basic::LogStream* log_stream = new Basic::LogStream();
            log_stream->logs.push_back(this->console_log);
            log_stream->logs.push_back(this->debug_log);
            log_stream->logs.push_back(this->file_log);
            log_stream->logs.push_back(this->memory_log);
            log_stream->logs.push_back(this->tail_log);

            debug_stream = log_stream;
        }

        return debug_stream;
    }

    Basic::TextWriter* Globals::DebugWriter()
    {
        if (debug_writer == 0)
            debug_writer = new Basic::TextWriter(LogStream());

        return debug_writer;
    }

    bool Globals::HandleError(const char* context, uint32 error)
    {
        DebugWriter()->write_literal("ERROR: ");
        DebugWriter()->write_c_str(context);
        DebugWriter()->write_literal(" code=");
        DebugWriter()->WriteError(error);
        DebugWriter()->WriteLine();
        return false;
    }
}

SERVICE_STATUS service_status = {0};
SERVICE_STATUS_HANDLE service_status_handle = 0;

void __stdcall ServiceHandlerProc(DWORD dwOpcode)
{
    switch(dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        Service::globals->SendStopSignal();
        break;
    }
}

void __stdcall ServiceProc(DWORD, char**)
{
    service_status_handle = RegisterServiceCtrlHandlerA(Service::globals->service_name.c_str(), ServiceHandlerProc);
    if (service_status_handle == 0)
    {
        Service::globals->HandleError("RegisterServiceCtrlHandler", GetLastError());
        return;
    }

    service_status.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwWaitHint = 0;

    service_status.dwCurrentState = SERVICE_START_PENDING;
    ::SetServiceStatus(service_status_handle, &service_status);

    bool success = Service::globals->Initialize();
    if (success)
    {
        service_status.dwCurrentState = SERVICE_RUNNING;
        ::SetServiceStatus(service_status_handle, &service_status);

        Service::globals->WaitForStopSignal();
    }
    else
    {
        service_status.dwWin32ExitCode = 1;
    }

    service_status.dwCurrentState = SERVICE_STOP_PENDING;
    service_status.dwWaitHint = 1000;
    ::SetServiceStatus(service_status_handle, &service_status);

    Service::globals->Cleanup();

    service_status.dwCurrentState = SERVICE_STOPPED;
    ::SetServiceStatus(service_status_handle, &service_status);
}

int main(int argc, char* argv[])
{
    // must be constructed first because Basic::globals needs it, and we set some variables from the command line parameters
    std::shared_ptr<Service::Globals> service_global_ref;
    make_immortal<Service::Globals>(&Service::globals, &service_global_ref);

    // must be initialized next so that error handling and ascii encoding are initialized
    make_immortal<Basic::Globals>(&Basic::globals, 0);

    Basic::globals->Initialize(service_global_ref, service_global_ref);

    if (argc == 3 && (0 == _stricmp(argv[1], "/u") || 0 == _stricmp(argv[1], "/uninstall")))
    {
        Service::globals->service_name = argv[2];

        SC_HANDLE sc_manager = ::OpenSCManagerA(0, 0, GENERIC_READ | GENERIC_WRITE);
        if (sc_manager == 0)
            return Service::globals->HandleError("OpenSCManagerA", GetLastError());

        SC_HANDLE service = ::OpenServiceA(sc_manager, Service::globals->service_name.c_str(), SERVICE_QUERY_CONFIG);
        if (service != 0)
        {
            bool success = (bool)::DeleteService(service);
            if (!success)
                return Service::globals->HandleError("DeleteService", GetLastError());

            ::CloseServiceHandle(service);
        }

        ::CloseServiceHandle(sc_manager);

        printf("Uninstalled %hs service.", Service::globals->service_name.c_str());

        return 0;
    }
    else if (argc == 3 || argc == 4 || argc == 5)
    {
        Service::globals->service_name = argv[2];
        Service::globals->self_sign_domain = "default_self_sign";

        if (argc == 4)
        {
            Service::globals->self_sign_domain = argv[3];
        }
        else if (argc == 5)
        {
            Service::globals->self_sign_domain = "bad_certificate_file";
            Service::globals->certificate_file_name = argv[3];
            Service::globals->certificate_file_password.insert(Service::globals->certificate_file_password.end(), argv[4], argv[4] + strlen(argv[4]));
        }

        if (0 == _stricmp(argv[1], "/i") || 0 == _stricmp(argv[1], "/install"))
        {
            char exe_path[MAX_PATH + 0x100];

            DWORD count = GetModuleFileNameA(0, exe_path, sizeof(exe_path));
            if (count == sizeof(exe_path))
                return Service::globals->HandleError("GetModuleFileNameA", GetLastError());

            strcat_s(exe_path, " /service \"");
            strcat_s(exe_path, argv[2]);
            strcat_s(exe_path, "\" \"");
            strcat_s(exe_path, argv[3]);
            strcat_s(exe_path, "\"");

            if (argc == 5)
            {
                strcat_s(exe_path, " \"");
                strcat_s(exe_path, argv[4]);
                strcat_s(exe_path, "\"");
            }

            SC_HANDLE sc_manager = ::OpenSCManagerA(0, 0, GENERIC_READ | GENERIC_WRITE);
            if (sc_manager == 0)
                return Service::globals->HandleError("OpenSCManagerA", GetLastError());

            SC_HANDLE service = ::OpenServiceA(sc_manager, Service::globals->service_name.c_str(), SERVICE_QUERY_CONFIG);
            if (service == 0)
            {
                service = ::CreateServiceA(
                    sc_manager,
                    Service::globals->service_name.c_str(),
                    Service::globals->service_name.c_str(),
                    SERVICE_ALL_ACCESS,
                    SERVICE_WIN32_OWN_PROCESS,
                    SERVICE_AUTO_START,
                    SERVICE_ERROR_NORMAL,
                    exe_path,
                    0,
                    0,
                    0,
                    0,
                    0);
                if (service == 0)
                    return Service::globals->HandleError("CreateServiceA", GetLastError());
            }

            ::CloseServiceHandle(service);
            ::CloseServiceHandle(sc_manager);

            printf("Installed %hs service.", Service::globals->service_name.c_str());

            return 0;
        }
        else if (0 == _stricmp(argv[1], "/c") || 0 == _stricmp(argv[1], "/console"))
        {
            bool success = Service::globals->Initialize();
            if (!success)
                return 1;

            Service::globals->WaitForStopSignal();
            Service::globals->Cleanup();

            return 0;
        }
        else if (0 == _stricmp(argv[1], "/s") || 0 == _stricmp(argv[1], "/service"))
        {
            SERVICE_TABLE_ENTRYA service_entry[] = {{(char*)Service::globals->service_name.c_str(), ServiceProc}, {0, 0}};

            bool success = StartServiceCtrlDispatcherA(service_entry);
            if (!success)
                return Service::globals->HandleError("StartServiceCtrlDispatcherA", GetLastError());

            return 0;
        }
    }

    printf("Help:\n");
    printf("  /u or /uninstall service_name\n");
    printf("  /i or /install service_name [self_sign_domain] | [certificate_file_name certificate_file_password]\n");
    printf("  /c or /console service_name [self_sign_domain] | [certificate_file_name certificate_file_password]\n");
    printf("  /s or /service service_name [self_sign_domain] | [certificate_file_name certificate_file_password]\n");

    return 0;
}
