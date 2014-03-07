// Copyright © 2013 Brian Spanton

#pragma once

#include <Windows.h>
#include "Basic.ICompletion.h"
#include "Basic.DebugStream.h"
#include "Basic.ListenSocket.h"
#include "Basic.Cng.h"
#include "Tls.CertificatesFrame.h"
#include "Basic.Frame.h"
#include "Basic.Uri.h"
#include "Service.AdminProtocol.h"
#include "Service.Types.h"
#include "Basic.ICompletionQueue.h"
#include "Tls.ICertificate.h"

namespace Service
{
	using namespace Basic;

	class Globals : public Frame, public ICompletion, public IRefHolder, public IErrorHandler, public ICompletionQueue, public Tls::ICertificate
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
		bool CompleteRead(AsyncBytes* bytes, int transferred, int error);
		bool AcquireCert(AsyncBytes* bytes, int transferred, int error);
		bool ParseCert(AsyncBytes* bytes, int transferred, int error);

	public:
		typedef Basic::StringMapCaseInsensitive<Basic::ListenSocket::Ref> Listeners; // REF
		typedef std::vector<Basic::UnicodeString::Ref> CommandList; // REF

		Basic::Ref<Basic::Console> console; // REF
		Basic::DebugLog::Ref debugLog; // REF

		std::string service_name;
		std::string self_sign_domain;
		std::string certificate_file_name;
		std::wstring certificate_file_password;

		Basic::UnicodeString::Ref command_stop; // REF
		Basic::UnicodeString::Ref command_log; // REF
		Basic::UnicodeString::Ref command_get; // REF
		Basic::UnicodeString::Ref command_follow_link; // REF
		Basic::UnicodeString::Ref command_select_form; // REF
		Basic::UnicodeString::Ref command_set_control_value; // REF
		Basic::UnicodeString::Ref command_submit; // REF
		Basic::UnicodeString::Ref command_render_links; // REF
		Basic::UnicodeString::Ref command_render_forms; // REF
		Basic::UnicodeString::Ref command_render_nodes; // REF
		Basic::UnicodeString::Ref command_search; // REF

		CommandList command_list;

		Basic::UnicodeString::Ref root_admin; // REF
		Basic::UnicodeString::Ref root_echo; // REF
		Basic::UnicodeString::Ref root_question; // REF

		AdminProtocol::Ref adminProtocol; // REF

		UnicodeString::Ref title_property; // REF
		UnicodeString::Ref as_of_property; // REF
		UnicodeString::Ref source_property; // REF

		Index::Ref index; // REF

		Globals();
		~Globals();

		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ICompletion::CompleteAsync(OVERLAPPED_ENTRY& entry);

		bool Initialize();
		void WaitForStopSignal();
		void Cleanup();
		bool SendStopSignal();
		bool Thread();
		bool SetThreadCount(uint32 count);

		void Store(UnicodeString::Ref source, Json::Value::Ref value);
		void Search(UnicodeString::Ref query, Json::Array::Ref* results);

		template <int value_count>		
		void GetFilePath(const char* name, char (&value)[value_count])
		{
			char exe_path[MAX_PATH + 0x100];

			DWORD exe_path_count = GetModuleFileNameA(0, exe_path, sizeof(exe_path));
			if (exe_path_count == sizeof(exe_path))
				throw new Exception("GetModuleFileNameA", GetLastError());

			char drive[MAX_PATH + 0x100];
			char directory[MAX_PATH + 0x100];

			_splitpath_s(exe_path, drive, sizeof(drive), directory, sizeof(directory), 0, 0, 0, 0);

			strcpy_s(value, drive);
			strcat_s(value, directory);
			strcat_s(value, "\\");
			strcat_s(value, name);
		}

		virtual bool IErrorHandler::HandleError(const char* context, uint32 error);
		virtual Basic::IStream<Codepoint>* IErrorHandler::DebugStream();
		virtual Basic::TextWriter* IErrorHandler::DebugWriter();

		virtual void ICompletionQueue::BindToCompletionQueue(Socket::Ref socket);
		virtual void ICompletionQueue::BindToCompletionQueue(LogFile::Ref socket);
		virtual void ICompletionQueue::PostCompletion(Basic::ICompletion* completion, LPOVERLAPPED overlapped);
		virtual void ICompletionQueue::QueueProcess(Basic::Ref<IProcess> process, ByteString::Ref cookie);

		virtual bool ICertificate::CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult);
		virtual Tls::Certificates* ICertificate::Certificates();
	};

	extern Basic::Inline<Globals>* globals;
}