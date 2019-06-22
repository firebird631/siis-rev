/**
 * @brief SiiS strategy ncurses displayer implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#ifndef SIIS_NCURSESDISPLAYER_H
#define SIIS_NCURSESDISPLAYER_H

#include "displayer.h"
#include <o3d/core/mutex.h>

#include <ncurses.h>

namespace siis {

/**
 * @brief SiiS strategy ncurses displayer implementation.
 * @author Frederic Scherma
 * @date 2019-03-26
 */
class SIIS_API NcursesDisplayer : public Displayer
{
public:

    NcursesDisplayer();

    virtual ~NcursesDisplayer() override;

    virtual void init(Config *config) override;
    virtual void terminate() override;

//    virtual void start() override;
//    virtual void stop() override;
    virtual void sync() override;

    virtual void display(
            const o3d::String &panel,
            const o3d::String &msg,
            o3d::System::MessageLevel type = o3d::System::MSG_INFO) override;

    virtual void clear(const o3d::String &panel) override;

    virtual void echo(o3d::WChar c) override;

private:

    o3d::Mutex m_mutex[4];
    o3d::Bool m_events[4];
    std::deque<o3d::String> m_messages[2];
    o3d::String m_time;
    o3d::String m_notice;

    WINDOW *m_main;
    WINDOW *m_win1;
    WINDOW *m_win2;
    WINDOW *m_noticeWin;
    WINDOW *m_timeWin;

    o3d::Int32 m_max[2];

    void displayInt(const o3d::String &panel, const o3d::String &msg, o3d::System::MessageLevel type);
    void clearInt(const o3d::String &panel);
};

} // namespace siis

#endif // SIIS_NCURSESDISPLAYER_H
