#pragma once

#include <Windows.h>
#include "Basic.ICompletion.h"
#include "Basic.DebugStream.h"
#include "Basic.ListenSocket.h"
#include "Basic.Cng.h"
#include "Tls.CertificatesFrame.h"
#include "Dynamo.HeapStorage.h"
#include "Basic.Uri.h"
#include "Dynamo.AdminProtocol.h"
#include "Dynamo.Types.h"

namespace Dynamo
{
	class Globals : public Frame, public ICompletion, public IRefHolder
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
		typedef Basic::StringMapCaseInsensitive<Basic::ListenSocket::Ref> Listeners; // $$$
		typedef std::vector<Basic::UnicodeString::Ref> CommandList; // $$$

		Basic::Ref<Basic::Console> console; // $$$
		Basic::DebugLog::Ref debugLog; // $$$

		Basic::UnicodeString::Ref command_stop; // $$$
		Basic::UnicodeString::Ref command_log; // $$$
		Basic::UnicodeString::Ref command_amazon; // $$$
		Basic::UnicodeString::Ref command_netflix; // $$$
		Basic::UnicodeString::Ref command_get; // $$$
		Basic::UnicodeString::Ref command_follow_link; // $$$
		Basic::UnicodeString::Ref command_select_form; // $$$
		Basic::UnicodeString::Ref command_set_control_value; // $$$
		Basic::UnicodeString::Ref command_submit; // $$$
		Basic::UnicodeString::Ref command_render_links; // $$$
		Basic::UnicodeString::Ref command_render_forms; // $$$
		Basic::UnicodeString::Ref command_render_nodes; // $$$
		Basic::UnicodeString::Ref command_search; // $$$

		CommandList command_list;

		Basic::UnicodeString::Ref streams_namespace; // $$$

		Basic::UnicodeString::Ref amazon_title_class; // $$$
		Basic::UnicodeString::Ref amazon_result_id_prefix; // $$$
		Basic::UnicodeString::Ref amazon_source_name; // $$$
		Basic::UnicodeString::Ref amazon_sign_in_link; // $$$
		Basic::UnicodeString::Ref amazon_sign_in_form; // $$$
		Basic::UnicodeString::Ref amazon_email_control; // $$$
		Basic::UnicodeString::Ref amazon_password_control; // $$$
		Basic::UnicodeString::Ref amazon_prime_link; // $$$
		Basic::UnicodeString::Ref amazon_browse_link; // $$$
		Basic::UnicodeString::Ref amazon_movies_link; // $$$
		Basic::UnicodeString::Ref amazon_next_page_link; // $$$

		Basic::UnicodeString::Ref netflix_sign_in_link; // $$$
		Basic::UnicodeString::Ref netflix_movieid_param; // $$$
		Basic::UnicodeString::Ref netflix_movie_url; // $$$
		Basic::UnicodeString::Ref netflix_search_form; // $$$
		Basic::UnicodeString::Ref netflix_logon_form; // $$$
		Basic::UnicodeString::Ref netflix_email_control; // $$$
		Basic::UnicodeString::Ref netflix_password_control; // $$$
		Basic::UnicodeString::Ref netflix_query1_control; // $$$
		Basic::UnicodeString::Ref netflix_query2_control; // $$$
		Basic::UnicodeString::Ref netflix_search_path; // $$$
		Basic::UnicodeString::Ref netflix_row_param; // $$$

		StringList netflix_search_space;

		Basic::Uri::Ref netflix_url; // $$$
		Basic::Uri::Ref amazon_url; // $$$

		Basic::UnicodeString::Ref root_admin; // $$$
		Basic::UnicodeString::Ref root_echo; // $$$
		Basic::UnicodeString::Ref root_question; // $$$

		AdminProtocol::Ref adminProtocol; // $$$

		UnicodeString::Ref title_property; // $$$
		UnicodeString::Ref as_of_property; // $$$
		UnicodeString::Ref source_property; // $$$

		VideoMap::Ref videos; // $$$

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

		void CreateCertFrame(Tls::CertificatesFrame::Ref* frame);

		LPFN_CONNECTEX ConnectEx;

		Basic::IStream<Codepoint>* DebugStream();
		Basic::TextWriter* DebugWriter();

		void CreateSocket(int af, int type, int protocol, Basic::ICompletion* completion, SOCKET* createdSocket);
		void PostCompletion(Basic::ICompletion* completion, LPOVERLAPPED overlapped);
		void QueueProcess(Basic::Ref<IProcess> completion, ByteString::Ref cookie);

		bool CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult);

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
	};

	extern Basic::Inline<Globals>* globals;
}