/**
 * @brief SiiS strategy raw terminal displayer implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#ifndef SIIS_TTYDISPLAYER_H
#define SIIS_TTYDISPLAYER_H

#include "displayer.h"

namespace siis {

/**
 * @brief SiiS strategy raw terminal displayer implementation.
 * @author Frederic Scherma
 * @date 2019-03-26
 */
class SIIS_API TtyDisplayer : public Displayer
{
public:

    TtyDisplayer();

    virtual ~TtyDisplayer() override;

    virtual void init(Config *config) override;
    virtual void terminate() override;

//    virtual void start() override;
//    virtual void stop() override;
    virtual void sync() override;

    /**
     * @brief display Display a message in a specific panel
     * @param panel String panel name or empty for default.
     * @param msg Message content.
     */
    virtual void display(
            const o3d::String &panel,
            const o3d::String &msg,
            o3d::System::MessageLevel type = o3d::System::MSG_INFO) override;

    virtual void clear(const o3d::String &panel) override;

    virtual void echo(o3d::WChar c) override;
};

} // namespace siis

#endif // SIIS_TTYDISPLAYER_H
