/**
 * @brief SiiS strategy main.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include <o3d/core/architecture.h>
#include <o3d/core/application.h>
#include <o3d/core/main.h>
#include <o3d/core/commandline.h>
#include <o3d/core/dynamiclibrary.h>
#include <o3d/core/processor.h>
#include <o3d/core/filemanager.h>

#include "siis/logger.h"
#include "siis/poolworker.h"
#include "siis/config/config.h"
#include "siis/collection.h"
#include "siis/database/database.h"
#include "siis/cache/cache.h"
#include "siis/trade/tradesignal.h"

#include "siis/display/ncursesdisplayer.h"
#include "siis/display/ttydisplayer.h"

#include "live/live.h"
#include "backtest/backtest.h"
#include "learning/learning.h"
#include "optimization/optimization.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>

using namespace o3d;
using namespace siis;

static volatile sig_atomic_t catch_sigint = 0;
static volatile sig_atomic_t catch_sigquit = 0;
static volatile sig_atomic_t catch_sigterm = 0;

static void sig_handler(int signo)
{
    if (signo == SIGINT) {
        catch_sigint = 1;
    } else if (signo == SIGQUIT) {
        catch_sigquit = 1;
    } else if (signo == SIGTERM) {
        catch_sigterm = 1;
    }
}

// Main class
class SiisStrategy {

public:

    SiisStrategy() :
        m_poolWorker(nullptr),
        m_config(nullptr),
        m_siisPath(),
        m_strategyCollection(nullptr),
        m_supervisorCollection(nullptr),
        m_database(nullptr),
        m_cache(nullptr),
        m_handler(nullptr),
        m_displayer(nullptr),
        m_analysisLog(nullptr),
        m_orderLog(nullptr)
    {
        struct passwd *pw = getpwuid(getuid());
        m_siisPath = Dir(pw->pw_dir);
        m_siisPath.cd(".siis");

        Debug::instance()->setDefaultLog(m_siisPath.getFullPathName() + "/log/strategy.log");

        m_analysisLog = new FileLogger(m_siisPath.getFullPathName() + "/log/strategy-analysis.log");
        m_orderLog = new FileLogger(m_siisPath.getFullPathName() + "/log/strategy-order.log");

        analysisLog = m_analysisLog;
        orderLog = m_orderLog;
    }

    ~SiisStrategy()
    {
        terminate();
    }

    void displayHelp()
    {
        printf("Command line help:\n");
        printf("\n");
        printf("  -h --help This help message\n");
        printf("  -v --version SiiS strategy version number\n");
        printf("  -n --nointeractive To disable ncurse interactive mode\n");
        printf("  -d --verbose [0..4] Set verbosity level from any 0 to 4 only criticals\n");
        printf("  -c --cpu Force the number of workers to uses (from 1 to max CPUs)\n");
        printf("\n");
        printf("  -p --profile <filename.json> To define the profile configuration\n");
        printf("  -s --strategy <filename.json> To define the strategy configuration\n");
        printf("  -x --learning <filename.json> To define the learning override configuration\n");
        printf("  -S --supervisor <filename.json> To define the supervisor configuration\n");
        printf("  -m --market To define one ore more specific markets only (must exists in configuration file)\n");
        printf("\n");
        printf("  -l --live Live (mode) Live real or paper trading (see -p flag) using the connector\n");
        printf("  -P --paper Paper trader only in live mode (see -l flag)\n");
        printf("  -b --backtest (mode) Process a backtesting (need -f -t and -i options). Not compatible with others mode\n");
        printf("  -L --learn (mode) Machine learning training\n");
        printf("  -o --optimize (mode) Machine learning optimization\n");
        printf("\n");
        printf("  -f --from Define the start datetime for backtest/learn/optimize mode in format YYYY-mm-ddTHH:MM:SS (example 2019-01-01T00:00:00)\n");
        printf("  -t --to Define the stop datetime for backtest/learn/optimize mode in format YYYY-mm-ddTHH:MM:SS (example 2019-01-01T00:00:00)\n");
        printf("  -i --timestep Timestep increment in second or in string for the backtest/learn/optimize mode (example 1m for 1 minute, 4h, 1M for 1 month)\n");
        printf("\n");
        printf("In interactive mode type the 'q' key and confirm with 'y' to exit the program or cancel with 'n'.\n");
        printf("Else simply type CTRL-C signal.\n");
    }

    void displayVersion()
    {
        printf("SiiS strategy version 2.0.0a\n");
    }

    o3d::Int32 init()
    {
        signal(SIGINT, sig_handler);
        signal(SIGQUIT, sig_handler);
        signal(SIGTERM, sig_handler);

        // minimalist command line options (need specific config file)
        CommandLine *cmd = Application::getCommandLine();
        cmd->addSwitch('h', "help");
        cmd->addSwitch('v', "version");
        cmd->addOption('p', "profile");
        cmd->addOption('s', "strategy");
        cmd->addOption('x', "learning");
        cmd->addOption('S', "supervisor");
        cmd->addRepeatableOption('m', "market");
        cmd->addOptionalOption('f', "from", "");
        cmd->addOptionalOption('t', "to", "");
        cmd->addOptionalOption('i', "timestep", "0");
        cmd->addSwitch('l', "live");
        cmd->addSwitch('b', "backtest");
        cmd->addSwitch('P', "paper");
        cmd->addSwitch('n', "nointerative");
        cmd->addSwitch('L', "learn");
        cmd->addSwitch('o', "optimize");
        cmd->addOptionalOption('c', "cpu", "0");
        cmd->addOptionalOption('d', "verbose", "0");

        if (!cmd->parse()) {
            throw E_InvalidParameter("Invalid parameters");
        }

        if (cmd->getSwitch('h')) {
            displayHelp();
            return 1;
        }

        if (cmd->getSwitch('v')) {
            displayVersion();
            return 1;
        }

        String strategyConfigFileName = cmd->getOptionValue('s');
        String profileConfigFileName = cmd->getOptionValue('p');
        String learningConfigFileName = cmd->getOptionValue('x');
        String supervisorConfigFileName = cmd->getOptionValue('S');

        if (strategyConfigFileName.isValid() && profileConfigFileName.isValid()) {
            throw E_InvalidParameter("Either strategy or profile configuration file can be specified");
        }

        if ((cmd->getSwitch('l') || cmd->getSwitch('b')) && strategyConfigFileName.isEmpty() && profileConfigFileName.isEmpty()) {
            throw E_InvalidParameter("No strategy or profile configuration file specified");
        }

        if ((cmd->getSwitch('L') || cmd->getSwitch('o')) && supervisorConfigFileName.isEmpty()) {
            throw E_InvalidParameter("No supervisor configuration file specified");
        }

        if (strategyConfigFileName.isEmpty() && profileConfigFileName.isEmpty() && supervisorConfigFileName.isEmpty()) {
            displayHelp();
            return 1;
        }

        return 0;
    }

    void start(o3d::Int32 numCPU)
    {
        CommandLine *cmd = Application::getCommandLine();

        String strategyConfigFileName = cmd->getOptionValue('s');
        String profileConfigFileName = cmd->getOptionValue('p');
        String learningConfigFileName = cmd->getOptionValue('x');
        String supervisorConfigFileName = cmd->getOptionValue('S');
        o3d::Int32 cmdNumCPU = cmd->getOptionValue('c').toInt32();
        o3d::Int32 numWorkers = -1;

        // config
        m_config = new Config();
        m_config->initPaths(m_siisPath);

        m_config->loadCmdLine(cmd);
        m_config->loadCommon();

        if (strategyConfigFileName.isValid()) {
            m_config->loadStrategySpec(strategyConfigFileName);
        } else if (profileConfigFileName.isValid()) {
            m_config->loadProfileSpec(profileConfigFileName);
        }

        if (learningConfigFileName.isValid()) {
            m_config->loadLearningSpec(learningConfigFileName);
        }

        if (supervisorConfigFileName.isValid()) {
            m_config->loadSupervisorSpec(supervisorConfigFileName);
        }

        if (m_config->getNumWorkers() > 0) {
            numWorkers = m_config->getNumWorkers();
        }

        // cmd line parameter override configuration
        if (cmdNumCPU > 0 && cmdNumCPU < numCPU) {
            numWorkers = cmdNumCPU;
        }

        if (numWorkers <= 0) {
            numWorkers = numCPU;
        }

        m_database = siis::Database::builder(m_config->getDBType(),
                                            m_config->getDBHost(), m_config->getDBPort(), m_config->getDBName(),
                                            m_config->getDBUser(), m_config->getDBPwd());

        m_database->init();

        m_cache = Cache::builder(m_config->getCacheType(),
                                 m_config->getCacheHost(), m_config->getCachePort(), m_config->getCacheName(),
                                 m_config->getCacheUser(), m_config->getCachePwd());

        m_cache->init();

        m_poolWorker = new PoolWorker(numWorkers);
        m_poolWorker->init();

        // strategies
        o3d::Dir strategiesDir(o3d::Application::getAppPath() + "/strategies");
        if (!strategiesDir.exists()) {
            strategiesDir = o3d::Dir(o3d::FileManager::instance()->getWorkingDirectory() + "/strategies");
            if (!strategiesDir.exists()) {
                strategiesDir = o3d::Dir(o3d::Application::getAppPath() + "../strategies/");
            }
        }

        m_strategyCollection = new StrategyCollection(strategiesDir.getFullPathName());
        m_strategyCollection->init();

        if (m_config->getStrategy().isValid()) {
            if (!m_strategyCollection->has(m_config->getStrategy())) {
                O3D_ERROR(E_InvalidParameter(String("Strategy {0} not found").arg(m_config->getStrategy())));
            }
        }

        // supervisors
        o3d::Dir supervisorsDir(o3d::Application::getAppPath() + "/supervisors");
        if (!supervisorsDir.exists()) {
            supervisorsDir = o3d::Dir(o3d::FileManager::instance()->getWorkingDirectory() + "/supervisors");
            if (!supervisorsDir.exists()) {
                supervisorsDir = o3d::Dir(o3d::FileManager::instance()->getWorkingDirectory());
            }
        }

        m_supervisorCollection = new SupervisorCollection(supervisorsDir.getFullPathName());
        m_supervisorCollection->init();

        if (m_config->getSupervisor().isValid()) {
            if (!m_supervisorCollection->has(m_config->getSupervisor())) {
                O3D_ERROR(E_InvalidParameter(String("Supervisor {0} not found").arg(m_config->getSupervisor())));
            }
        }

        // run specified handler
        if (m_config->getHandlerType() == Config::HANDLER_LIVE) {
            m_handler = new Live();
        } else if (m_config->getHandlerType() == Config::HANDLER_BACKTEST) {
            m_handler = new Backtest();
        } else if (m_config->getHandlerType() == Config::HANDLER_LEARN) {
            m_handler = new Learning();
        } else if (m_config->getHandlerType() == Config::HANDLER_OPTIMIZE) {
            m_handler = new Optimization();
        } else {
            O3D_ERROR(E_InvalidParameter("Unsupported handler"));
        }

        if (m_config->isPaperMode()) {
            m_handler->setPaperMode(true);
        }

        if (m_config->isNoInteractive()) {
            m_displayer = new TtyDisplayer();
        } else {
            m_displayer = new NcursesDisplayer();
        }

        o3d::Int32 logLevel = cmd->getOptionValue('d').toInt32();

        m_displayer->init(m_config);
        m_displayer->setVerbosity(logLevel);
        o3d::Debug::instance()->getDefaultLog().setLogLevel(static_cast<o3d::Logger::LogLevel>(logLevel));

        m_handler->init(m_displayer, m_config, m_strategyCollection, m_poolWorker, m_database, m_cache);

        m_cache->start();
        m_database->start();
        m_handler->start();
    }

    void setupTerminal()
    {
        if (m_displayer) {
            m_displayer->init(m_config);
        }
    }

    void terminate()
    {
        if (m_handler) {
            m_handler->stop();
        }

        if (m_displayer) {
            m_displayer->terminate();
        }

        if (m_handler) {
            m_handler->terminate(m_config);
            o3d::deletePtr(m_handler);
        }

        if (m_poolWorker) {
            m_poolWorker->terminate();
            o3d::deletePtr(m_poolWorker);
        }

        if (m_strategyCollection) {
            m_strategyCollection->terminate();
            o3d::deletePtr(m_strategyCollection);
        }

        if (m_supervisorCollection) {
            m_supervisorCollection->terminate();
            o3d::deletePtr(m_supervisorCollection);
        }

        if (m_database) {
            m_database->stop();
            m_database->terminate();
            o3d::deletePtr(m_database);
        }

        if (m_cache) {
            m_cache->stop();
            m_cache->terminate();
            o3d::deletePtr(m_cache);
        }

        deletePtr(m_config);
        deletePtr(m_displayer);

        analysisLog = nullptr;
        orderLog = nullptr;

        deletePtr(m_analysisLog);
        deletePtr(m_orderLog);
    }

//    void message(const o3d::String msg)
//    {
//        int row, col;
//        getmaxyx(stdscr, row, col);

//        mvprintw(row-1, 0, msg.toUtf8().getData());
//    }

//    void clearMessage()
//    {
//        int row, col;
//        getmaxyx(stdscr, row, col);

//        o3d::String emptyRaw;
//        emptyRaw.setFill(' ', col);

//        mvprintw(row-1, 0, emptyRaw.toUtf8().getData());
//        refresh();
//    }

    void run()
    {
        bool running = true;
        bool waitExitConfirm = false;
        double prevProgress = 0.0;

        char c = 0;

        while (running) {
            // don't waste the CPU on the primary thread
            o3d::System::waitMs(10);

            if (!m_config->isNoInteractive()) {
                c = static_cast<char>(getch());

                if (c >= 32 && c <= 126) {
                    // @todo set pos
                    m_displayer->echo(static_cast<o3d::WChar>(c));

                    if (c == 'q' and !waitExitConfirm) {
                        waitExitConfirm = true;
                        m_displayer->display("notice", "Do you really want to quit ? [y/n] ");

                        c = 0;
                    } else if (waitExitConfirm) {
                        if (c == 'y' || c == 'Y') {
                            running = false;
                            c = 0;
                        } else if (c == 'n' || c == 'N') {
                            waitExitConfirm = false;
                            c = 0;
                            m_displayer->clear("notice");
                        } else {
                            m_displayer->display("notice", "Do you really want to quit ? [y/n] (again) ");
                        }
                    } else if (c == '?') {
                        m_poolWorker->ping();
                    } else {
                        m_displayer->clear("notice");
                        c = 0;
                    }
                }
            }

            if (catch_sigint) {
                // manual exit using CTRL+C
                catch_sigint = 0;
                if (m_config->isNoInteractive()) {
                    running = false;
                    m_displayer->display("notice", "CTRL-C hitted. Now quit ! ");
                } else {
                    waitExitConfirm = true;
                    m_displayer->display("notice", "Do you really want to quit ? [y/n] ");
                }
            }

            if (catch_sigquit) {
                // exit signal received
                catch_sigquit = 0;
                running = false;
            }

            if (catch_sigterm) {
                // exit signal received
                catch_sigterm = 0;
                running = false;
            }

            if (m_config->isNoInteractive()) {
                // display progression in percent
                if (m_handler->progress() < 100.0 && (m_handler->progress() - prevProgress >= 5.0)) {
                    m_displayer->display("notice", o3d::String::print("Progress %.2f%% ...", m_handler->progress()));
                    prevProgress = m_handler->progress();
                }

                if (m_handler->progress() >= 100.0) {
                    m_displayer->display("notice", o3d::String::print("Completed !"));
                    running = false;
                }
            } else {
                // update datetime in interactive mode
                o3d::DateTime dt;  // @todo localdatetime
                dt.fromTime(m_handler->timestamp(), False);
                m_displayer->display("time", dt.buildString("%Y-%m-%d %H:%M:%S"));
            }

            // sync display to main thread
            m_displayer->sync();
        }
    }

    // main entry
    static Int32 main()
    {
        System::print("Starting SiiS strategy", "siis");
        SiisStrategy *siisStrategy = new SiisStrategy();

        Processor processor;
        o3d::Int32 numCPU = processor.getNumCPU();
        o3d::Int32 res = 0;

        try {
            res = siisStrategy->init();
            if (res != 0) {
                siisStrategy->terminate();
                o3d::deletePtr(siisStrategy);

                return res;
            }

            siisStrategy->start(numCPU);
        } catch(E_BaseException &e) {
            System::print(e.getMsg(), "siis", System::MSG_ERROR);

            siisStrategy->terminate();
            o3d::deletePtr(siisStrategy);

            return -1;
        }

        System::print(String("Start with pool of {0} workers").arg(siisStrategy->m_poolWorker->getNumWorkers()), "siis");

        // as global instance
        SiisStrategy::ms_app = siisStrategy;
        siisStrategy->setupTerminal();

        try {
            siisStrategy->run();
        } catch(E_BaseException &e) {
            System::print(e.getMsg(), "siis", System::MSG_ERROR);
            // @todo might notify monitor service or auto restart as possible
        }

        System::print("Terminate...", "siis");

        siisStrategy->terminate();

        o3d::deletePtr(siisStrategy);
        SiisStrategy::ms_app = nullptr;

        System::print("Bye !", "siis");

        return 0;
    }

private:

    static SiisStrategy *ms_app;

    PoolWorker *m_poolWorker;
    Config *m_config;
    Dir m_siisPath;

    StrategyCollection *m_strategyCollection;
    SupervisorCollection *m_supervisorCollection;

    siis::Database *m_database;
    Cache *m_cache;
    Handler *m_handler;
    Displayer *m_displayer;

    Logger *m_analysisLog;
    Logger *m_orderLog;
};

class StrategyAppSettings : public AppSettings
{
public:

    StrategyAppSettings() : AppSettings()
    {
        sizeOfFastAlloc16 = 16384;
        sizeOfFastAlloc32 = 16384;
        sizeOfFastAlloc64 = 16384;
        useDisplay = false;
        clearLog = false;
    }
};

SiisStrategy* SiisStrategy::ms_app = nullptr;

O3D_CONSOLE_MAIN(SiisStrategy, StrategyAppSettings)
