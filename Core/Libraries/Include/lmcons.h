#pragma once
#ifndef _LMCONS_H_
#define _LMCONS_H_

// LM constants compatibility for macOS port
#ifdef __APPLE__

// Network constants
#define UNLEN 256      // Maximum user name length
#define GNLEN 256      // Maximum group name length
#define PWLEN 256      // Maximum password length
#define CNLEN 15       // Maximum computer name length
#define DNLEN 256      // Maximum domain name length
#define NNLEN 80       // Maximum netgroup name length
#define SNLEN 80       // Maximum service name length

// Net API constants
#define NET_API_STATUS DWORD
#define NERR_Success 0

#endif // __APPLE__
#endif // _LMCONS_H_
