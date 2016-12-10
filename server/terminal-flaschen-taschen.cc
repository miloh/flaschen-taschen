// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "led-flaschen-taschen.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define SCREEN_CLEAR    "\033c"
#define SCREEN_PREFIX   "\033[48;2;0;0;0m"  // set black background
#define SCREEN_POSTFIX  "\033[0m"           // reset terminal settings
#define SCREEN_CURSOR_UP_FORMAT "\033[%dA"  // Move cursor up given lines.
#define CURSOR_OFF      "\033[?25l"
#define CURSOR_ON       "\033[?25h"

// We have all colors same width for fast in-place updates in a precalculated
// buffer. So each pixel needs to be fixed width: we do %03d (which luckily is
// not interpreted as octal by the terminal)
#define PIXEL_PREFIX   "\033[48;2;"      // Setting 24-bit background color
#define COLOR_FORMAT   "%03d;%03d;%03dm" // RGB color formatting
#define PIXEL_CONTENT  "  "  // Two spaces make a somewhat 1:1 aspect ratio pixel

#define FPS_PLACEHOLDER "___________"
#define FPS_BACKSPACE   "\b\b\b\b\b\b\b\b\b\b\b"

static void reliable_write(int fd, const char *buf, size_t size) {
    int written;
    while (size && (written = write(fd, buf, size)) > 0) {
        size -= written;
        buf += written;
    }
}

TerminalFlaschenTaschen::TerminalFlaschenTaschen(int fd, int width, int height)
    : terminal_fd_(fd), width_(width), height_(height), is_first_(true),
      last_time_usec_(-1) {
}

void TerminalFlaschenTaschen::PostDaemonInit() {
    buffer_.append(SCREEN_PREFIX);
    initial_offset_ = buffer_.size();
    char scratch[64];
    snprintf(scratch, sizeof(scratch),
             PIXEL_PREFIX COLOR_FORMAT PIXEL_CONTENT, 0, 0, 0);  // black pixel.
    pixel_offset_ = strlen(scratch);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            buffer_.append(scratch);
        }
        buffer_.append("\n");
    }

    buffer_.append(SCREEN_POSTFIX);

    fps_offset_ = buffer_.size();
    buffer_.append(FPS_PLACEHOLDER FPS_BACKSPACE);

    snprintf(scratch, sizeof(scratch), SCREEN_CURSOR_UP_FORMAT, height_);
    buffer_.append(scratch);
}

TerminalFlaschenTaschen::~TerminalFlaschenTaschen() {
    if (!is_first_) {
        reliable_write(terminal_fd_, SCREEN_CLEAR, strlen(SCREEN_CLEAR));
        reliable_write(terminal_fd_, CURSOR_ON, strlen(CURSOR_ON));
    }
}

void TerminalFlaschenTaschen::SetPixel(int x, int y, const Color &col) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
    const int pos = initial_offset_
        + (width_ * y + x) * pixel_offset_
        + strlen(PIXEL_PREFIX)   // Exactly where the color-formatting starts
        + y;                     // <- one newline per y
    char *buf = const_cast<char*>(buffer_.data()) + pos;  // Living on the edge
    WriteByteDecimal(buf, col.r);      // rrr;___;___
    WriteByteDecimal(buf + 4, col.g);  // ___;ggg;___
    WriteByteDecimal(buf + 8, col.b);  // ___;___;bbb
}

void TerminalFlaschenTaschen::Send() {
    if (is_first_) {
        assert(!buffer_.empty());  // Looks like PostDaemonInit() was not called
        reliable_write(terminal_fd_, SCREEN_CLEAR, strlen(SCREEN_CLEAR));
        reliable_write(terminal_fd_, CURSOR_OFF, strlen(CURSOR_OFF));
        is_first_ = false;
    }

    char *fps_place = const_cast<char*>(buffer_.data()) + fps_offset_;
    struct timeval tp;
    gettimeofday(&tp, NULL);
    const int64_t time_now_usec = tp.tv_sec * 1000000 + tp.tv_usec;
    const int64_t duration = time_now_usec - last_time_usec_;
    if (last_time_usec_ > 0 && duration > 500 && duration < 10000000) {
        const float fps = 1e6 / duration;
        snprintf(fps_place, strlen(FPS_PLACEHOLDER)+1, "%7.1f fps", fps);
        fps_place[strlen(FPS_PLACEHOLDER)] = '\b';
    } else {
        memcpy(fps_place, FPS_PLACEHOLDER, strlen(FPS_PLACEHOLDER));
    }
    last_time_usec_ = time_now_usec;

    reliable_write(terminal_fd_, buffer_.data(), buffer_.size());
}
