/**
 * @brief SiiS strategy displayer interface.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#ifndef SIIS_DISPLAYER_H
#define SIIS_DISPLAYER_H

#include <o3d/core/base.h>

#include "../base.h"
#include "../config/config.h"

namespace siis {

/**
 * @brief SiiS strategy handler processing interface.
 * @author Frederic Scherma
 * @date 2019-03-10
 */
class SIIS_API Displayer
{
public:

    virtual ~Displayer() = 0;

    virtual void init(Config *config) = 0;
    virtual void terminate() = 0;

//    /**
//     * @brief start Start execution thread(s), do the sync in sync method.
//     */
//    virtual void start() = 0;

//    /**
//     * @brief stop Stop execution of the processing thread(s).
//     */
//    virtual void stop() = 0;

    /**
     * @brief sync Process necessary synchronizations with the main thread.
     */
    virtual void sync() = 0;

    /**
     * @brief display Display a message in a specific panel
     * @param panel String panel name or empty for default.
     * @param msg Message content.
     */
    virtual void display(
            const o3d::String &panel,
            const o3d::String &msg,
            o3d::System::MessageLevel type = o3d::System::MSG_INFO) = 0;

    /**
     * @brief clear Clear the content of a panel.
     */
    virtual void clear(const o3d::String &panel) = 0;

    virtual void echo(o3d::WChar c) = 0;
};

} // namespace siis

#endif // SIIS_DISPLAYER_H
