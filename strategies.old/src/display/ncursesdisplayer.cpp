/**
 * @brief SiiS strategy ncurses displayer implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#include "siis/display/ncursesdisplayer.h"

using namespace siis;

NcursesDisplayer::NcursesDisplayer() :
    m_main(nullptr),
    m_win1(nullptr),
    m_win2(nullptr),
    m_noticeWin(nullptr),
    m_timeWin(nullptr),
    m_max{0, 0}
{

}

NcursesDisplayer::~NcursesDisplayer()
{

}

void NcursesDisplayer::init(Config *config)
{
    if (!m_win1) {
        initscr();
        raw();
        cbreak();

        start_color();
        // init_pair(1, COLOR_CYAN, COLOR_BLACK);

        nodelay(stdscr, true);
        noecho();

        int row, col;
        getmaxyx(stdscr, row, col);

        int half = row / 2;

        // m_main = newwin(row, col, 0, 0);

        // m_win1 = subwin(m_main, half-1, col, 0, 0);
        m_win1 = newwin(half, col, 0, 0);
        // box(m_win1, 0, 0);
        // m_win2 = subwin(m_main, half-1, col, half, 0);
        m_win2 = newwin(half-1, col, half, 0);
        // box(m_win2, 0, 0);
        // m_noticeWin = subwin(m_main, 1, col/2-1, row-1, 0);
        m_noticeWin = newwin(1, col/2-1, row-1, 0);
        // box(m_noticeWin, 0, 0);
        // m_timeWin = subwin(m_main, 1, col/2, row-1, col/2);
        m_timeWin = newwin(1, col/2, row-1, col/2);
        // box(m_timeWin, 0, 0);

        scrollok(m_win1, 0);
        scrollok(m_win2, 0);

        m_max[0] = half;
        m_max[1] = half-1;
    }
}

void NcursesDisplayer::terminate()
{
    if (m_win1) {
        delwin(m_win1);
        delwin(m_win2);
        delwin(m_noticeWin);
        delwin(m_timeWin);

        if (m_main) {
            delwin(m_main);
        }

        endwin();
    }
}

void NcursesDisplayer::sync()
{
    if (m_events[0]) {
        m_mutex[0].lock();
        m_events[0] = false;

        wclear(m_win1);

        // redraw all
        o3d::Int32 n = 0;
        for (o3d::String &_m : m_messages[0]) {
            mvwaddstr(m_win1, n, 0, _m.toUtf8().getData());
            ++n;
        }
        wrefresh(m_win1);

        m_mutex[0].unlock();
    }

    if (m_events[1]) {
        m_mutex[1].lock();
        m_events[1] = false;

        wclear(m_win2);

        // redraw all
        o3d::Int32 n = 0;
        for (o3d::String &_m : m_messages[1]) {
            mvwaddstr(m_win2, n, 0, _m.toUtf8().getData());
            ++n;
        }
        wrefresh(m_win2);

        m_mutex[1].unlock();
    }

    if (m_events[2]) {
        m_mutex[2].lock();
        m_events[0] = false;

        werase(m_noticeWin);
        mvwaddstr(m_noticeWin, 0, 0, m_notice.toUtf8().getData());
        wrefresh(m_noticeWin);

        m_mutex[2].unlock();
    }

    if (m_events[3]) {
        m_mutex[3].lock();
        m_events[3] = false;

        int row, col;
        getmaxyx(stdscr, row, col);

        o3d::CString d = m_time.toUtf8();
        int x = (col/2) - static_cast<int>(d.length());
        mvwaddstr(m_timeWin, 0, x, d.getData());
        wrefresh(m_timeWin);

        m_mutex[3].unlock();
    }
}

void NcursesDisplayer::display(const o3d::String &panel, const o3d::String &msg, o3d::System::MessageLevel type)
{
    if (m_win1) {
        if (panel == "win1" || panel == "default") {
            m_mutex[0].lock();
            m_messages[0].push_back(msg);
            if (m_messages[0].size() >= static_cast<size_t>(m_max[0])) {
                m_messages[0].pop_front();
            }
            m_events[0] = true;
            m_mutex[0].unlock();
        } else if (panel == "win2" || panel == "content") {
            m_mutex[1].lock();
            m_messages[1].push_back(msg);
            if (m_messages[1].size() >= static_cast<size_t>(m_max[1])) {
                m_messages[1].pop_front();
            }
            m_events[1] = true;
            m_mutex[1].unlock();
        } else if (panel == "notice") {
            m_mutex[2].lock();
            m_notice = msg;
            m_events[2] = true;
            m_mutex[2].unlock();
        } else if (panel == "time") {
            m_mutex[3].lock();
            m_time = msg;
            m_events[3] = true;
            m_mutex[3].unlock();
        }
    } else {
        o3d::System::print(msg, panel, type);
    }
}

void NcursesDisplayer::clear(const o3d::String &panel)
{
    if (m_win1) {
        if (panel == "win1" || panel == "default") {
            m_mutex[0].lock();
            m_messages[0].clear();
            m_events[0] = true;
            m_mutex[1].unlock();
        } else if (panel == "win2" || panel == "content") {
            m_mutex[1].lock();
            m_messages[1].clear();
            m_events[1] = true;
            m_mutex[1].unlock();
        } else if (panel == "notice") {
            m_mutex[2].lock();
            m_notice = "";
            m_events[2] = true;
            m_mutex[2].unlock();
        }
    }
}

void NcursesDisplayer::echo(o3d::WChar c)
{
    if (m_noticeWin) {
        m_mutex[2].lock();
        wechochar(m_noticeWin, static_cast<chtype>(c));
        wmove(m_noticeWin, 0, 0);
        m_mutex[2].unlock();
    }
}
