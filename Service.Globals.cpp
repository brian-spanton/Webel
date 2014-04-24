// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.Globals.h"
#include "Basic.Globals.h"
#include "Tls.Globals.h"
#include "Json.Globals.h"
#include "Html.Globals.h"
#include "Http.Globals.h"
#include "Basic.IStream.h"
#include "Basic.FrameStream.h"
#include "Service.AdminProtocol.h"
#include "Web.Server.h"
#include "Web.Proxy.h"
#include "Html.ElementNode.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.SuffixArray.h"
#include "Service.StandardEncodings.h"
#include "Tls.ICertificate.h"
#include "Service.HttpServer.h"
#include "Service.HtmlNamedCharacterReferences.h"
#include "Web.Globals.h"

namespace Service
{
    Basic::Inline<Globals>* globals = 0;

    __declspec(thread) Basic::IStream<Codepoint>* debug_stream = 0;
    __declspec(thread) Basic::TextWriter* debug_writer = 0;

    class ProcessCompletion : public ICompletion, public IRefHolder
    {
    private:
        Basic::Ref<IRefCounted> queue_ref;
        Basic::Ref<IProcess> process;
        ByteString::Ref cookie;

    public:
        typedef Basic::Ref<ProcessCompletion> Ref;

        ProcessCompletion()
        {
            this->queue_ref.SetHolder(this);
        }

        void PrepareForQueue(Basic::Ref<IProcess> process, ByteString::Ref cookie)
        {
            queue_ref = this;
            this->process = process;
            this->cookie = cookie;
        }

        virtual void Basic::ICompletion::CompleteAsync(OVERLAPPED_ENTRY& SortEntry)
        {
            ProcessEvent event;
            this->process->Process(&event);
            this->queue_ref = 0;
        }
    };

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

        this->debugLog = New<Basic::DebugLog>();
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
        __super::Initialize();

        this->index = New<Index>();

        Tls::globals = new Tls::Globals();
        Tls::globals->Initialize();

        Http::globals = new Http::Globals();
        Http::globals->Initialize();

        Html::globals = new Basic::Inline<Html::Globals>();
        Html::globals->Initialize();

        Json::globals = new Json::Globals();
        Json::globals->Initialize();

        Web::globals = new Web::Globals();
        Web::globals->Initialize();

        command_stop.Initialize("stop");
        command_list.push_back(command_stop);

        command_log.Initialize("log");
        command_list.push_back(command_log);

        command_get.Initialize("get");
        command_list.push_back(command_get);

        command_follow_link.Initialize("link");
        command_list.push_back(command_follow_link);

        command_select_form.Initialize("form");
        command_list.push_back(command_select_form);

        command_set_control_value.Initialize("control");
        command_list.push_back(command_set_control_value);

        command_submit.Initialize("submit");
        command_list.push_back(command_submit);

        command_render_links.Initialize("links");
        command_list.push_back(command_render_links);

        command_render_forms.Initialize("forms");
        command_list.push_back(command_render_forms);

        command_render_nodes.Initialize("nodes");
        command_list.push_back(command_render_nodes);

        command_search.Initialize("search");
        command_list.push_back(command_search);

        root_admin.Initialize("admin");
        root_echo.Initialize("echo");
        root_question.Initialize("question");

        title_property.Initialize("title"); // $ schema for search index
        as_of_property.Initialize("as of"); // $ schema for search index
        source_property.Initialize("source"); // $ schema for search index

        DebugWriter()->WriteLine("initializing io completion port");

        this->queue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
        if (this->queue == 0)
            return Basic::globals->HandleError("CreateIoCompletionPort", GetLastError());

        DebugWriter()->WriteLine("initializing log file");

        char log_path[MAX_PATH + 0x100];
        GetFilePath("service.log", log_path);

        this->debugLog->Initialize(log_path);

        DebugWriter()->WriteLine("initializing stop event");

        this->stopEvent = CreateEvent(0, TRUE, FALSE, 0);
        if (this->stopEvent == 0)
            return Basic::globals->HandleError("CreateEvent", GetLastError());

        DebugWriter()->WriteLine("initializing console");

        this->console = New<Basic::Console>();

        this->adminProtocol = New<AdminProtocol>();
        this->adminProtocol->Initialize(this->console);

        this->console->Initialize(this->adminProtocol, &this->consoleThread);

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

        SetThreadCount(0);

        return true;
    }

    bool Globals::ReadCertificate()
    {
        char pfx_path[MAX_PATH + 0x100];
        GetFilePath(Service::globals->certificate_file_name.c_str(), pfx_path);

        DebugWriter()->Write("reading ");
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

        HANDLE result = CreateIoCompletionPort(this->pfx_file, this->queue, reinterpret_cast<ULONG_PTR>(static_cast<ICompletion*>(this)), 0);
        if (result == 0)
            return Basic::globals->HandleError("CreateIoCompletionPort", GetLastError());

        LARGE_INTEGER size;
        bool success = (bool)GetFileSizeEx(this->pfx_file, &size);
        if (!success)
            return Basic::globals->HandleError("GetFileSizeEx", GetLastError());

        if (size.HighPart > 0)
            return Basic::globals->HandleError("GetFileSizeEx returned unexpectedly large size", 0);

        AsyncBytes::Ref pfx_data = New<AsyncBytes>("7");
        pfx_data->Initialize(size.LowPart);
        pfx_data->PrepareForReceive("ConnectedSocket::StartReceive WSARecv", this);

        success = (bool)ReadFile(this->pfx_file, pfx_data->bytes, size.LowPart, 0, pfx_data);
        if (!success)
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING)
            {
                pfx_data->Internal = error;
                Service::globals->PostCompletion(this, pfx_data);
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

    void Globals::CompleteAsync(OVERLAPPED_ENTRY& entry)
    {
        int transferred = entry.dwNumberOfBytesTransferred;
        int error = static_cast<int>(entry.lpOverlapped->Internal);

        AsyncBytes::Ref bytes = AsyncBytes::FromOverlapped(entry.lpOverlapped);
        bytes->IoCompleted();

        bool success = CompleteRead(bytes, transferred, error);
        if (!success)
        {
            SendStopSignal();
        }
    }

    bool Globals::CompleteRead(AsyncBytes* bytes, int transferred, int error)
    {
        bool success = ParseCert(bytes, transferred, error);
        if (!success)
        {
            success = CreateSelfSignCert();
            if (!success)
                return false;
        }

        return true;
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
            return Basic::globals->HandleError("Process::main CryptAcquireCertificatePrivateKey", GetLastError());

        if (!free)
            return Basic::globals->HandleError("Process::main !free", 0);

        if (keySpec != CERT_NCRYPT_KEY_SPEC)
            return Basic::globals->HandleError("Process::main != CERT_NCRYPT_KEY_SPEC", 0);

        this->certificates.resize(1);

        this->certificates[0].resize(this->cert->cbCertEncoded);
        CopyMemory(&(this->certificates[0][0]), this->cert->pbCertEncoded, this->cert->cbCertEncoded);

        DebugWriter()->WriteLine("initializing encodings");

        switch_to_state(State::encodings_pending_state);

        StandardEncodings::Ref standard_encodings = New<StandardEncodings>();
        standard_encodings->Initialize(this, (ByteString*)0);

        return true;
    }

    bool Globals::CreateSelfSignCert()
    {
        byte name[0x100];
        uint32 count = _countof(name);

        std::string x_500;

        x_500 = "CN=";
        x_500 += this->self_sign_domain;

        DebugWriter()->Write("creating transient self-sign certificate ");
        DebugWriter()->WriteLine(x_500.c_str());

        bool success = (bool)CertStrToNameA(X509_ASN_ENCODING, x_500.c_str(), 0, 0, name, &count, 0);
        if (!success)
            return Basic::globals->HandleError("Process::main CertStrToNameA", GetLastError());

        CERT_NAME_BLOB blob;
        blob.cbData = count;
        blob.pbData = name;

        this->cert = CertCreateSelfSignCertificate(0, &blob, 0, 0, 0, 0, 0, 0); // $ seems to spin up win32 thread pool?
        if (this->cert == 0)
            return Basic::globals->HandleError("Process::main CertCreateSelfSignCertificate", GetLastError());

        success = ExtractPrivateKey();
        if (!success)
            return false;

        return true;
    }

    bool Globals::ParseCert(AsyncBytes* bytes, int transferred, int error)
    {
        if (error != ERROR_SUCCESS)
            return Basic::globals->HandleError("read cert from file failed", error);

        DebugWriter()->WriteLine("initializing certificate from file");

        CRYPT_DATA_BLOB pfx_blob;
        pfx_blob.cbData = transferred;
        pfx_blob.pbData = bytes->bytes;

        Basic::HCERTSTORE store = PFXImportCertStore(&pfx_blob, this->certificate_file_password.c_str(), CRYPT_MACHINE_KEYSET | PKCS12_ALLOW_OVERWRITE_KEY);
        if (store == 0)
            return Basic::globals->HandleError("Process::main PFXImportCertStore", GetLastError());

        this->cert = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, 0, 0);
        if (this->cert == 0)
            return Basic::globals->HandleError("Process::main CertFindCertificateInStore", GetLastError());

        bool success = ExtractPrivateKey();
        if (!success)
            return false;

        return true;
    }

    class EndPoint : public Frame
    {
    protected:
        Basic::ListenSocket::Ref listener;

    public:
        typedef Basic::Ref<EndPoint, IProcess> Ref;

        void Initialize(ListenSocket::Face face, short port)
        {
            __super::Initialize();

            this->listener = New<Basic::ListenSocket>();
            this->listener->Initialize(face, port);
        }

        void SpawnListeners(uint16 count)
        {
            for (uint16 i = 0; i < count; i++)
            {
                SpawnListener();
            }
        }

        void Process(IEvent* event, bool* yield)
        {
            (*yield) = true;

            if (event->get_type() == Http::EventType::accept_complete_event)
            {
                SpawnListener();
            }
        }

        virtual void SpawnListener() = 0;
    };

    class HttpServerEndpoint : public EndPoint
    {
    private:
        Basic::Ref<Tls::ICertificate> certificate;

    public:
        typedef Basic::Ref<HttpServerEndpoint, IProcess> Ref;

        void Initialize(ListenSocket::Face face, short port, Basic::Ref<Tls::ICertificate> certificate)
        {
            __super::Initialize(face, port);

            this->certificate = certificate;
        }

        virtual void SpawnListener()
        {
            HttpServer::Ref protocol = New<HttpServer>();
            protocol->Initialize(this->listener, this->certificate, this, (ByteString*)0);
        }
    };

    class HttpProxyEndpoint : public EndPoint
    {
    private:
        Basic::Ref<Tls::ICertificate> certificate;
        Uri::Ref server_url;

    public:
        typedef Basic::Ref<HttpProxyEndpoint, IProcess> Ref;

        void Initialize(ListenSocket::Face face, short port, Basic::Ref<Tls::ICertificate> certificate, Uri::Ref server_url)
        {
            __super::Initialize(face, port);

            this->certificate = certificate;
            this->server_url = server_url;
        }

        virtual void SpawnListener()
        {
            Web::Proxy::Ref protocol = New<Web::Proxy>();
            protocol->Initialize(this->listener, this->certificate, this, (ByteString*)0, this->server_url);
        }
    };

    void Globals::Process(IEvent* event, bool* yield)
    {
        (*yield) = true;

        switch (frame_state())
        {
        case State::encodings_pending_state:
            if (event->get_type() == Basic::EventType::encodings_complete_event)
            {
                DebugWriter()->WriteLine("initializing html globals");

                HtmlNamedCharacterReferences::Ref named_characters = New<HtmlNamedCharacterReferences>();
                named_characters->Initialize(this, (ByteString*)0);

                switch_to_state(State::named_character_references_pending_state);
            }
            break;

        case State::named_character_references_pending_state:
            if (event->get_type() == Service::EventType::characters_complete_event)
            {
                DebugWriter()->WriteLine("initializing endpoints");

                HttpServerEndpoint::Ref http_endpoint = New<HttpServerEndpoint>();
                http_endpoint->Initialize(Basic::ListenSocket::Face_Default, 81, 0);
                http_endpoint->SpawnListeners(20);

                HttpServerEndpoint::Ref https_endpoint = New<HttpServerEndpoint>();
                https_endpoint->Initialize(Basic::ListenSocket::Face_Default, 82, this);
                https_endpoint->SpawnListeners(20);

                switch_to_state(State::accepts_pending_state);
            }
            break;

        case State::accepts_pending_state:
            break;

        default:
            throw new Exception("Html::Globals::Complete unexpected state");
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
                        throw new Basic::Exception("GetQueuedCompletionStatusEx", error);

                    return false;
                }
                else
                {
                    count = 0;
                }
            }

            for (uint32 i = 0; i < count; i++)
            {
                OVERLAPPED_ENTRY* SortEntry = entries + i;
                Basic::Ref<Basic::ICompletion> object = reinterpret_cast<Basic::ICompletion*>(SortEntry->lpCompletionKey);
                object->CompleteAsync(*SortEntry);
            }

            DWORD result = WaitForSingleObject(stopEvent, 0);
            if (result == WAIT_OBJECT_0)
                break;
        }

        return true;
    }

    class CompletionCompletion : public Basic::ICompletion, public IRefHolder
    {
    public:
        Basic::Ref<IRefCounted> queue_ref;
        Basic::Ref<Basic::ICompletion> completion;

    public:
        typedef Basic::Ref<CompletionCompletion> Ref;

        CompletionCompletion()
        {
            this->queue_ref.SetHolder(this);
        }

        void PrepareForQueue(Basic::ICompletion* completion)
        {
            this->queue_ref = this;
            this->completion = completion;
        }

        virtual void Basic::ICompletion::CompleteAsync(OVERLAPPED_ENTRY& SortEntry)
        {
            this->completion->CompleteAsync(SortEntry);
            this->queue_ref = 0;
        }
    };

    void Globals::PostCompletion(Basic::ICompletion* completion, LPOVERLAPPED overlapped)
    {
        CompletionCompletion::Ref outer_completion = New<CompletionCompletion>();
        outer_completion->PrepareForQueue(completion);

        BOOL success = PostQueuedCompletionStatus(this->queue, 0, reinterpret_cast<ULONG_PTR>(outer_completion.item()), overlapped);
        if (success == 0)
            throw new Basic::Exception("Globals::PostCompletion PostQueuedCompletionStatus", GetLastError());
    }

    void Globals::QueueProcess(Basic::Ref<IProcess> process, ByteString::Ref cookie)
    {
        ProcessCompletion::Ref completion = New<ProcessCompletion>();
        completion->PrepareForQueue(process, cookie);

        BOOL success = PostQueuedCompletionStatus(this->queue, 0, reinterpret_cast<ULONG_PTR>(completion.item()), 0);
        if (success == 0)
            throw new Basic::Exception("Globals::QueueProcess PostQueuedCompletionStatus", GetLastError());
    }

    void Globals::BindToCompletionQueue(LogFile::Ref logfile)
    {
        HANDLE handle = logfile->file;
        Basic::ICompletion* completion = logfile;

        HANDLE result = CreateIoCompletionPort(handle, this->queue, reinterpret_cast<ULONG_PTR>(completion), 0);
        if (result == 0)
            throw new Basic::Exception("CreateIoCompletionPort", GetLastError());
    }

    void Globals::BindToCompletionQueue(Socket::Ref socket)
    {
        HANDLE handle = reinterpret_cast<HANDLE>(socket->socket);
        Basic::ICompletion* completion = socket;

        HANDLE result = CreateIoCompletionPort(handle, this->queue, reinterpret_cast<ULONG_PTR>(completion), 0);
        if (result == 0)
            throw new Basic::Exception("CreateIoCompletionPort", GetLastError());
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

    void Globals::Store(UnicodeString::Ref source, Json::Value::Ref value)
    {
        if (value->type == Json::Value::Type::array_value)
        {
            Json::Array* array = (Json::Array*)value.item();

            for (Json::ValueList::iterator it = array->elements.begin(); it != array->elements.end(); it++)
            {
                Store(source, (*it));
            }
        }
        else if (value->type == Json::Value::Type::object_value)
        {
            Json::Object* object = (Json::Object*)value.item();

            Json::MemberList::iterator title_it = object->members.find(title_property);

            if (title_it != object->members.end() &&
                title_it->second->type == Json::Value::Type::string_value)
            {
                Json::String::Ref as_of = New<Json::String>();
                as_of->value = New<UnicodeString>();

                TextWriter writer(as_of->value);
                writer.WriteTimestamp();

                Json::MemberList::_Pairib as_of_result = object->members.insert(Json::MemberList::value_type(as_of_property, as_of.item()));
                if (as_of_result.second == false)
                    as_of_result.first->second = as_of;

                Json::String* title_string = (Json::String*)title_it->second.item();
                this->index->Add(title_string->value, object);
            }
        }
    }

    void Globals::Search(UnicodeString::Ref query, Json::Array::Ref* results)
    {
        Json::Array::Ref list = New<Json::Array>();

        uint32 begin;
        uint32 end;

        this->index->Search(query, &begin, &end);

        for (uint32 i = begin; i != end; i++)
        {
            list->elements.push_back(this->index->results[i].value.item());
        }

        (*results) = list;
    }

    Basic::IStream<Codepoint>* Globals::DebugStream()
    {
        if (debug_stream == 0)
        {
            Basic::DebugStream::Ref debug_frame = New<Basic::DebugStream>();
            debug_frame->Initialize(this->debugLog);

            Basic::FrameStream<Codepoint>* frame_stream = new Basic::Inline<Basic::FrameStream<Codepoint> >();
            frame_stream->Initialize(debug_frame);

            debug_stream = frame_stream;
        }

        return debug_stream;
    }

    Basic::TextWriter* Globals::DebugWriter()
    {
        if (debug_writer == 0)
            debug_writer = new Basic::TextWriter(DebugStream());

        return debug_writer;
    }

    bool Globals::HandleError(const char* context, uint32 error)
    {
        DebugWriter()->Write("ERROR: ");
        DebugWriter()->Write(context);
        DebugWriter()->Write(" code=");
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
    Service::globals = new Basic::Inline<Service::Globals>();

    // must be initialized next so that error handling and ascii encoding are initialized
    Basic::globals = new Basic::Inline<Basic::Globals>();
    Basic::globals->Initialize(Service::globals, Service::globals);

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
