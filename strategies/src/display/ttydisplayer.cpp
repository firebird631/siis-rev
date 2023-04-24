/**
 * @brief SiiS strategy tty displayer implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#include "siis/display/ttydisplayer.h"

using namespace siis;

TtyDisplayer::TtyDisplayer()
{

}

TtyDisplayer::~TtyDisplayer()
{

}

void TtyDisplayer::init(Config *config)
{

}

void TtyDisplayer::terminate()
{

}

void TtyDisplayer::sync()
{

}

void TtyDisplayer::display(const o3d::String &panel, const o3d::String &msg, o3d::System::MessageLevel type)
{
    if (panel == "win1" || panel == "default") {
        o3d::System::print(msg, panel, type);
    } else if (panel == "win2" || panel == "content") {
        o3d::System::print(msg, panel, type);
    } else if (panel == "notice") {
        o3d::System::print(msg, panel, type);
    } else if (panel == "time") {
        // don't display time
    } else {
        o3d::System::print(msg, panel, type);
    }
}

void TtyDisplayer::clear(const o3d::String &panel)
{

}

void TtyDisplayer::echo(o3d::WChar c)
{

}
