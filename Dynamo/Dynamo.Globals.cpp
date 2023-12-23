#include "stdafx.h"
#include "Dynamo.Globals.h"
#include "Basic.Globals.h"
#include "Tls.Globals.h"
#include "Json.Globals.h"
#include "Html.Globals.h"
#include "Http.Globals.h"
#include "Basic.IStream.h"
#include "Basic.FrameStream.h"
#include "Dynamo.AdminProtocol.h"
#include "Http.Server.h"
#include "Http.Proxy.h"
#include "Html.ElementNode.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.SuffixArray.h"

namespace Dynamo
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
	}

	Globals::~Globals()
	{
		if (queue != 0)
			CloseHandle(queue);

		if (stopEvent != 0)
			CloseHandle(stopEvent);
	}

	Basic::IStream<Codepoint>* Globals::DebugStream()
	{
		if (debug_stream == 0)
		{
			Basic::DebugStream::Ref debug_frame = New<Basic::DebugStream>();
			debug_frame->Initialize(debugLog);

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

	bool Globals::Initialize()
	{
		__super::Initialize();

		this->videos = New<VideoMap>();

		Basic::globals = new Basic::Inline<Basic::Globals>();
		Basic::globals->Initialize();

		Tls::globals = new Tls::Globals();
		Tls::globals->Initialize();

		Http::globals = new Http::Globals();
		Http::globals->Initialize();

		Html::globals = new Basic::Inline<Html::Globals>();
		Html::globals->Initialize();

		Json::globals = new Json::Globals();
		Json::globals->Initialize();

		command_stop.Initialize("stop");
		command_list.push_back(command_stop);

		command_log.Initialize("log");
		command_list.push_back(command_log);

		command_amazon.Initialize("amazon");
		command_list.push_back(command_amazon);

		command_netflix.Initialize("netflix");
		command_list.push_back(command_netflix);

		command_get.Initialize("get");
		command_list.push_back(command_get);

		command_follow_link.Initialize("l");
		command_list.push_back(command_follow_link);

		command_select_form.Initialize("f");
		command_list.push_back(command_select_form);

		command_set_control_value.Initialize("c");
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

		streams_namespace.Initialize("dynamo.streams");

		amazon_title_class.Initialize("ilt2");
		amazon_result_id_prefix.Initialize("result_");
		amazon_source_name.Initialize("Amazon");
		amazon_sign_in_link.Initialize("Hello. Sign in Your Account");
		amazon_sign_in_form.Initialize("ap_signin_form");
		amazon_email_control.Initialize("email");
		amazon_password_control.Initialize("password");
		amazon_prime_link.Initialize("Your Prime");
		amazon_browse_link.Initialize("Browse Prime Instant Video");
		amazon_movies_link.Initialize("movies");
		amazon_next_page_link.Initialize("Next Page");

		netflix_sign_in_link.Initialize("Member Sign In");
		netflix_movieid_param.Initialize("movieid");
		netflix_movie_url.Initialize("http://dvd.netflix.com/Movie");
		netflix_search_form.Initialize("global-search");
		netflix_logon_form.Initialize("login-form");
		netflix_email_control.Initialize("email");
		netflix_password_control.Initialize("password");
		netflix_query1_control.Initialize("raw_query");
		netflix_query2_control.Initialize("v1");
		netflix_search_path.Initialize("Search");
		netflix_row_param.Initialize("row");

		for (Codepoint c = '0'; c <= '9'; c++)
		{
			Basic::UnicodeString::Ref term = New<UnicodeString>();
			term->push_back(c);
			netflix_search_space.push_back(term);
		}

		for (Codepoint c = 'a'; c <= 'z'; c++)
		{
			Basic::UnicodeString::Ref term = New<UnicodeString>();
			term->push_back(c);
			netflix_search_space.push_back(term);
		}

		root_admin.Initialize("admin");
		root_echo.Initialize("echo");
		root_question.Initialize("question");

		title_property.Initialize("title");
		as_of_property.Initialize("as of");
		source_property.Initialize("source");

		this->netflix_url = New<Basic::Uri>();
		this->netflix_url->Initialize("https://signup.netflix.com/Login");

		this->amazon_url = New<Basic::Uri>();
		this->amazon_url->Initialize("http://www.amazon.com/s/ref=sr_il_to_instant-video?rh=n%3A2858778011&ie=UTF8");

		this->debugLog = New<Basic::DebugLog>();

		DebugWriter()->WriteLine("initializing io completion port");

		this->queue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		if (this->queue == 0)
			return Basic::globals->HandleError("CreateIoCompletionPort", GetLastError());

		DebugWriter()->WriteLine("initializing log file");

		char log_path[MAX_PATH + 0x100];
		GetFilePath("CameraProxy.log", log_path);

		debugLog->Initialize(log_path, queue);

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

		WSADATA wsaData;
		int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (error != 0)
			return Basic::globals->HandleError("WSAStartup", error);

		SOCKET tempSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (tempSocket == INVALID_SOCKET)
			return Basic::globals->HandleError("socket", WSAGetLastError());

		uint32 count;

		GUID funcId = WSAID_CONNECTEX;
		error = WSAIoctl(
			tempSocket,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&funcId,
			sizeof(funcId),
			&ConnectEx,
			sizeof(ConnectEx),
			&count,
			0,
			0);
		if (error == SOCKET_ERROR)
			return Basic::globals->HandleError("socket", WSAGetLastError());

		closesocket(tempSocket);

		DebugWriter()->WriteLine("reading certificate");

		char pfx_path[MAX_PATH + 0x100];
		GetFilePath("CameraProxy.pfx", pfx_path);

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
				Dynamo::globals->PostCompletion(this, pfx_data);
			}
		}

		SetThreadCount(0);

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
		bool success = AcquireCert(bytes, transferred, error);
		if (!success)
			return false;

		DWORD keySpec;
		BOOL free;

		success = (bool)CryptAcquireCertificatePrivateKey(
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

		Basic::globals->InitializeEncodings(this, (ByteString*)0);

		return true;
	}

	bool Globals::AcquireCert(AsyncBytes* bytes, int transferred, int error)
	{
		bool success = ParseCert(bytes, transferred, error);
		if (!success)
		{
			DebugWriter()->WriteLine("initializing new self-sign certificate");

			byte name[0x100];
			uint32 count = _countof(name);

			bool success = (bool)CertStrToNameA(X509_ASN_ENCODING, "CN=spanton.net", 0, 0, name, &count, 0);
			if (!success)
				return Basic::globals->HandleError("Process::main CertStrToNameA", GetLastError());

			CERT_NAME_BLOB blob;
			blob.cbData = count;
			blob.pbData = name;

			this->cert = CertCreateSelfSignCertificate(0, &blob, 0, 0, 0, 0, 0, 0); // $ seems to spin up win32 thread pool?
			if (this->cert == 0)
				return Basic::globals->HandleError("Process::main CertCreateSelfSignCertificate", GetLastError());
		}

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

		Basic::HCERTSTORE store = PFXImportCertStore(&pfx_blob, L"dynamite", CRYPT_MACHINE_KEYSET | PKCS12_ALLOW_OVERWRITE_KEY);
		if (store == 0)
			return Basic::globals->HandleError("Process::main PFXImportCertStore", GetLastError());

		this->cert = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, 0, 0);
		if (this->cert == 0)
			return Basic::globals->HandleError("Process::main CertFindCertificateInStore", GetLastError());

		return true;
	}

	class EndPoint : public Frame
	{
	protected:
		Basic::ListenSocket::Ref listener;
		bool secure;

	public:
		typedef Basic::Ref<EndPoint, IProcess> Ref;

		void Initialize(ListenSocket::Face face, short port, bool secure)
		{
			__super::Initialize();

			this->listener = New<Basic::ListenSocket>();
			this->listener->Initialize(face, port);

			this->secure = secure;
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
	public:
		typedef Basic::Ref<HttpServerEndpoint, IProcess> Ref;

		virtual void SpawnListener()
		{
			Http::Server::Ref protocol = New<Http::Server>();
			protocol->Initialize(this->listener, this->secure, this, (ByteString*)0);
		}
	};

	class HttpProxyEndpoint : public EndPoint
	{
	private:
		Uri::Ref server_url;

	public:
		typedef Basic::Ref<HttpProxyEndpoint, IProcess> Ref;

		void Initialize(ListenSocket::Face face, short port, bool secure, Uri::Ref server_url)
		{
			__super::Initialize(face, port, secure);

			this->server_url = server_url;
		}

		virtual void SpawnListener()
		{
			Http::Proxy::Ref protocol = New<Http::Proxy>();
			protocol->Initialize(this->listener, this->secure, this, (ByteString*)0, this->server_url);
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

				Html::globals->InitializeNamedCharacterReferences(this, (ByteString*)0);

				switch_to_state(State::named_character_references_pending_state);
			}
			break;

		case State::named_character_references_pending_state:
			if (event->get_type() == Html::EventType::characters_complete_event)
			{
				DebugWriter()->WriteLine("initializing endpoints");

				HttpServerEndpoint::Ref http_endpoint = New<HttpServerEndpoint>();
				http_endpoint->Initialize(Basic::ListenSocket::Face_Default, 81, false);
				http_endpoint->SpawnListeners(20);

				HttpServerEndpoint::Ref https_endpoint = New<HttpServerEndpoint>();
				https_endpoint->Initialize(Basic::ListenSocket::Face_Default, 82, true);
				https_endpoint->SpawnListeners(20);

				Uri::Ref server_1 = New<Uri>();
				server_1->Initialize("http://192.168.1.51/");
				HttpProxyEndpoint::Ref proxy_1_endpoint = New<HttpProxyEndpoint>();
				proxy_1_endpoint->Initialize(Basic::ListenSocket::Face_Default, 451, true, server_1);
				proxy_1_endpoint->SpawnListeners(20);

				Uri::Ref server_2 = New<Uri>();
				server_2->Initialize("http://192.168.1.52/");
				HttpProxyEndpoint::Ref proxy_2_endpoint = New<HttpProxyEndpoint>();
				proxy_2_endpoint->Initialize(Basic::ListenSocket::Face_Default, 452, true, server_2);
				proxy_2_endpoint->SpawnListeners(20);

				Uri::Ref server_3 = New<Uri>();
				server_3->Initialize("http://192.168.1.53/");
				HttpProxyEndpoint::Ref proxy_3_endpoint = New<HttpProxyEndpoint>();
				proxy_3_endpoint->Initialize(Basic::ListenSocket::Face_Default, 453, true, server_3);
				proxy_3_endpoint->SpawnListeners(20);

				Uri::Ref server_4 = New<Uri>();
				server_4->Initialize("http://192.168.1.54/");
				HttpProxyEndpoint::Ref proxy_4_endpoint = New<HttpProxyEndpoint>();
				proxy_4_endpoint->Initialize(Basic::ListenSocket::Face_Default, 454, true, server_4);
				proxy_4_endpoint->SpawnListeners(20);

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

	void Globals::CreateSocket(int af, int type, int protocol, Basic::ICompletion* completion, SOCKET* createdSocket)
	{
		SOCKET socket = ::socket(af, type, protocol);
		if (socket == INVALID_SOCKET)
			throw new Basic::Exception("socket", WSAGetLastError());

		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), this->queue, reinterpret_cast<ULONG_PTR>(completion), 0);
		if (result == 0)
			throw new Basic::Exception("CreateIoCompletionPort", GetLastError());

		(*createdSocket) = socket;
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
			return Basic::globals->HandleError("Process::CertDecrypt NCryptDecrypt", error);

		return true;
	}

	void Globals::CreateCertFrame(Tls::CertificatesFrame::Ref* frame)
	{
		(*frame) = New<Tls::CertificatesFrame>();
		(*frame)->Initialize(&this->certificates);
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
				this->videos->Add(title_string->value, object);
			}
		}
	}

	void Globals::Search(UnicodeString::Ref query, Json::Array::Ref* results)
	{
		Json::Array::Ref list = New<Json::Array>();

		uint32 begin;
		uint32 end;

		this->videos->Search(query, &begin, &end);

		for (uint32 i = begin; i != end; i++)
		{
			list->elements.push_back(this->videos->results[i].value.item());
		}

		(*results) = list;
	}
}

bool HandleEarlyError(const char* context, uint32 error)
{
	printf("%hs 0x%08X", context, error);
	return false;
}

SERVICE_STATUS service_status = {0};
SERVICE_STATUS_HANDLE service_status_handle = 0;

void __stdcall ServiceHandlerProc(DWORD dwOpcode)
{
	switch(dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		Dynamo::globals->SendStopSignal();
		break;
	}
}

void __stdcall ServiceProc(DWORD, char**)
{
	service_status_handle = RegisterServiceCtrlHandlerA("CameraProxy", ServiceHandlerProc);
	if (service_status_handle == 0)
	{
		HandleEarlyError("RegisterServiceCtrlHandler", GetLastError());
		return;
	}

	service_status.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwWaitHint = 0;

	service_status.dwCurrentState = SERVICE_START_PENDING;
	::SetServiceStatus(service_status_handle, &service_status);

	bool success = Dynamo::globals->Initialize();
	if (success)
	{
		service_status.dwCurrentState = SERVICE_RUNNING;
		::SetServiceStatus(service_status_handle, &service_status);

		Dynamo::globals->WaitForStopSignal();
	}
	else
	{
		service_status.dwWin32ExitCode = 1;
	}

	service_status.dwCurrentState = SERVICE_STOP_PENDING;
	service_status.dwWaitHint = 1000;
	::SetServiceStatus(service_status_handle, &service_status);

	Dynamo::globals->Cleanup();

	service_status.dwCurrentState = SERVICE_STOPPED;
	::SetServiceStatus(service_status_handle, &service_status);
}

int main(int argc, char* argv[])
{
	Dynamo::globals = new Basic::Inline<Dynamo::Globals>();

	if (argc == 2)
	{
		if (0 == _stricmp(argv[1], "/i") || 0 == _stricmp(argv[1], "/install"))
		{
			SC_HANDLE sc_manager = ::OpenSCManagerA(0, 0, GENERIC_READ | GENERIC_WRITE);
			if (sc_manager == 0)
				return HandleEarlyError("OpenSCManagerA", GetLastError());

			char exe_path[MAX_PATH + 0x100];

			DWORD count = GetModuleFileNameA(0, exe_path, sizeof(exe_path));
			if (count == sizeof(exe_path))
				return HandleEarlyError("GetModuleFileNameA", GetLastError());

			strcat_s(exe_path, " /service");

			SC_HANDLE service = ::OpenServiceA(sc_manager, "CameraProxy", SERVICE_QUERY_CONFIG);
			if (service == 0)
			{
				service = ::CreateServiceA(
					sc_manager,
					"CameraProxy",
					"Security Camera Proxy",
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
					return HandleEarlyError("CreateServiceA", GetLastError());
			}

			::CloseServiceHandle(service);
			::CloseServiceHandle(sc_manager);

			printf("Installed CameraProxy service.");

			return 0;
		}
		else if (0 == _stricmp(argv[1], "/u") || 0 == _stricmp(argv[1], "/uninstall"))
		{
			SC_HANDLE sc_manager = ::OpenSCManagerA(0, 0, GENERIC_READ | GENERIC_WRITE);
			if (sc_manager == 0)
				return HandleEarlyError("OpenSCManagerA", GetLastError());

			char exe_path[MAX_PATH + 0x100];

			DWORD count = GetModuleFileNameA(0, exe_path, sizeof(exe_path));
			if (count == sizeof(exe_path))
				return HandleEarlyError("GetModuleFileNameA", GetLastError());

			strcat_s(exe_path, " /service");

			SC_HANDLE service = ::OpenServiceA(sc_manager, "CameraProxy", SERVICE_QUERY_CONFIG);
			if (service != 0)
			{
				bool success = (bool)::DeleteService(service);
				if (!success)
					return HandleEarlyError("DeleteService", GetLastError());

				::CloseServiceHandle(service);
			}

			::CloseServiceHandle(sc_manager);

			printf("Uninstalled CameraProxy service.");

			return 0;
		}
		else if (0 == _stricmp(argv[1], "/c") || 0 == _stricmp(argv[1], "/console"))
		{
			bool success = Dynamo::globals->Initialize();
			if (!success)
				return 1;

			Dynamo::globals->WaitForStopSignal();
			Dynamo::globals->Cleanup();

			return 0;
		}
		else if (0 == _stricmp(argv[1], "/s") || 0 == _stricmp(argv[1], "/service"))
		{
			SERVICE_TABLE_ENTRYA service_entry[] = {{"CameraProxy", ServiceProc}, {0, 0}};

			bool success = StartServiceCtrlDispatcherA(service_entry);
			if (!success)
				return HandleEarlyError("StartServiceCtrlDispatcherA", GetLastError());

			return 0;
		}
	}

	printf("Help:\n");
	printf("  /i or /install\n");
	printf("  /u or /uninstall\n");
	printf("  /c or /console\n");
	printf("  /s or /service\n");

	return 0;
}
