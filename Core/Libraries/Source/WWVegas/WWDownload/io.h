#ifndef IO_H
#define IO_H

#ifdef _WIN32
    #include <io.h>
#else
    // Unix/macOS I/O compatibility
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    
    // File access modes
    #define _O_RDONLY O_RDONLY
    #define _O_WRONLY O_WRONLY
    #define _O_RDWR O_RDWR
    #define _O_APPEND O_APPEND
    #define _O_CREAT O_CREAT
    #define _O_TRUNC O_TRUNC
    #define _O_EXCL O_EXCL
    #define _O_BINARY 0
    #define _O_TEXT 0
    
    // File permission modes
    #define _S_IREAD S_IRUSR
    #define _S_IWRITE S_IWUSR
    #define _S_IEXEC S_IXUSR
    
    // File functions
    #define _open(path, oflag, ...) open(path, oflag, ##__VA_ARGS__)
    #define _close(fd) close(fd)
    #define _read(fd, buf, count) read(fd, buf, count)
    #define _write(fd, buf, count) write(fd, buf, count)
    #define _lseek(fd, offset, whence) lseek(fd, offset, whence)
    
    // File information
    #define _filelength(fd) ({ struct stat st; fstat(fd, &st) == 0 ? st.st_size : -1; })
    #define _access(path, mode) access(path, mode)
    
    // Access modes
    #define F_OK 0
    #define R_OK 4
    #define W_OK 2
    #define X_OK 1
    
#endif

#endif // IO_H
