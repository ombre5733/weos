/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

//! newlib defines errno as a macro in order to support reentrancy. For
//! retargetting, the macro has to be undefined and errors have to be
//! written into the errno variable.
#undef errno
extern int errno;

int _write(int file, char* ptr, int len);
extern void putChar(char ch);

//! environ must be a pointer to an array of environment variables. For a
//! minimal implementation, an empty list is sufficient.
static char* environmentVariables[1] = { 0 };
char** environ = environmentVariables;

/*
//! Exits a program without cleaning up files.
void _exit(int status)
{
    _write(STDERR_FILENO, "exit\n", 5);
    while (1)
    {
    }
}
*/

//! Closes the file identified by \p file.
int _close(int file)
{
    return -1;
}

//! Transfers control to a new process.
int _execve(char* name, char** argv, char** env)
{
    errno = ENOMEM;
    return -1;
}

//! Creates a new process.
int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

//! Returns the status of an open file.Status of an open file.
int _fstat(int file, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

//! Returns the process ID.
int _getpid(void)
{
    return 1;
}

//! Checks if an output stream is a terminal.
int _isatty(int file)
{
    switch (file)
    {
        case STDOUT_FILENO:
        case STDERR_FILENO:
        case STDIN_FILENO:
            return 1;
        default:
            errno = EBADF;
            return 0;
    }
}

//! Kills a process.
int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

//! Establishes a new name for an existing file.
int _link(char* oldName, char* newName)
{
    errno = EMLINK;
    return -1;
}

//! Sets the position in a file.
int _lseek(int file, int ptr, int dir)
{
    return 0;
}

//! Opens a file.
int _open(const char* name, int flags, int mode)
{
    return -1;
}

//! Reads from a file.
int _read(int file, char* ptr, int len)
{
    return 0;
}

//! Increases the program data space.
caddr_t _sbrk(int incr)
{
    // The following symbols are defined in the linker script.
    extern char __heap_start__;
    extern char __heap_end__;

    static char* heapPtr = &__heap_start__;
    char* temp;

    if (heapPtr + incr > &__heap_end__)
    {
        _write(STDERR_FILENO, "Out of memory\n", 14);
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    temp = heapPtr;
    heapPtr += incr;
    return (caddr_t)temp;
}

//! Returns the status of a file.
int _stat(char* file, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

//! Returns timing information for the current process.
int _times(struct tms* buf)
{
    return -1;
}

//! Removes a file's directory entry.
int _unlink(char* name)
{
    errno = ENOENT;
    return -1;
}

//! Waits for a child process.
int _wait(int* status)
{
    errno = ECHILD;
    return -1;
}

//! Writes to a file.
int _write(int file, char* ptr, int len)
{
    int n;
    switch (file)
    {
        case STDOUT_FILENO:
        case STDERR_FILENO:
            for (n = 0; n < len; ++n)
            {
                putChar(*ptr++);
            }
            break;
        default:
            errno = EBADF;
            return -1;
    }
    return len;
}
