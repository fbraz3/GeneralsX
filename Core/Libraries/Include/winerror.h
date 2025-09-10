#pragma once
#ifndef _WINERROR_H_
#define _WINERROR_H_

// Windows Error Codes compatibility for macOS port
#ifdef __APPLE__

// Standard Windows HRESULT success/error codes
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

// Common Windows error codes
#define S_OK                          0x00000000L
#define S_FALSE                       0x00000001L
#define E_NOTIMPL                     0x80004001L
#define E_NOINTERFACE                 0x80004002L
#define E_POINTER                     0x80004003L
#define E_ABORT                       0x80004004L
#define E_FAIL                        0x80004005L
#define E_UNEXPECTED                  0x8000FFFFL
#define E_ACCESSDENIED                0x80070005L
#define E_HANDLE                      0x80070006L
#define E_OUTOFMEMORY                 0x8007000EL
#define E_INVALIDARG                  0x80070057L

// Additional common error codes
#define ERROR_SUCCESS                 0L
#define ERROR_INVALID_FUNCTION        1L
#define ERROR_FILE_NOT_FOUND          2L
#define ERROR_PATH_NOT_FOUND          3L
#define ERROR_TOO_MANY_OPEN_FILES     4L
#define ERROR_ACCESS_DENIED           5L
#define ERROR_INVALID_HANDLE          6L
#define ERROR_ARENA_TRASHED           7L
#define ERROR_NOT_ENOUGH_MEMORY       8L
#define ERROR_INVALID_BLOCK           9L
#define ERROR_BAD_ENVIRONMENT         10L
#define ERROR_BAD_FORMAT              11L
#define ERROR_INVALID_ACCESS          12L
#define ERROR_INVALID_DATA            13L
#define ERROR_OUTOFMEMORY             14L
#define ERROR_INVALID_DRIVE           15L
#define ERROR_CURRENT_DIRECTORY       16L
#define ERROR_NOT_SAME_DEVICE         17L
#define ERROR_NO_MORE_FILES           18L
#define ERROR_WRITE_PROTECT           19L
#define ERROR_BAD_UNIT                20L

// Network error codes
#define ERROR_NOT_READY               21L
#define ERROR_BAD_COMMAND             22L
#define ERROR_CRC                     23L
#define ERROR_BAD_LENGTH              24L
#define ERROR_SEEK                    25L
#define ERROR_NOT_DOS_DISK            26L
#define ERROR_SECTOR_NOT_FOUND        27L
#define ERROR_OUT_OF_PAPER            28L
#define ERROR_WRITE_FAULT             29L
#define ERROR_READ_FAULT              30L

// Registry error codes
#define ERROR_BADDB                   1009L
#define ERROR_BADKEY                  1010L
#define ERROR_CANTOPEN                1011L
#define ERROR_CANTREAD                1012L
#define ERROR_CANTWRITE               1013L
#define ERROR_REGISTRY_RECOVERED      1014L
#define ERROR_REGISTRY_CORRUPT        1015L
#define ERROR_REGISTRY_IO_FAILED      1016L
#define ERROR_NOT_REGISTRY_FILE       1017L
#define ERROR_KEY_DELETED             1018L

// File I/O error codes
#define ERROR_ALREADY_EXISTS          183L
#define ERROR_FILENAME_EXCED_RANGE    206L
#define ERROR_NESTING_NOT_ALLOWED     215L
#define ERROR_FILE_EXISTS             80L
#define ERROR_CANNOT_MAKE             82L
#define ERROR_FAIL_I24                83L
#define ERROR_OUT_OF_STRUCTURES       84L
#define ERROR_ALREADY_ASSIGNED        85L
#define ERROR_INVALID_PASSWORD        86L
#define ERROR_INVALID_PARAMETER       87L
#define ERROR_NET_WRITE_FAULT         88L
#define ERROR_NO_PROC_SLOTS           89L

// System error codes
#define ERROR_TOO_MANY_SEMAPHORES     100L
#define ERROR_EXCL_SEM_ALREADY_OWNED  101L
#define ERROR_SEM_IS_SET              102L
#define ERROR_TOO_MANY_SEM_REQUESTS   103L
#define ERROR_INVALID_AT_INTERRUPT_TIME 104L
#define ERROR_SEM_OWNER_DIED          105L
#define ERROR_SEM_USER_LIMIT          106L
#define ERROR_DISK_CHANGE             107L
#define ERROR_DRIVE_LOCKED            108L
#define ERROR_BROKEN_PIPE             109L

// Timeout and handle error codes
#define WAIT_FAILED                   0xFFFFFFFFL
#define WAIT_OBJECT_0                 0x00000000L
#define WAIT_TIMEOUT                  0x00000102L
#define WAIT_ABANDONED                0x00000080L

// COM/OLE error codes
#define CO_E_INIT_TLS                 0x80004006L
#define CO_E_INIT_SHARED_ALLOCATOR    0x80004007L
#define CO_E_INIT_MEMORY_ALLOCATOR    0x80004008L
#define CO_E_INIT_CLASS_CACHE         0x80004009L
#define CO_E_INIT_RPC_CHANNEL         0x8000400AL
#define CO_E_INIT_TLS_SET_CHANNEL_CONTROL 0x8000400BL
#define CO_E_INIT_TLS_CHANNEL_CONTROL 0x8000400CL
#define CO_E_INIT_UNACCEPTED_USER_ALLOCATOR 0x8000400DL
#define CO_E_INIT_SCM_MUTEX_EXISTS    0x8000400EL
#define CO_E_INIT_SCM_FILE_MAPPING_EXISTS 0x8000400FL
#define CO_E_INIT_SCM_MAP_VIEW_OF_FILE 0x80004010L
#define CO_E_INIT_SCM_EXEC_FAILURE    0x80004011L
#define CO_E_INIT_ONLY_SINGLE_THREADED 0x80004012L

// DirectX/Graphics error codes
#define DXERR_GENERIC_ERROR           0x80004005L
#define DXERR_INVALIDCALL             0x8876086CL
#define DXERR_INVALIDDATA             0x8876023CL
#define DXERR_UNSUPPORTED             0x80004001L

// Security error codes
#define ERROR_PRIVILEGE_NOT_HELD      1314L
#define ERROR_INVALID_OWNER           1307L
#define ERROR_INVALID_PRIMARY_GROUP   1308L
#define ERROR_INVALID_IMPERSONATION_LEVEL 1308L
#define ERROR_INVALID_PRIMARY_GROUP   1308L
#define ERROR_INVALID_IMPERSONATION_LEVEL 1310L

// Additional DirectShow/Media error codes
#define VFW_E_INVALIDMEDIATYPE        0x80040200L
#define VFW_E_INVALIDSUBTYPE          0x80040201L
#define VFW_E_NEED_OWNER              0x80040202L
#define VFW_E_ENUM_OUT_OF_SYNC        0x80040203L
#define VFW_E_ALREADY_CONNECTED       0x80040204L
#define VFW_E_FILTER_ACTIVE           0x80040205L
#define VFW_E_NO_TYPES                0x80040206L
#define VFW_E_NO_ACCEPTABLE_TYPES     0x80040207L

// SNMP error codes
#define SNMP_ERRORSTATUS_NOERROR      0
#define SNMP_ERRORSTATUS_TOOBIG       1
#define SNMP_ERRORSTATUS_NOSUCHNAME   2
#define SNMP_ERRORSTATUS_BADVALUE     3
#define SNMP_ERRORSTATUS_READONLY     4
#define SNMP_ERRORSTATUS_GENERR       5

#endif // __APPLE__
#endif // _WINERROR_H_
