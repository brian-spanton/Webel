// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.TextWriter.h"
#include "Basic.Globals.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    TextWriter::TextWriter() :
        decoder(Basic::globals->ascii_index)
    {
    }

    TextWriter::TextWriter(IStream<Codepoint>* dest) :
        dest(dest),
        decoder(Basic::globals->ascii_index, this->dest) // initialization is in order of declaration in class def
    {
    }

    void TextWriter::Initialize(IStream<Codepoint>* dest)
    {
        this->dest = dest;
        this->decoder.set_destination(this->dest);
    }

    void TextWriter::write_elements(const char* text, uint32 count)
    {
        this->decoder.write_elements((const byte*)text, count);
    }

    void TextWriter::write_c_str(const char* text)
    {
        size_t length = strlen(text);
        write_elements(text, length);
    }

    void TextWriter::WriteLine(const char* text)
    {
        write_c_str(text);
        WriteLine();
    }

    void TextWriter::WriteLine()
    {
        write_literal("\r\n");
    }

    void TextWriter::WriteTimestamp()
    {
        SYSTEMTIME time;
        GetSystemTime(&time);
        WriteFormat<0x100>("%04d/%02d/%02d %02d:%02d:%02d.%03d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
    }

    void TextWriter::WriteError(uint32 error)
    {
        switch (error)
        {
#define CASE(e) \
        case e: \
            write_literal(#e); \
            break

            // WinError.h
            CASE(WAIT_TIMEOUT);
            CASE(ERROR_INVALID_FUNCTION);
            CASE(ERROR_FILE_NOT_FOUND);
            CASE(ERROR_PATH_NOT_FOUND);
            CASE(ERROR_TOO_MANY_OPEN_FILES);
            CASE(ERROR_ACCESS_DENIED);
            CASE(ERROR_INVALID_HANDLE);
            CASE(ERROR_ARENA_TRASHED);
            CASE(ERROR_NOT_ENOUGH_MEMORY);
            CASE(ERROR_INVALID_BLOCK);
            CASE(ERROR_BAD_ENVIRONMENT);
            CASE(ERROR_BAD_FORMAT);
            CASE(ERROR_INVALID_ACCESS);
            CASE(ERROR_INVALID_DATA);
            CASE(ERROR_OUTOFMEMORY);
            CASE(ERROR_INVALID_DRIVE);
            CASE(ERROR_CURRENT_DIRECTORY);
            CASE(ERROR_NOT_SAME_DEVICE);
            CASE(ERROR_NO_MORE_FILES);
            CASE(ERROR_WRITE_PROTECT);
            CASE(ERROR_BAD_UNIT);
            CASE(ERROR_NOT_READY);
            CASE(ERROR_BAD_COMMAND);
            CASE(ERROR_CRC);
            CASE(ERROR_BAD_LENGTH);
            CASE(ERROR_SEEK);
            CASE(ERROR_NOT_DOS_DISK);
            CASE(ERROR_SECTOR_NOT_FOUND);
            CASE(ERROR_OUT_OF_PAPER);
            CASE(ERROR_WRITE_FAULT);
            CASE(ERROR_READ_FAULT);
            CASE(ERROR_GEN_FAILURE);
            CASE(ERROR_SHARING_VIOLATION);
            CASE(ERROR_LOCK_VIOLATION);
            CASE(ERROR_WRONG_DISK);
            CASE(ERROR_SHARING_BUFFER_EXCEEDED);
            CASE(ERROR_HANDLE_EOF);
            CASE(ERROR_HANDLE_DISK_FULL);
            CASE(ERROR_NOT_SUPPORTED);
            CASE(ERROR_REM_NOT_LIST);
            CASE(ERROR_DUP_NAME);
            CASE(ERROR_BAD_NETPATH);
            CASE(ERROR_NETWORK_BUSY);
            CASE(ERROR_DEV_NOT_EXIST);
            CASE(ERROR_TOO_MANY_CMDS);
            CASE(ERROR_ADAP_HDW_ERR);
            CASE(ERROR_BAD_NET_RESP);
            CASE(ERROR_UNEXP_NET_ERR);
            CASE(ERROR_BAD_REM_ADAP);
            CASE(ERROR_PRINTQ_FULL);
            CASE(ERROR_NO_SPOOL_SPACE);
            CASE(ERROR_PRINT_CANCELLED);
            CASE(ERROR_NETNAME_DELETED);
            CASE(ERROR_NETWORK_ACCESS_DENIED);
            CASE(ERROR_BAD_DEV_TYPE);
            CASE(ERROR_BAD_NET_NAME);
            CASE(ERROR_TOO_MANY_NAMES);
            CASE(ERROR_TOO_MANY_SESS);
            CASE(ERROR_SHARING_PAUSED);
            CASE(ERROR_REQ_NOT_ACCEP);
            CASE(ERROR_REDIR_PAUSED);
            CASE(ERROR_FILE_EXISTS);
            CASE(ERROR_CANNOT_MAKE);
            CASE(ERROR_FAIL_I24);
            CASE(ERROR_OUT_OF_STRUCTURES);
            CASE(ERROR_ALREADY_ASSIGNED);
            CASE(ERROR_INVALID_PASSWORD);
            CASE(ERROR_INVALID_PARAMETER);
            CASE(ERROR_EA_ACCESS_DENIED);
            CASE(ERROR_OPERATION_ABORTED);
            CASE(ERROR_IO_INCOMPLETE);
            CASE(ERROR_IO_PENDING);
            CASE(ERROR_NOACCESS);
            CASE(ERROR_SWAPERROR);
            CASE(ERROR_STACK_OVERFLOW);
            CASE(ERROR_INVALID_MESSAGE);
            CASE(ERROR_CAN_NOT_COMPLETE);
            CASE(ERROR_INVALID_FLAGS);
            CASE(ERROR_ABANDONED_WAIT_0);
            CASE(ERROR_INVALID_ADDRESS);

            // WinSock.h
            CASE(WSAEINTR);
            CASE(WSAEBADF);
            CASE(WSAEACCES);
            CASE(WSAEFAULT);
            CASE(WSAEINVAL);
            CASE(WSAEMFILE);
            CASE(WSAEWOULDBLOCK);
            CASE(WSAEINPROGRESS);
            CASE(WSAEALREADY);
            CASE(WSAENOTSOCK);
            CASE(WSAEDESTADDRREQ);
            CASE(WSAEMSGSIZE);
            CASE(WSAEPROTOTYPE);
            CASE(WSAENOPROTOOPT);
            CASE(WSAEPROTONOSUPPORT);
            CASE(WSAESOCKTNOSUPPORT);
            CASE(WSAEOPNOTSUPP);
            CASE(WSAEPFNOSUPPORT);
            CASE(WSAEAFNOSUPPORT);
            CASE(WSAEADDRINUSE);
            CASE(WSAEADDRNOTAVAIL);
            CASE(WSAENETDOWN);
            CASE(WSAENETUNREACH);
            CASE(WSAENETRESET);
            CASE(WSAECONNABORTED);
            CASE(WSAECONNRESET);
            CASE(WSAENOBUFS);
            CASE(WSAEISCONN);
            CASE(WSAENOTCONN);
            CASE(WSAESHUTDOWN);
            CASE(WSAETOOMANYREFS);
            CASE(WSAETIMEDOUT);
            CASE(WSAECONNREFUSED);
            CASE(WSAELOOP);
            CASE(WSAENAMETOOLONG);
            CASE(WSAEHOSTDOWN);
            CASE(WSAEHOSTUNREACH);
            CASE(WSAENOTEMPTY);
            CASE(WSAEPROCLIM);
            CASE(WSAEUSERS);
            CASE(WSAEDQUOT);
            CASE(WSAESTALE);
            CASE(WSAEREMOTE);
            CASE(WSAEDISCON);
            CASE(WSASYSNOTREADY);
            CASE(WSAVERNOTSUPPORTED);
            CASE(WSANOTINITIALISED);
            CASE(WSAHOST_NOT_FOUND);
            CASE(WSATRY_AGAIN);
            CASE(WSANO_RECOVERY);
            CASE(WSANO_DATA);
            CASE(WSA_SECURE_HOST_NOT_FOUND);
            CASE(WSA_IPSEC_NAME_POLICY_ERROR);

            // ntstatus.h 0xC0000001 - 0xC0000123
            CASE(STATUS_UNSUCCESSFUL);
            CASE(STATUS_NOT_IMPLEMENTED);
            CASE(STATUS_INVALID_INFO_CLASS);
            CASE(STATUS_INFO_LENGTH_MISMATCH);
            CASE(STATUS_ACCESS_VIOLATION);
            CASE(STATUS_IN_PAGE_ERROR);
            CASE(STATUS_PAGEFILE_QUOTA);
            CASE(STATUS_INVALID_HANDLE);
            CASE(STATUS_BAD_INITIAL_STACK);
            CASE(STATUS_BAD_INITIAL_PC);
            CASE(STATUS_INVALID_CID);
            CASE(STATUS_TIMER_NOT_CANCELED);
            CASE(STATUS_INVALID_PARAMETER);
            CASE(STATUS_NO_SUCH_DEVICE);
            CASE(STATUS_NO_SUCH_FILE);
            CASE(STATUS_INVALID_DEVICE_REQUEST);
            CASE(STATUS_END_OF_FILE);
            CASE(STATUS_WRONG_VOLUME);
            CASE(STATUS_NO_MEDIA_IN_DEVICE);
            CASE(STATUS_UNRECOGNIZED_MEDIA);
            CASE(STATUS_NONEXISTENT_SECTOR);
            CASE(STATUS_MORE_PROCESSING_REQUIRED);
            CASE(STATUS_NO_MEMORY);
            CASE(STATUS_CONFLICTING_ADDRESSES);
            CASE(STATUS_NOT_MAPPED_VIEW);
            CASE(STATUS_UNABLE_TO_FREE_VM);
            CASE(STATUS_UNABLE_TO_DELETE_SECTION);
            CASE(STATUS_INVALID_SYSTEM_SERVICE);
            CASE(STATUS_ILLEGAL_INSTRUCTION);
            CASE(STATUS_INVALID_LOCK_SEQUENCE);
            CASE(STATUS_INVALID_VIEW_SIZE);
            CASE(STATUS_INVALID_FILE_FOR_SECTION);
            CASE(STATUS_ALREADY_COMMITTED);
            CASE(STATUS_ACCESS_DENIED);
            CASE(STATUS_BUFFER_TOO_SMALL);
            CASE(STATUS_OBJECT_TYPE_MISMATCH);
            CASE(STATUS_NONCONTINUABLE_EXCEPTION);
            CASE(STATUS_INVALID_DISPOSITION);
            CASE(STATUS_UNWIND);
            CASE(STATUS_BAD_STACK);
            CASE(STATUS_INVALID_UNWIND_TARGET);
            CASE(STATUS_NOT_LOCKED);
            CASE(STATUS_PARITY_ERROR);
            CASE(STATUS_UNABLE_TO_DECOMMIT_VM);
            CASE(STATUS_NOT_COMMITTED);
            CASE(STATUS_INVALID_PORT_ATTRIBUTES);
            CASE(STATUS_PORT_MESSAGE_TOO_LONG);
            CASE(STATUS_INVALID_PARAMETER_MIX);
            CASE(STATUS_INVALID_QUOTA_LOWER);
            CASE(STATUS_DISK_CORRUPT_ERROR);
            CASE(STATUS_OBJECT_NAME_INVALID);
            CASE(STATUS_OBJECT_NAME_NOT_FOUND);
            CASE(STATUS_OBJECT_NAME_COLLISION);
            CASE(STATUS_PORT_DISCONNECTED);
            CASE(STATUS_DEVICE_ALREADY_ATTACHED);
            CASE(STATUS_OBJECT_PATH_INVALID);
            CASE(STATUS_OBJECT_PATH_NOT_FOUND);
            CASE(STATUS_OBJECT_PATH_SYNTAX_BAD);
            CASE(STATUS_DATA_OVERRUN);
            CASE(STATUS_DATA_LATE_ERROR);
            CASE(STATUS_DATA_ERROR);
            CASE(STATUS_CRC_ERROR);
            CASE(STATUS_SECTION_TOO_BIG);
            CASE(STATUS_PORT_CONNECTION_REFUSED);
            CASE(STATUS_INVALID_PORT_HANDLE);
            CASE(STATUS_SHARING_VIOLATION);
            CASE(STATUS_QUOTA_EXCEEDED);
            CASE(STATUS_INVALID_PAGE_PROTECTION);
            CASE(STATUS_MUTANT_NOT_OWNED);
            CASE(STATUS_SEMAPHORE_LIMIT_EXCEEDED);
            CASE(STATUS_PORT_ALREADY_SET);
            CASE(STATUS_SECTION_NOT_IMAGE);
            CASE(STATUS_SUSPEND_COUNT_EXCEEDED);
            CASE(STATUS_THREAD_IS_TERMINATING);
            CASE(STATUS_BAD_WORKING_SET_LIMIT);
            CASE(STATUS_INCOMPATIBLE_FILE_MAP);
            CASE(STATUS_SECTION_PROTECTION);
            CASE(STATUS_EAS_NOT_SUPPORTED);
            CASE(STATUS_EA_TOO_LARGE);
            CASE(STATUS_NONEXISTENT_EA_ENTRY);
            CASE(STATUS_NO_EAS_ON_FILE);
            CASE(STATUS_EA_CORRUPT_ERROR);
            CASE(STATUS_FILE_LOCK_CONFLICT);
            CASE(STATUS_LOCK_NOT_GRANTED);
            CASE(STATUS_DELETE_PENDING);
            CASE(STATUS_CTL_FILE_NOT_SUPPORTED);
            CASE(STATUS_UNKNOWN_REVISION);
            CASE(STATUS_REVISION_MISMATCH);
            CASE(STATUS_INVALID_OWNER);
            CASE(STATUS_INVALID_PRIMARY_GROUP);
            CASE(STATUS_NO_IMPERSONATION_TOKEN);
            CASE(STATUS_CANT_DISABLE_MANDATORY);
            CASE(STATUS_NO_LOGON_SERVERS);
            CASE(STATUS_NO_SUCH_LOGON_SESSION);
            CASE(STATUS_NO_SUCH_PRIVILEGE);
            CASE(STATUS_PRIVILEGE_NOT_HELD);
            CASE(STATUS_INVALID_ACCOUNT_NAME);
            CASE(STATUS_USER_EXISTS);
            CASE(STATUS_NO_SUCH_USER);
            CASE(STATUS_GROUP_EXISTS);
            CASE(STATUS_NO_SUCH_GROUP);
            CASE(STATUS_MEMBER_IN_GROUP);
            CASE(STATUS_MEMBER_NOT_IN_GROUP);
            CASE(STATUS_LAST_ADMIN);
            CASE(STATUS_WRONG_PASSWORD);
            CASE(STATUS_ILL_FORMED_PASSWORD);
            CASE(STATUS_PASSWORD_RESTRICTION);
            CASE(STATUS_LOGON_FAILURE);
            CASE(STATUS_ACCOUNT_RESTRICTION);
            CASE(STATUS_INVALID_LOGON_HOURS);
            CASE(STATUS_INVALID_WORKSTATION);
            CASE(STATUS_PASSWORD_EXPIRED);
            CASE(STATUS_ACCOUNT_DISABLED);
            CASE(STATUS_NONE_MAPPED);
            CASE(STATUS_TOO_MANY_LUIDS_REQUESTED);
            CASE(STATUS_LUIDS_EXHAUSTED);
            CASE(STATUS_INVALID_SUB_AUTHORITY);
            CASE(STATUS_INVALID_ACL);
            CASE(STATUS_INVALID_SID);
            CASE(STATUS_INVALID_SECURITY_DESCR);
            CASE(STATUS_PROCEDURE_NOT_FOUND);
            CASE(STATUS_INVALID_IMAGE_FORMAT);
            CASE(STATUS_NO_TOKEN);
            CASE(STATUS_BAD_INHERITANCE_ACL);
            CASE(STATUS_RANGE_NOT_LOCKED);
            CASE(STATUS_DISK_FULL);
            CASE(STATUS_SERVER_DISABLED);
            CASE(STATUS_SERVER_NOT_DISABLED);
            CASE(STATUS_TOO_MANY_GUIDS_REQUESTED);
            CASE(STATUS_GUIDS_EXHAUSTED);
            CASE(STATUS_INVALID_ID_AUTHORITY);
            CASE(STATUS_AGENTS_EXHAUSTED);
            CASE(STATUS_INVALID_VOLUME_LABEL);
            CASE(STATUS_SECTION_NOT_EXTENDED);
            CASE(STATUS_NOT_MAPPED_DATA);
            CASE(STATUS_RESOURCE_DATA_NOT_FOUND);
            CASE(STATUS_RESOURCE_TYPE_NOT_FOUND);
            CASE(STATUS_RESOURCE_NAME_NOT_FOUND);
            CASE(STATUS_ARRAY_BOUNDS_EXCEEDED);
            CASE(STATUS_FLOAT_DENORMAL_OPERAND);
            CASE(STATUS_FLOAT_DIVIDE_BY_ZERO);
            CASE(STATUS_FLOAT_INEXACT_RESULT);
            CASE(STATUS_FLOAT_INVALID_OPERATION);
            CASE(STATUS_FLOAT_OVERFLOW);
            CASE(STATUS_FLOAT_STACK_CHECK);
            CASE(STATUS_FLOAT_UNDERFLOW);
            CASE(STATUS_INTEGER_DIVIDE_BY_ZERO);
            CASE(STATUS_INTEGER_OVERFLOW);
            CASE(STATUS_PRIVILEGED_INSTRUCTION);
            CASE(STATUS_TOO_MANY_PAGING_FILES);
            CASE(STATUS_FILE_INVALID);
            CASE(STATUS_ALLOTTED_SPACE_EXCEEDED);
            CASE(STATUS_INSUFFICIENT_RESOURCES);
            CASE(STATUS_DFS_EXIT_PATH_FOUND);
            CASE(STATUS_DEVICE_DATA_ERROR);
            CASE(STATUS_DEVICE_NOT_CONNECTED);
            CASE(STATUS_DEVICE_POWER_FAILURE);
            CASE(STATUS_FREE_VM_NOT_AT_BASE);
            CASE(STATUS_MEMORY_NOT_ALLOCATED);
            CASE(STATUS_WORKING_SET_QUOTA);
            CASE(STATUS_MEDIA_WRITE_PROTECTED);
            CASE(STATUS_DEVICE_NOT_READY);
            CASE(STATUS_INVALID_GROUP_ATTRIBUTES);
            CASE(STATUS_BAD_IMPERSONATION_LEVEL);
            CASE(STATUS_CANT_OPEN_ANONYMOUS);
            CASE(STATUS_BAD_VALIDATION_CLASS);
            CASE(STATUS_BAD_TOKEN_TYPE);
            CASE(STATUS_BAD_MASTER_BOOT_RECORD);
            CASE(STATUS_INSTRUCTION_MISALIGNMENT);
            CASE(STATUS_INSTANCE_NOT_AVAILABLE);
            CASE(STATUS_PIPE_NOT_AVAILABLE);
            CASE(STATUS_INVALID_PIPE_STATE);
            CASE(STATUS_PIPE_BUSY);
            CASE(STATUS_ILLEGAL_FUNCTION);
            CASE(STATUS_PIPE_DISCONNECTED);
            CASE(STATUS_PIPE_CLOSING);
            CASE(STATUS_PIPE_CONNECTED);
            CASE(STATUS_PIPE_LISTENING);
            CASE(STATUS_INVALID_READ_MODE);
            CASE(STATUS_IO_TIMEOUT);
            CASE(STATUS_FILE_FORCED_CLOSED);
            CASE(STATUS_PROFILING_NOT_STARTED);
            CASE(STATUS_PROFILING_NOT_STOPPED);
            CASE(STATUS_COULD_NOT_INTERPRET);
            CASE(STATUS_FILE_IS_A_DIRECTORY);
            CASE(STATUS_NOT_SUPPORTED);
            CASE(STATUS_REMOTE_NOT_LISTENING);
            CASE(STATUS_DUPLICATE_NAME);
            CASE(STATUS_BAD_NETWORK_PATH);
            CASE(STATUS_NETWORK_BUSY);
            CASE(STATUS_DEVICE_DOES_NOT_EXIST);
            CASE(STATUS_TOO_MANY_COMMANDS);
            CASE(STATUS_ADAPTER_HARDWARE_ERROR);
            CASE(STATUS_INVALID_NETWORK_RESPONSE);
            CASE(STATUS_UNEXPECTED_NETWORK_ERROR);
            CASE(STATUS_BAD_REMOTE_ADAPTER);
            CASE(STATUS_PRINT_QUEUE_FULL);
            CASE(STATUS_NO_SPOOL_SPACE);
            CASE(STATUS_PRINT_CANCELLED);
            CASE(STATUS_NETWORK_NAME_DELETED);
            CASE(STATUS_NETWORK_ACCESS_DENIED);
            CASE(STATUS_BAD_DEVICE_TYPE);
            CASE(STATUS_BAD_NETWORK_NAME);
            CASE(STATUS_TOO_MANY_NAMES);
            CASE(STATUS_TOO_MANY_SESSIONS);
            CASE(STATUS_SHARING_PAUSED);
            CASE(STATUS_REQUEST_NOT_ACCEPTED);
            CASE(STATUS_REDIRECTOR_PAUSED);
            CASE(STATUS_NET_WRITE_FAULT);
            CASE(STATUS_PROFILING_AT_LIMIT);
            CASE(STATUS_NOT_SAME_DEVICE);
            CASE(STATUS_FILE_RENAMED);
            CASE(STATUS_VIRTUAL_CIRCUIT_CLOSED);
            CASE(STATUS_NO_SECURITY_ON_OBJECT);
            CASE(STATUS_CANT_WAIT);
            CASE(STATUS_PIPE_EMPTY);
            CASE(STATUS_CANT_ACCESS_DOMAIN_INFO);
            CASE(STATUS_CANT_TERMINATE_SELF);
            CASE(STATUS_INVALID_SERVER_STATE);
            CASE(STATUS_INVALID_DOMAIN_STATE);
            CASE(STATUS_INVALID_DOMAIN_ROLE);
            CASE(STATUS_NO_SUCH_DOMAIN);
            CASE(STATUS_DOMAIN_EXISTS);
            CASE(STATUS_DOMAIN_LIMIT_EXCEEDED);
            CASE(STATUS_OPLOCK_NOT_GRANTED);
            CASE(STATUS_INVALID_OPLOCK_PROTOCOL);
            CASE(STATUS_INTERNAL_DB_CORRUPTION);
            CASE(STATUS_INTERNAL_ERROR);
            CASE(STATUS_GENERIC_NOT_MAPPED);
            CASE(STATUS_BAD_DESCRIPTOR_FORMAT);
            CASE(STATUS_INVALID_USER_BUFFER);
            CASE(STATUS_UNEXPECTED_IO_ERROR);
            CASE(STATUS_UNEXPECTED_MM_CREATE_ERR);
            CASE(STATUS_UNEXPECTED_MM_MAP_ERROR);
            CASE(STATUS_UNEXPECTED_MM_EXTEND_ERR);
            CASE(STATUS_NOT_LOGON_PROCESS);
            CASE(STATUS_LOGON_SESSION_EXISTS);
            CASE(STATUS_INVALID_PARAMETER_1);
            CASE(STATUS_INVALID_PARAMETER_2);
            CASE(STATUS_INVALID_PARAMETER_3);
            CASE(STATUS_INVALID_PARAMETER_4);
            CASE(STATUS_INVALID_PARAMETER_5);
            CASE(STATUS_INVALID_PARAMETER_6);
            CASE(STATUS_INVALID_PARAMETER_7);
            CASE(STATUS_INVALID_PARAMETER_8);
            CASE(STATUS_INVALID_PARAMETER_9);
            CASE(STATUS_INVALID_PARAMETER_10);
            CASE(STATUS_INVALID_PARAMETER_11);
            CASE(STATUS_INVALID_PARAMETER_12);
            CASE(STATUS_REDIRECTOR_NOT_STARTED);
            CASE(STATUS_REDIRECTOR_STARTED);
            CASE(STATUS_STACK_OVERFLOW);
            CASE(STATUS_NO_SUCH_PACKAGE);
            CASE(STATUS_BAD_FUNCTION_TABLE);
            CASE(STATUS_VARIABLE_NOT_FOUND);
            CASE(STATUS_DIRECTORY_NOT_EMPTY);
            CASE(STATUS_FILE_CORRUPT_ERROR);
            CASE(STATUS_NOT_A_DIRECTORY);
            CASE(STATUS_BAD_LOGON_SESSION_STATE);
            CASE(STATUS_LOGON_SESSION_COLLISION);
            CASE(STATUS_NAME_TOO_LONG);
            CASE(STATUS_FILES_OPEN);
            CASE(STATUS_CONNECTION_IN_USE);
            CASE(STATUS_MESSAGE_NOT_FOUND);
            CASE(STATUS_PROCESS_IS_TERMINATING);
            CASE(STATUS_INVALID_LOGON_TYPE);
            CASE(STATUS_NO_GUID_TRANSLATION);
            CASE(STATUS_CANNOT_IMPERSONATE);
            CASE(STATUS_IMAGE_ALREADY_LOADED);
            CASE(STATUS_ABIOS_NOT_PRESENT);
            CASE(STATUS_ABIOS_LID_NOT_EXIST);
            CASE(STATUS_ABIOS_LID_ALREADY_OWNED);
            CASE(STATUS_ABIOS_NOT_LID_OWNER);
            CASE(STATUS_ABIOS_INVALID_COMMAND);
            CASE(STATUS_ABIOS_INVALID_LID);
            CASE(STATUS_ABIOS_SELECTOR_NOT_AVAILABLE);
            CASE(STATUS_ABIOS_INVALID_SELECTOR);
            CASE(STATUS_NO_LDT);
            CASE(STATUS_INVALID_LDT_SIZE);
            CASE(STATUS_INVALID_LDT_OFFSET);
            CASE(STATUS_INVALID_LDT_DESCRIPTOR);
            CASE(STATUS_INVALID_IMAGE_NE_FORMAT);
            CASE(STATUS_RXACT_INVALID_STATE);
            CASE(STATUS_RXACT_COMMIT_FAILURE);
            CASE(STATUS_MAPPED_FILE_SIZE_ZERO);
            CASE(STATUS_TOO_MANY_OPENED_FILES);
            CASE(STATUS_CANCELLED);
            CASE(STATUS_CANNOT_DELETE);
            CASE(STATUS_INVALID_COMPUTER_NAME);
            CASE(STATUS_FILE_DELETED);

            // ntstatus.h 0xC0000205 - 0xC0000217
            CASE(STATUS_INSUFF_SERVER_RESOURCES);
            CASE(STATUS_INVALID_BUFFER_SIZE);
            CASE(STATUS_INVALID_ADDRESS_COMPONENT);
            CASE(STATUS_INVALID_ADDRESS_WILDCARD);
            CASE(STATUS_TOO_MANY_ADDRESSES);
            CASE(STATUS_ADDRESS_ALREADY_EXISTS);
            CASE(STATUS_ADDRESS_CLOSED);
            CASE(STATUS_CONNECTION_DISCONNECTED);
            CASE(STATUS_CONNECTION_RESET);
            CASE(STATUS_TOO_MANY_NODES);
            CASE(STATUS_TRANSACTION_ABORTED);
            CASE(STATUS_TRANSACTION_TIMED_OUT);
            CASE(STATUS_TRANSACTION_NO_RELEASE);
            CASE(STATUS_TRANSACTION_NO_MATCH);
            CASE(STATUS_TRANSACTION_RESPONDED);
            CASE(STATUS_TRANSACTION_INVALID_ID);
            CASE(STATUS_TRANSACTION_INVALID_TYPE);
            CASE(STATUS_NOT_SERVER_SESSION);
            CASE(STATUS_NOT_CLIENT_SESSION);

            // ntstatus.h 0xC0000234 - 0xC0000241
            CASE(STATUS_ACCOUNT_LOCKED_OUT);
            CASE(STATUS_HANDLE_NOT_CLOSABLE);
            CASE(STATUS_CONNECTION_REFUSED);
            CASE(STATUS_GRACEFUL_DISCONNECT);
            CASE(STATUS_ADDRESS_ALREADY_ASSOCIATED);
            CASE(STATUS_ADDRESS_NOT_ASSOCIATED);
            CASE(STATUS_CONNECTION_INVALID);
            CASE(STATUS_CONNECTION_ACTIVE);
            CASE(STATUS_NETWORK_UNREACHABLE);
            CASE(STATUS_HOST_UNREACHABLE);
            CASE(STATUS_PROTOCOL_UNREACHABLE);
            CASE(STATUS_PORT_UNREACHABLE);
            CASE(STATUS_REQUEST_ABORTED);
            CASE(STATUS_CONNECTION_ABORTED);

            // crypto
            CASE(NTE_BAD_UID);
            CASE(NTE_BAD_HASH);
            CASE(NTE_BAD_KEY);
            CASE(NTE_BAD_LEN);
            CASE(NTE_BAD_DATA);
            CASE(NTE_BAD_SIGNATURE);
            CASE(NTE_BAD_VER);
            CASE(NTE_BAD_ALGID);
            CASE(NTE_BAD_FLAGS);
            CASE(NTE_BAD_TYPE);
            CASE(NTE_BAD_KEY_STATE);
            CASE(NTE_BAD_HASH_STATE);
            CASE(NTE_NO_KEY);
            CASE(NTE_NO_MEMORY);
            CASE(NTE_EXISTS);
            CASE(NTE_PERM);
            CASE(NTE_NOT_FOUND);
            CASE(NTE_DOUBLE_ENCRYPT);
            CASE(NTE_BAD_PROVIDER);
            CASE(NTE_BAD_PROV_TYPE);
            CASE(NTE_BAD_PUBLIC_KEY);
            CASE(NTE_BAD_KEYSET);
            CASE(NTE_PROV_TYPE_NOT_DEF);
            CASE(NTE_PROV_TYPE_ENTRY_BAD);
            CASE(NTE_KEYSET_NOT_DEF);
            CASE(NTE_KEYSET_ENTRY_BAD);
            CASE(NTE_PROV_TYPE_NO_MATCH);
            CASE(NTE_SIGNATURE_FILE_BAD);
            CASE(NTE_PROVIDER_DLL_FAIL);
            CASE(NTE_PROV_DLL_NOT_FOUND);
            CASE(NTE_BAD_KEYSET_PARAM);
            CASE(NTE_FAIL);
            CASE(NTE_SYS_ERR);
            CASE(NTE_SILENT_CONTEXT);
            CASE(NTE_TOKEN_KEYSET_STORAGE_FULL);
            CASE(NTE_TEMPORARY_PROFILE);
            CASE(NTE_FIXEDPARAMETER);
            CASE(NTE_INVALID_HANDLE);
            CASE(NTE_INVALID_PARAMETER);
            CASE(NTE_BUFFER_TOO_SMALL);
            CASE(NTE_NOT_SUPPORTED);
            CASE(NTE_NO_MORE_ITEMS);
            CASE(NTE_BUFFERS_OVERLAP);
            CASE(NTE_DECRYPTION_FAILURE);
            CASE(NTE_INTERNAL_ERROR);
            CASE(NTE_UI_REQUIRED);
            CASE(NTE_HMAC_NOT_SUPPORTED);
            CASE(NTE_DEVICE_NOT_READY);
            CASE(NTE_AUTHENTICATION_IGNORED);
            CASE(NTE_VALIDATION_FAILED);
            CASE(NTE_INCORRECT_PASSWORD);
            CASE(NTE_ENCRYPTION_FAILURE);
            CASE(SEC_E_INSUFFICIENT_MEMORY);
            CASE(SEC_E_INVALID_HANDLE);
            CASE(SEC_E_UNSUPPORTED_FUNCTION);
            CASE(SEC_E_TARGET_UNKNOWN);
            CASE(SEC_E_INTERNAL_ERROR);
            CASE(SEC_E_SECPKG_NOT_FOUND);
            CASE(SEC_E_NOT_OWNER);
            CASE(SEC_E_CANNOT_INSTALL);
            CASE(SEC_E_INVALID_TOKEN);
            CASE(SEC_E_CANNOT_PACK);
            CASE(SEC_E_QOP_NOT_SUPPORTED);
            CASE(SEC_E_NO_IMPERSONATION);
            CASE(SEC_E_LOGON_DENIED);
            CASE(SEC_E_UNKNOWN_CREDENTIALS);
            CASE(SEC_E_NO_CREDENTIALS);
            CASE(SEC_E_MESSAGE_ALTERED);
            CASE(SEC_E_OUT_OF_SEQUENCE);
            CASE(SEC_E_NO_AUTHENTICATING_AUTHORITY);
            CASE(SEC_I_CONTINUE_NEEDED);
            CASE(SEC_I_COMPLETE_NEEDED);
            CASE(SEC_I_COMPLETE_AND_CONTINUE);
            CASE(SEC_I_LOCAL_LOGON);
            CASE(SEC_E_BAD_PKGID);
            CASE(SEC_E_CONTEXT_EXPIRED);
            CASE(SEC_I_CONTEXT_EXPIRED);
            CASE(SEC_E_INCOMPLETE_MESSAGE);
            CASE(SEC_E_INCOMPLETE_CREDENTIALS);
            CASE(SEC_E_BUFFER_TOO_SMALL);
            CASE(SEC_I_INCOMPLETE_CREDENTIALS);
            CASE(SEC_I_RENEGOTIATE);
            CASE(SEC_E_WRONG_PRINCIPAL);
            CASE(SEC_I_NO_LSA_CONTEXT);
            CASE(SEC_E_TIME_SKEW);
            CASE(SEC_E_UNTRUSTED_ROOT);
            CASE(SEC_E_ILLEGAL_MESSAGE);
            CASE(SEC_E_CERT_UNKNOWN);
            CASE(SEC_E_CERT_EXPIRED);
            CASE(SEC_E_ENCRYPT_FAILURE);
            CASE(SEC_E_DECRYPT_FAILURE);
            CASE(SEC_E_ALGORITHM_MISMATCH);
            CASE(SEC_E_SECURITY_QOS_FAILED);
            CASE(SEC_E_UNFINISHED_CONTEXT_DELETED);
            CASE(SEC_E_NO_TGT_REPLY);
            CASE(SEC_E_NO_IP_ADDRESSES);
            CASE(SEC_E_WRONG_CREDENTIAL_HANDLE);
            CASE(SEC_E_CRYPTO_SYSTEM_INVALID);
            CASE(SEC_E_MAX_REFERRALS_EXCEEDED);
            CASE(SEC_E_MUST_BE_KDC);
            CASE(SEC_E_STRONG_CRYPTO_NOT_SUPPORTED);
            CASE(SEC_E_TOO_MANY_PRINCIPALS);
            CASE(SEC_E_NO_PA_DATA);
            CASE(SEC_E_PKINIT_NAME_MISMATCH);
            CASE(SEC_E_SMARTCARD_LOGON_REQUIRED);
            CASE(SEC_E_SHUTDOWN_IN_PROGRESS);
            CASE(SEC_E_KDC_INVALID_REQUEST);
            CASE(SEC_E_KDC_UNABLE_TO_REFER);
            CASE(SEC_E_KDC_UNKNOWN_ETYPE);
            CASE(SEC_E_UNSUPPORTED_PREAUTH);
            CASE(SEC_E_DELEGATION_REQUIRED);
            CASE(SEC_E_BAD_BINDINGS);
            CASE(SEC_E_MULTIPLE_ACCOUNTS);
            CASE(SEC_E_NO_KERB_KEY);
            CASE(SEC_E_CERT_WRONG_USAGE);
            CASE(SEC_E_DOWNGRADE_DETECTED);
            CASE(SEC_E_SMARTCARD_CERT_REVOKED);
            CASE(SEC_E_ISSUING_CA_UNTRUSTED);
            CASE(SEC_E_REVOCATION_OFFLINE_C);
            CASE(SEC_E_PKINIT_CLIENT_FAILURE);
            CASE(SEC_E_SMARTCARD_CERT_EXPIRED);
            CASE(SEC_E_NO_S4U_PROT_SUPPORT);
            CASE(SEC_E_CROSSREALM_DELEGATION_FAILURE);
            CASE(SEC_E_REVOCATION_OFFLINE_KDC);
            CASE(SEC_E_ISSUING_CA_UNTRUSTED_KDC);
            CASE(SEC_E_KDC_CERT_EXPIRED);
            CASE(SEC_E_KDC_CERT_REVOKED);
            CASE(SEC_I_SIGNATURE_NEEDED);
            CASE(SEC_E_INVALID_PARAMETER);
            CASE(SEC_E_DELEGATION_POLICY);
            CASE(SEC_E_POLICY_NLTM_ONLY);
            CASE(SEC_I_NO_RENEGOTIATION);
            CASE(SEC_E_NO_CONTEXT);
            CASE(SEC_E_PKU2U_CERT_FAILURE);
            CASE(SEC_E_MUTUAL_AUTH_FAILED);
            CASE(SEC_I_MESSAGE_FRAGMENT);
            CASE(SEC_E_ONLY_HTTPS_ALLOWED);
            CASE(SEC_I_CONTINUE_NEEDED_MESSAGE_OK);
            CASE(CRYPT_E_MSG_ERROR);
            CASE(CRYPT_E_UNKNOWN_ALGO);
            CASE(CRYPT_E_OID_FORMAT);
            CASE(CRYPT_E_INVALID_MSG_TYPE);
            CASE(CRYPT_E_UNEXPECTED_ENCODING);
            CASE(CRYPT_E_AUTH_ATTR_MISSING);
            CASE(CRYPT_E_HASH_VALUE);
            CASE(CRYPT_E_INVALID_INDEX);
            CASE(CRYPT_E_ALREADY_DECRYPTED);
            CASE(CRYPT_E_NOT_DECRYPTED);
            CASE(CRYPT_E_RECIPIENT_NOT_FOUND);
            CASE(CRYPT_E_CONTROL_TYPE);
            CASE(CRYPT_E_ISSUER_SERIALNUMBER);
            CASE(CRYPT_E_SIGNER_NOT_FOUND);
            CASE(CRYPT_E_ATTRIBUTES_MISSING);
            CASE(CRYPT_E_STREAM_MSG_NOT_READY);
            CASE(CRYPT_E_STREAM_INSUFFICIENT_DATA);
            CASE(CRYPT_I_NEW_PROTECTION_REQUIRED);
            CASE(CRYPT_E_BAD_LEN);
            CASE(CRYPT_E_BAD_ENCODE);
            CASE(CRYPT_E_FILE_ERROR);
            CASE(CRYPT_E_NOT_FOUND);
            CASE(CRYPT_E_EXISTS);
            CASE(CRYPT_E_NO_PROVIDER);
            CASE(CRYPT_E_SELF_SIGNED);
            CASE(CRYPT_E_DELETED_PREV);
            CASE(CRYPT_E_NO_MATCH);
            CASE(CRYPT_E_UNEXPECTED_MSG_TYPE);
            CASE(CRYPT_E_NO_KEY_PROPERTY);
            CASE(CRYPT_E_NO_DECRYPT_CERT);
            CASE(CRYPT_E_BAD_MSG);
            CASE(CRYPT_E_NO_SIGNER);
            CASE(CRYPT_E_PENDING_CLOSE);
            CASE(CRYPT_E_REVOKED);
            CASE(CRYPT_E_NO_REVOCATION_DLL);
            CASE(CRYPT_E_NO_REVOCATION_CHECK);
            CASE(CRYPT_E_REVOCATION_OFFLINE);
            CASE(CRYPT_E_NOT_IN_REVOCATION_DATABASE);
            CASE(CRYPT_E_INVALID_NUMERIC_STRING);
            CASE(CRYPT_E_INVALID_PRINTABLE_STRING);
            CASE(CRYPT_E_INVALID_IA5_STRING);
            CASE(CRYPT_E_INVALID_X500_STRING);
            CASE(CRYPT_E_NOT_CHAR_STRING);
            CASE(CRYPT_E_FILERESIZED);
            CASE(CRYPT_E_SECURITY_SETTINGS);
            CASE(CRYPT_E_NO_VERIFY_USAGE_DLL);
            CASE(CRYPT_E_NO_VERIFY_USAGE_CHECK);
            CASE(CRYPT_E_VERIFY_USAGE_OFFLINE);
            CASE(CRYPT_E_NOT_IN_CTL);
            CASE(CRYPT_E_NO_TRUSTED_SIGNER);
            CASE(CRYPT_E_MISSING_PUBKEY_PARA);
            CASE(CRYPT_E_OBJECT_LOCATOR_OBJECT_NOT_FOUND);
            CASE(CRYPT_E_OSS_ERROR);
            CASE(OSS_MORE_BUF);
            CASE(OSS_NEGATIVE_UINTEGER);
            CASE(OSS_PDU_RANGE);
            CASE(OSS_MORE_INPUT);
            CASE(OSS_DATA_ERROR);
            CASE(OSS_BAD_ARG);
            CASE(OSS_BAD_VERSION);
            CASE(OSS_OUT_MEMORY);
            CASE(OSS_PDU_MISMATCH);
            CASE(OSS_LIMITED);
            CASE(OSS_BAD_PTR);
            CASE(OSS_BAD_TIME);
            CASE(OSS_INDEFINITE_NOT_SUPPORTED);
            CASE(OSS_MEM_ERROR);
            CASE(OSS_BAD_TABLE);
            CASE(OSS_TOO_LONG);
            CASE(OSS_CONSTRAINT_VIOLATED);
            CASE(OSS_FATAL_ERROR);
            CASE(OSS_ACCESS_SERIALIZATION_ERROR);
            CASE(OSS_NULL_TBL);
            CASE(OSS_NULL_FCN);
            CASE(OSS_BAD_ENCRULES);
            CASE(OSS_UNAVAIL_ENCRULES);
            CASE(OSS_CANT_OPEN_TRACE_WINDOW);
            CASE(OSS_UNIMPLEMENTED);
            CASE(OSS_OID_DLL_NOT_LINKED);
            CASE(OSS_CANT_OPEN_TRACE_FILE);
            CASE(OSS_TRACE_FILE_ALREADY_OPEN);
            CASE(OSS_TABLE_MISMATCH);
            CASE(OSS_TYPE_NOT_SUPPORTED);
            CASE(OSS_REAL_DLL_NOT_LINKED);
            CASE(OSS_REAL_CODE_NOT_LINKED);
            CASE(OSS_OUT_OF_RANGE);
            CASE(OSS_COPIER_DLL_NOT_LINKED);
            CASE(OSS_CONSTRAINT_DLL_NOT_LINKED);
            CASE(OSS_COMPARATOR_DLL_NOT_LINKED);
            CASE(OSS_COMPARATOR_CODE_NOT_LINKED);
            CASE(OSS_MEM_MGR_DLL_NOT_LINKED);
            CASE(OSS_PDV_DLL_NOT_LINKED);
            CASE(OSS_PDV_CODE_NOT_LINKED);
            CASE(OSS_API_DLL_NOT_LINKED);
            CASE(OSS_BERDER_DLL_NOT_LINKED);
            CASE(OSS_PER_DLL_NOT_LINKED);
            CASE(OSS_OPEN_TYPE_ERROR);
            CASE(OSS_MUTEX_NOT_CREATED);
            CASE(OSS_CANT_CLOSE_TRACE_FILE);
            CASE(CRYPT_E_ASN1_ERROR);
            CASE(CRYPT_E_ASN1_INTERNAL);
            CASE(CRYPT_E_ASN1_EOD);
            CASE(CRYPT_E_ASN1_CORRUPT);
            CASE(CRYPT_E_ASN1_LARGE);
            CASE(CRYPT_E_ASN1_CONSTRAINT);
            CASE(CRYPT_E_ASN1_MEMORY);
            CASE(CRYPT_E_ASN1_OVERFLOW);
            CASE(CRYPT_E_ASN1_BADPDU);
            CASE(CRYPT_E_ASN1_BADARGS);
            CASE(CRYPT_E_ASN1_BADREAL);
            CASE(CRYPT_E_ASN1_BADTAG);
            CASE(CRYPT_E_ASN1_CHOICE);
            CASE(CRYPT_E_ASN1_RULE);
            CASE(CRYPT_E_ASN1_UTF8);
            CASE(CRYPT_E_ASN1_PDU_TYPE);
            CASE(CRYPT_E_ASN1_NYI);
            CASE(CRYPT_E_ASN1_EXTENDED);
            CASE(CRYPT_E_ASN1_NOEOD);
            CASE(CERTSRV_E_BAD_REQUESTSUBJECT);
            CASE(CERTSRV_E_NO_REQUEST);
            CASE(CERTSRV_E_BAD_REQUESTSTATUS);
            CASE(CERTSRV_E_PROPERTY_EMPTY);
            CASE(CERTSRV_E_INVALID_CA_CERTIFICATE);
            CASE(CERTSRV_E_SERVER_SUSPENDED);
            CASE(CERTSRV_E_ENCODING_LENGTH);
            CASE(CERTSRV_E_ROLECONFLICT);
            CASE(CERTSRV_E_RESTRICTEDOFFICER);
            CASE(CERTSRV_E_KEY_ARCHIVAL_NOT_CONFIGURED);
            CASE(CERTSRV_E_NO_VALID_KRA);
            CASE(CERTSRV_E_BAD_REQUEST_KEY_ARCHIVAL);
            CASE(CERTSRV_E_NO_CAADMIN_DEFINED);
            CASE(CERTSRV_E_BAD_RENEWAL_CERT_ATTRIBUTE);
            CASE(CERTSRV_E_NO_DB_SESSIONS);
            CASE(CERTSRV_E_ALIGNMENT_FAULT);
            CASE(CERTSRV_E_ENROLL_DENIED);
            CASE(CERTSRV_E_TEMPLATE_DENIED);
            CASE(CERTSRV_E_DOWNLEVEL_DC_SSL_OR_UPGRADE);
            CASE(CERTSRV_E_ADMIN_DENIED_REQUEST);
            CASE(CERTSRV_E_NO_POLICY_SERVER);
            CASE(CERTSRV_E_WEAK_SIGNATURE_OR_KEY);
            CASE(CERTSRV_E_UNSUPPORTED_CERT_TYPE);
            CASE(CERTSRV_E_NO_CERT_TYPE);
            CASE(CERTSRV_E_TEMPLATE_CONFLICT);
            CASE(CERTSRV_E_SUBJECT_ALT_NAME_REQUIRED);
            CASE(CERTSRV_E_ARCHIVED_KEY_REQUIRED);
            CASE(CERTSRV_E_SMIME_REQUIRED);
            CASE(CERTSRV_E_BAD_RENEWAL_SUBJECT);
            CASE(CERTSRV_E_BAD_TEMPLATE_VERSION);
            CASE(CERTSRV_E_TEMPLATE_POLICY_REQUIRED);
            CASE(CERTSRV_E_SIGNATURE_POLICY_REQUIRED);
            CASE(CERTSRV_E_SIGNATURE_COUNT);
            CASE(CERTSRV_E_SIGNATURE_REJECTED);
            CASE(CERTSRV_E_ISSUANCE_POLICY_REQUIRED);
            CASE(CERTSRV_E_SUBJECT_UPN_REQUIRED);
            CASE(CERTSRV_E_SUBJECT_DIRECTORY_GUID_REQUIRED);
            CASE(CERTSRV_E_SUBJECT_DNS_REQUIRED);
            CASE(CERTSRV_E_ARCHIVED_KEY_UNEXPECTED);
            CASE(CERTSRV_E_KEY_LENGTH);
            CASE(CERTSRV_E_SUBJECT_EMAIL_REQUIRED);
            CASE(CERTSRV_E_UNKNOWN_CERT_TYPE);
            CASE(CERTSRV_E_CERT_TYPE_OVERLAP);
            CASE(CERTSRV_E_TOO_MANY_SIGNATURES);
            CASE(CERTSRV_E_RENEWAL_BAD_PUBLIC_KEY);
            CASE(XENROLL_E_KEY_NOT_EXPORTABLE);
            CASE(XENROLL_E_CANNOT_ADD_ROOT_CERT);
            CASE(XENROLL_E_RESPONSE_KA_HASH_NOT_FOUND);
            CASE(XENROLL_E_RESPONSE_UNEXPECTED_KA_HASH);
            CASE(XENROLL_E_RESPONSE_KA_HASH_MISMATCH);
            CASE(XENROLL_E_KEYSPEC_SMIME_MISMATCH);
            CASE(TRUST_E_SYSTEM_ERROR);
            CASE(TRUST_E_NO_SIGNER_CERT);
            CASE(TRUST_E_COUNTER_SIGNER);
            CASE(TRUST_E_CERT_SIGNATURE);
            CASE(TRUST_E_TIME_STAMP);
            CASE(TRUST_E_BAD_DIGEST);
            CASE(TRUST_E_BASIC_CONSTRAINTS);
            CASE(TRUST_E_FINANCIAL_CRITERIA);
            CASE(MSSIPOTF_E_OUTOFMEMRANGE);
            CASE(MSSIPOTF_E_CANTGETOBJECT);
            CASE(MSSIPOTF_E_NOHEADTABLE);
            CASE(MSSIPOTF_E_BAD_MAGICNUMBER);
            CASE(MSSIPOTF_E_BAD_OFFSET_TABLE);
            CASE(MSSIPOTF_E_TABLE_TAGORDER);
            CASE(MSSIPOTF_E_TABLE_LONGWORD);
            CASE(MSSIPOTF_E_BAD_FIRST_TABLE_PLACEMENT);
            CASE(MSSIPOTF_E_TABLES_OVERLAP);
            CASE(MSSIPOTF_E_TABLE_PADBYTES);
            CASE(MSSIPOTF_E_FILETOOSMALL);
            CASE(MSSIPOTF_E_TABLE_CHECKSUM);
            CASE(MSSIPOTF_E_FILE_CHECKSUM);
            CASE(MSSIPOTF_E_FAILED_POLICY);
            CASE(MSSIPOTF_E_FAILED_HINTS_CHECK);
            CASE(MSSIPOTF_E_NOT_OPENTYPE);
            CASE(MSSIPOTF_E_FILE);
            CASE(MSSIPOTF_E_CRYPT);
            CASE(MSSIPOTF_E_BADVERSION);
            CASE(MSSIPOTF_E_DSIG_STRUCTURE);
            CASE(MSSIPOTF_E_PCONST_CHECK);
            CASE(MSSIPOTF_E_STRUCTURE);
            CASE(ERROR_CRED_REQUIRES_CONFIRMATION);
            CASE(TRUST_E_PROVIDER_UNKNOWN);
            CASE(TRUST_E_ACTION_UNKNOWN);
            CASE(TRUST_E_SUBJECT_FORM_UNKNOWN);
            CASE(TRUST_E_SUBJECT_NOT_TRUSTED);
            CASE(DIGSIG_E_ENCODE);
            CASE(DIGSIG_E_DECODE);
            CASE(DIGSIG_E_EXTENSIBILITY);
            CASE(DIGSIG_E_CRYPTO);
            CASE(PERSIST_E_SIZEDEFINITE);
            CASE(PERSIST_E_SIZEINDEFINITE);
            CASE(PERSIST_E_NOTSELFSIZING);
            CASE(TRUST_E_NOSIGNATURE);
            CASE(CERT_E_EXPIRED);
            CASE(CERT_E_VALIDITYPERIODNESTING);
            CASE(CERT_E_ROLE);
            CASE(CERT_E_PATHLENCONST);
            CASE(CERT_E_CRITICAL);
            CASE(CERT_E_PURPOSE);
            CASE(CERT_E_ISSUERCHAINING);
            CASE(CERT_E_MALFORMED);
            CASE(CERT_E_UNTRUSTEDROOT);
            CASE(CERT_E_CHAINING);
            CASE(TRUST_E_FAIL);
            CASE(CERT_E_REVOKED);
            CASE(CERT_E_UNTRUSTEDTESTROOT);
            CASE(CERT_E_REVOCATION_FAILURE);
            CASE(CERT_E_CN_NO_MATCH);
            CASE(CERT_E_WRONG_USAGE);
            CASE(TRUST_E_EXPLICIT_DISTRUST);
            CASE(CERT_E_UNTRUSTEDCA);
            CASE(CERT_E_INVALID_POLICY);
            CASE(CERT_E_INVALID_NAME);

#undef CASE

        case 0:
            break;

        default:
            WriteFormat<0x100>("%u / 0x%08X", static_cast<uint32>(error), static_cast<uint32>(error));
            break;
        }
    }
}