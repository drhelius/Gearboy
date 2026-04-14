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

#ifndef _MEMORY_STREAM_H_
#define _MEMORY_STREAM_H_

#include <iostream>
#include <streambuf>
#include <cstring>

class memory_buffer : public std::streambuf
{
private:
    char* buffer_start;
    char* buffer_end;

public:
    memory_buffer(char* buffer, size_t size)
    {
        buffer_start = buffer;
        buffer_end = buffer + size;
        setp(buffer_start, buffer_end);
    }

    size_t size() const
    {
        return pptr() - buffer_start;
    }

protected:
    std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        if (pptr() + n > buffer_end)
            return 0;

        std::memcpy(pptr(), s, (size_t)n);
        pbump((int)n);
        return n;
    }

    int_type overflow(int_type) override
    {
        return EOF;
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::out) override
    {
        if (which & std::ios_base::out)
        {
            char* new_pos;

            if (dir == std::ios_base::beg)
                new_pos = buffer_start + off;
            else if (dir == std::ios_base::cur)
                new_pos = pptr() + off;
            else
                new_pos = buffer_end + off;

            if (new_pos >= buffer_start && new_pos <= buffer_end)
            {
                setp(buffer_start, buffer_end);
                pbump((int)(new_pos - buffer_start));
                return pos_type(new_pos - buffer_start);
            }
        }

        return pos_type(off_type(-1));
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::out) override
    {
        return seekoff(off_type(pos), std::ios_base::beg, which);
    }
};

class memory_stream : public std::ostream
{
private:
    memory_buffer buf;

public:
    memory_stream(char* buffer, size_t size)
        : std::ostream(NULL), buf(buffer, size)
    {
        rdbuf(&buf);
    }

    size_t size() const
    {
        return buf.size();
    }
};

class memory_input_buffer : public std::streambuf {
private:
    const char* buffer_start;
    const char* buffer_end;

public:
    memory_input_buffer(const char* buffer, size_t size)
    {
        buffer_start = buffer;
        buffer_end = buffer + size;
        setg(const_cast<char*>(buffer_start), const_cast<char*>(buffer_start), const_cast<char*>(buffer_end));
    }

protected:
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
    {
        if (which & std::ios_base::in)
        {
            char* new_pos;

            if (dir == std::ios_base::beg)
                new_pos = const_cast<char*>(buffer_start) + off;
            else if (dir == std::ios_base::cur)
                new_pos = gptr() + off;
            else // std::ios_base::end
                new_pos = const_cast<char*>(buffer_end) + off;

            if (new_pos >= const_cast<char*>(buffer_start) && new_pos <= const_cast<char*>(buffer_end))
            {
                setg(const_cast<char*>(buffer_start), new_pos, const_cast<char*>(buffer_end));
                return pos_type(new_pos - const_cast<char*>(buffer_start));
            }
        }
        return pos_type(off_type(-1));
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in) override
    {
        return seekoff(off_type(pos), std::ios_base::beg, which);
    }
};

class memory_input_stream : public std::istream
{
private:
    memory_input_buffer buf;

public:
    memory_input_stream(const char* buffer, size_t size)
        : std::istream(NULL), buf(buffer, size)
    {
        rdbuf(&buf);
    }
};

#endif // _MEMORY_STREAM_H_
