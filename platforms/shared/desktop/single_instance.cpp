/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#include "gearboy.h"

#define SINGLE_INSTANCE_IMPORT
#include "single_instance.h"

static const char* k_lock_filename = ".lock";
static const char* k_mailbox_filename = ".mailbox";
static const char* k_mailbox_tmp_filename = ".mailbox.tmp";

#if defined(_WIN32)
static const char* k_event_name_prefix = "Local\\Gearboy_";
#else
static const char* k_pid_filename = ".pid";
#endif

static std::string s_lock_path;
static std::string s_mailbox_path;
static std::string s_mailbox_tmp_path;
static bool s_is_primary = false;
static bool s_initialized = false;
static bool s_disabled = false;
static bool s_pending_load = false;
static char s_pending_rom_path[4096];
static char s_pending_symbol_path[4096];

#if defined(_WIN32)
static HANDLE s_lock_file = INVALID_HANDLE_VALUE;
static HANDLE s_event = NULL;
static std::string s_event_name;
#else
static int s_lock_fd = -1;
static std::string s_pid_path;
static volatile sig_atomic_t s_signal_received = 0;
#endif

#if !defined(_WIN32)
static void signal_handler(int sig)
{
    if (sig == SIGUSR1)
        s_signal_received = 1;
}

static bool is_temp_dir_accessible(const std::string& base_path)
{
    std::string test_file = base_path + ".test";
    FILE* f = fopen(test_file.c_str(), "w");
    if (!f)
        return false;
    fclose(f);
    unlink(test_file.c_str());
    return true;
}
#endif

static bool is_lock_stale(void)
{
#if defined(_WIN32)
    // On Windows, the lock is automatically released when the process exits
    return false;
#else
    FILE* pid_file = fopen(s_pid_path.c_str(), "r");
    if (!pid_file)
        return true;

    pid_t stored_pid = 0;
    if (fscanf(pid_file, "%d", &stored_pid) != 1)
    {
        fclose(pid_file);
        return true;
    }
    fclose(pid_file);

    if (stored_pid <= 0)
        return true;

    // Check if process exists
    if (kill(stored_pid, 0) == -1)
    {
        if (errno == ESRCH)
            return true;
    }

    return false;
#endif
}

static void cleanup_stale_lock(void)
{
#if !defined(_WIN32)
    if (is_lock_stale())
    {
        Debug("Removing stale lock files");
        unlink(s_lock_path.c_str());
        unlink(s_pid_path.c_str());
        unlink(s_mailbox_path.c_str());
    }
#endif
}

static void write_pid_file(void)
{
#if !defined(_WIN32)
    FILE* pid_file = fopen(s_pid_path.c_str(), "w");
    if (pid_file)
    {
        fprintf(pid_file, "%d", getpid());
        fclose(pid_file);
    }
#endif
}

static bool read_mailbox(void)
{
    FILE* mailbox = fopen(s_mailbox_path.c_str(), "r");
    if (!mailbox)
        return false;

    s_pending_rom_path[0] = '\0';
    s_pending_symbol_path[0] = '\0';

    char line[4096];
    int line_num = 0;

    while (fgets(line, sizeof(line), mailbox) && line_num < 2)
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (line_num == 0)
            strncpy(s_pending_rom_path, line, sizeof(s_pending_rom_path) - 1);
        else if (line_num == 1)
            strncpy(s_pending_symbol_path, line, sizeof(s_pending_symbol_path) - 1);

        line_num++;
    }

    fclose(mailbox);
    unlink(s_mailbox_path.c_str());

    if (s_pending_rom_path[0] != '\0')
    {
        s_pending_load = true;
        Log("Received message from secondary instance: ROM=%s, Symbols=%s",
            s_pending_rom_path,
            s_pending_symbol_path[0] ? s_pending_symbol_path : "(none)");
        return true;
    }

    return false;
}

void single_instance_init(const char* app_name)
{
    if (s_initialized)
        return;

    s_initialized = true;
    s_is_primary = false;
    s_disabled = false;
    s_pending_load = false;
    s_pending_rom_path[0] = '\0';
    s_pending_symbol_path[0] = '\0';

#if defined(_WIN32)
    char temp_path[MAX_PATH];
    if (GetTempPathA(MAX_PATH, temp_path) == 0)
    {
        Log("Single instance mode disabled: unable to access temp directory");
        s_disabled = true;
        s_is_primary = true;
        return;
    }
    std::string base_path = std::string(temp_path) + app_name + "_";
#else
    std::string base_path = std::string("/tmp/") + app_name + "_";
    if (!is_temp_dir_accessible(base_path))
    {
        Log("Single instance mode disabled: unable to access temp directory");
        s_disabled = true;
        s_is_primary = true;
        return;
    }
#endif

    s_lock_path = base_path + k_lock_filename;
    s_mailbox_path = base_path + k_mailbox_filename;
    s_mailbox_tmp_path = base_path + k_mailbox_tmp_filename;

#if defined(_WIN32)
    s_event_name = std::string(k_event_name_prefix) + app_name;
#else
    s_pid_path = base_path + k_pid_filename;
#endif

    Debug("Single instance lock path: %s", s_lock_path.c_str());
}

void single_instance_destroy(void)
{
    if (!s_initialized)
        return;

#if defined(_WIN32)
    if (s_event != NULL)
    {
        CloseHandle(s_event);
        s_event = NULL;
    }

    if (s_lock_file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(s_lock_file);
        s_lock_file = INVALID_HANDLE_VALUE;
        DeleteFileA(s_lock_path.c_str());
    }
#else
    if (s_lock_fd >= 0)
    {
        close(s_lock_fd);
        s_lock_fd = -1;
        unlink(s_lock_path.c_str());
        unlink(s_pid_path.c_str());
    }
#endif

    s_initialized = false;
    s_is_primary = false;
}

bool single_instance_try_lock(void)
{
    if (!s_initialized || s_disabled)
        return true;

    cleanup_stale_lock();

#if defined(_WIN32)
    s_lock_file = CreateFileA(
        s_lock_path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,  // No sharing
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
        NULL
    );

    if (s_lock_file == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        if (error == ERROR_SHARING_VIOLATION)
        {
            Debug("Another instance is already running");
            s_is_primary = false;
            return false;
        }
        Error("Failed to create lock file: %lu", error);
        s_is_primary = true;
        return true;
    }

    // Lock the file exclusively
    OVERLAPPED overlapped = {0};
    if (!LockFileEx(s_lock_file, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &overlapped))
    {
        DWORD error = GetLastError();
        if (error == ERROR_LOCK_VIOLATION)
        {
            Debug("Another instance is already running (lock failed)");
            CloseHandle(s_lock_file);
            s_lock_file = INVALID_HANDLE_VALUE;
            s_is_primary = false;
            return false;
        }
    }

    // Create a named event for notification
    s_event = CreateEventA(NULL, FALSE, FALSE, s_event_name.c_str());
    if (s_event == NULL)
    {
        Error("Failed to create event: %lu", GetLastError());
    }

    s_is_primary = true;
    Log("Single instance lock acquired");
    return true;

#else
    s_lock_fd = open(s_lock_path.c_str(), O_CREAT | O_RDWR, 0600);
    if (s_lock_fd < 0)
    {
        Error("Failed to open lock file: %s", strerror(errno));
        s_is_primary = true;
        return true;
    }

    if (flock(s_lock_fd, LOCK_EX | LOCK_NB) == -1)
    {
        if (errno == EWOULDBLOCK)
        {
            Debug("Another instance is already running");
            close(s_lock_fd);
            s_lock_fd = -1;
            s_is_primary = false;
            return false;
        }
        Error("flock failed: %s", strerror(errno));
    }

    write_pid_file();

    // Set up signal handler
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    s_is_primary = true;
    Log("Single instance lock acquired");
    return true;
#endif
}

bool single_instance_is_primary(void)
{
    return s_is_primary;
}

void single_instance_send_message(const char* rom_path, const char* symbol_path)
{
    if (!s_initialized || s_disabled)
        return;

    // Write to temp file first, then rename for atomicity
    FILE* mailbox = fopen(s_mailbox_tmp_path.c_str(), "w");
    if (!mailbox)
    {
        Error("Failed to write mailbox: %s", strerror(errno));
        return;
    }

    fprintf(mailbox, "%s\n", rom_path ? rom_path : "");
    fprintf(mailbox, "%s\n", symbol_path ? symbol_path : "");
    fclose(mailbox);

    // Atomic rename
#if defined(_WIN32)
    MoveFileExA(s_mailbox_tmp_path.c_str(), s_mailbox_path.c_str(), MOVEFILE_REPLACE_EXISTING);
#else
    rename(s_mailbox_tmp_path.c_str(), s_mailbox_path.c_str());
#endif

    Log("Message sent to primary instance: ROM=%s, Symbols=%s",
        rom_path ? rom_path : "(none)",
        symbol_path ? symbol_path : "(none)");

#if defined(_WIN32)
    HANDLE event = OpenEventA(EVENT_MODIFY_STATE, FALSE, s_event_name.c_str());
    if (event != NULL)
    {
        SetEvent(event);
        CloseHandle(event);
        Debug("Signaled primary instance via event");
    }
    else
    {
        Debug("Could not open event, primary will poll");
    }
#else
    // Read PID and send signal
    FILE* pid_file = fopen(s_pid_path.c_str(), "r");
    if (pid_file)
    {
        pid_t pid = 0;
        if (fscanf(pid_file, "%d", &pid) == 1 && pid > 0)
        {
            if (kill(pid, SIGUSR1) == 0)
            {
                Debug("Signaled primary instance (PID %d) via SIGUSR1", pid);
            }
            else
            {
                Debug("Could not signal PID %d: %s", pid, strerror(errno));
            }
        }
        fclose(pid_file);
    }
#endif
}

void single_instance_poll(void)
{
    if (!s_initialized || s_disabled || !s_is_primary)
        return;

    bool should_check = false;

#if defined(_WIN32)
    // Check event (non-blocking)
    if (s_event != NULL)
    {
        if (WaitForSingleObject(s_event, 0) == WAIT_OBJECT_0)
            should_check = true;
    }
#else
    // Check if we received a signal
    if (s_signal_received)
    {
        s_signal_received = 0;
        should_check = true;
    }
#endif

    if (should_check)
        read_mailbox();
}

bool single_instance_get_pending_load(char* rom_path, int rom_path_size, char* symbol_path, int symbol_path_size)
{
    if (!s_pending_load)
        return false;

    s_pending_load = false;

    if (rom_path && rom_path_size > 0)
    {
        strncpy(rom_path, s_pending_rom_path, rom_path_size - 1);
        rom_path[rom_path_size - 1] = '\0';
    }

    if (symbol_path && symbol_path_size > 0)
    {
        strncpy(symbol_path, s_pending_symbol_path, symbol_path_size - 1);
        symbol_path[symbol_path_size - 1] = '\0';
    }

    s_pending_rom_path[0] = '\0';
    s_pending_symbol_path[0] = '\0';

    return true;
}
