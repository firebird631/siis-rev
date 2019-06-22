# @date 2019-03-04
# @author Romain Locci
# @author Frederic SCHERMA
# @license Copyright (c) 2019 SIIS
# Connector main entry.

from __init__ import APP_VERSION, APP_SHORT_NAME, APP_LONG_NAME

# import signal
import threading
import sys
import os
import json
import time
import datetime
import logging
import pathlib
import traceback

from common.utils import UTC, fix_thread_set_name, TIMEFRAME_FROM_STR_MAP

from database.database import Database

from common.siislog import SiisLog
from defaulthandler import DefaultHandler


# def signal_handler(sig, frame):
#     print('Press CTRL-C again to confirm exit !')


def has_exception(siis_logger, e):
    siis_logger.error(repr(e))
    siis_logger.error(traceback.format_exc())


def display_cmd_line_help():
    print('%s command line usage:' % APP_LONG_NAME)
    print("")
    print(" python main.py -c=<connector_filename.json> <--options>")
    print("")
    print("  -h --help display command line help.")
    print("  -v --version display the version number.")
    print("  -c --connector <connector_filename.json> Defined which connector configuration to starts with.")
    print("  -F --fetch Defined which connector configuration for broker and indenty details and --from --to --timeframe.")
    print("  -S --spec Optional specific options for the fecher.")
    print("  -f --from <datetime> Define the from datetime in YYYY-mm-ddTHH:MM:SS.")
    print("  -t --to <datetime> Define the to datetime in YYYY-mm-ddTHH:MM:SS.")
    print("  -l --last <number> Define the number of last price to fetch (not compatible with --to option).")
    print("  -s --timeframe <timeframe> Define the base timeframe to fetch.")
    print("  -C --cascaded <max-timeframe> Define the max generated timeframe from fetched base timeframe.")
    print("  -m --market <market> Define the market to fetch.")
    # print("  -b --binarize Defined which connector configuration for broker and indenty details and --from --to --timeframe.")
    print("")
    print(" During usage hit CTRL-C twice to exit.")


def install(options):
    config_path = "./"
    data_path = "./"

    home = pathlib.Path.home()
    if home.exists():
        if sys.platform == "linux":
            config_path = pathlib.Path(home, '.siis', 'config')
            connectors_path = pathlib.Path(home, '.siis', 'config', 'connectors')
            log_path = pathlib.Path(home, '.siis', 'log')
            reports_path = pathlib.Path(home, '.siis', 'reports')
            markets_path = pathlib.Path(home, '.siis', 'markets')
        elif sys.platform == "windows":
            app_data = os.getenv('APPDATA')

            config_path = pathlib.Path(home, app_data, 'siis', 'config')
            connectors_path = pathlib.Path(home, app_data, 'siis', 'config', 'connectors')
            log_path = pathlib.Path(home, app_data, 'siis', 'log')
            reports_path = pathlib.Path(home, app_data, 'siis', 'reports')
            markets_path = pathlib.Path(home, app_data, 'siis', 'markets')
        else:
            config_path = pathlib.Path(home, '.siis', 'config')
            connectors_path = pathlib.Path(home, '.siis', 'config', 'connectors')
            log_path = pathlib.Path(home, '.siis', 'log')
            reports_path = pathlib.Path(home, '.siis', 'reports')
            markets_path = pathlib.Path(home, '.siis', 'markets')
    else:
        # uses cwd
        home = pathlib.Path(os.getcwd())

        config_path = pathlib.Path(home, 'user', 'config')
        connectors_path = pathlib.Path(home, 'user', 'config', 'connectors')
        log_path = pathlib.Path(home, 'user', 'log')
        reports_path = pathlib.Path(home, 'user', 'reports')
        markets_path = pathlib.Path(home, 'user', 'markets')

    # config/
    if not config_path.exists():
        config_path.mkdir(parents=True)

    options['config-path'] = str(config_path)

    # config/connectors/
    if not connectors_path.exists():
        connectors_path.mkdir(parents=True)

    options['connectors-path'] = str(connectors_path)

    # markets/
    if not markets_path.exists():
        markets_path.mkdir(parents=True)

    options['markets-path'] = str(markets_path)

    # reports/
    if not reports_path.exists():
        reports_path.mkdir(parents=True)

    options['reports-path'] = str(reports_path)

    # log/
    if not log_path.exists():
        log_path.mkdir(parents=True)

    options['log-path'] = str(log_path)


# def do_binarizer(options, siis_logger):
#     from database.tickstorage import TextToBinary

#     print("Starting SIIS binarizer...")

#     timeframe = -1

#     if not options.get('timeframe'):
#         timeframe = 60  # default to 1min
#     else:
#         if options['timeframe'] in TIMEFRAME_FROM_STR_MAP:
#             timeframe = TIMEFRAME_FROM_STR_MAP[options['timeframe']]
#         else:
#             try:
#                 timeframe = int(options['timeframe'])
#             except:
#                 pass

#     if timeframe < 0:
#         siis_logger.error("Invalid timeframe !")
#         sys.exit(-1)

#     converter = TextToBinary(options['markets-path'], options['connector'], options['market'], options.get('from'), options.get('to'))
#     converter.process()

#     print("Binarization done!")

#     sys.exit(0)


# def do_fetcher(options, siis_logger):
#     print("Starting SIIS fetcher using %s identity..." % options['identity'])

#     # database manager
#     Database.create(options)
#     Database.inst().setup(options)

#     watcher_service = WatcherService(options)
#     fetcher = watcher_service.create_fetcher(options['connector'])

#     timeframe = -1
#     cascaded = None

#     if not options.get('timeframe'):
#         timeframe = 60  # default to 1min
#     else:
#         if options['timeframe'] in TIMEFRAME_FROM_STR_MAP:
#             timeframe = TIMEFRAME_FROM_STR_MAP[options['timeframe']]
#         else:
#             try:
#                 timeframe = int(options['timeframe'])
#             except:
#                 pass

#     if not options.get('cascaded'):
#         cascaded = None
#     else:
#         if options['cascaded'] in TIMEFRAME_FROM_STR_MAP:
#             cascaded = TIMEFRAME_FROM_STR_MAP[options['cascaded']]
#         else:
#             try:
#                 cascaded = int(options['cascaded'])
#             except:
#                 pass

#     if timeframe < 0:
#         siis_logger.error("Invalid timeframe")
#         sys.exit(-1)

#     try:
#         fetcher.connect()
#     except:
#         sys.exit(-1)

#     if fetcher.connected:
#         siis_logger.info("Fetcher authentified to %s, trying to collect data..." % fetcher.name)

#         markets = fetcher.matching_symbols_set(options['market'].split(','), fetcher.available_instruments())

#         try:
#             for market_id in markets:
#                 if not fetcher.has_instrument(market_id, options.get('spec')):
#                     siis_logger.error("Market %s not found !" % (market_id,))
#                 else:
#                     fetcher.fetch_and_generate(market_id, timeframe,
#                         options.get('from'), options.get('to'), options.get('last'),
#                         options.get('spec'), cascaded)
#         except KeyboardInterrupt:
#             pass
#         finally:
#             fetcher.disconnect()

#     fetcher = None

#     print("Flushing database...")
#     Database.terminate()

#     print("Fetch done!")
#     sys.exit(0)


def run_once(logger, handler):
    handler.run_once()


def run(logger, handler):
    running = True
    sig_int_count = 0
    sig_int_time = 0

    # don't waste with try/catch, do it only at last level
    # restart the loop if exception thrown
    while running:
        try:
            while running:
                run_once(logger, handler)

                if time.time() - sig_int_time > 2.0:
                    sig_int_count = 0

        except KeyboardInterrupt:
            if sig_int_count == 0:
                sig_int_time = time.time()
                print("Confirm with a second CTRL+C within 2 secondes !")

            sig_int_count += 1
            if sig_int_count > 1:
                break
        except Exception as e:
            has_exception(logger, e)


def parse_config(logger, options):
    try:
        with open('/'.join((options['config-path'], options['default'])), 'r') as conf:
            data = json.load(conf)

        if 'database' in data:
            options['database'] = {**options['database'], **data['database']}

    except Exception as e:
        has_exception(logger, e)
        raise


def parse_connector_spec(logger, options):
    try:
        with open('/'.join((options['connectors-path'], options['connector-config'])), 'r') as conf:
            data = json.load(conf)

        if 'connector' in data:
            options['connector'] = {**options['connector'], **data['connector']}

        if 'strategy' in data:
            options['strategy'] = {**options['strategy'], **data['strategy']}

        if 'markets' in data:
            options['markets'] = {**options['markets'], **data['markets']}

    except Exception as e:
        has_exception(logger, e)
        raise


def application(argv):
    fix_thread_set_name()

    options = {
        'identity': 'real',
        'config-path': './user/config',
        'connectors-path': './user/config/connectors',
        'log-path': './user/log',
        'reports-path': './user/reports',
        'markets-path': './user/markets',
        'log-name': 'connector.log',
        'default': "connector.json",
        'connector-config': "",
        'database': {
            'name': "siis",
            'type': "pgsql",
            'host': "127.0.0.1",
            'port': 5432,
            'user': "siis",
            'password': "siis"
        },
        'cache': {
            'name': "siis",
            'type': "redis",
            'host': "127.0.0.1",
            'port': 6379,
            'user': "siis",
            'password': "siis"
        },
        'strategy': {
            'protocol': "tcp",
            'host': "127.0.0.1",
            'port': 5600
        },
        'connector': {
            'name': "",
            'host': ""
        },
        'markets': {},
        'connectors': {
            'binance.com': {
                'classpath': 'connectors.binance.connector.BinanceConnector',
            },  
            'bitmex.com': {
                'classpath': 'connectors.bitmex.connector.BitMexConnector',
            },
            'ig.com': {
                'classpath': 'connectors.ig.connector.IGConnector',
            },
        }
    }

    # create initial siis data structure if necessary
    install(options)

    siis_log = SiisLog(options, "uterm")
    siis_logger = logging.getLogger('siis.connector')

    if len(argv) > 1:
        # utc or local datetime ?
        for n, arg in enumerate(argv):
            if arg.startswith('-'):
                if (arg == '--fetch' or arg == '-F'):
                    # use the fetcher
                    options['fetch'] = True
                elif (arg == '--spec' or arg == '-S') and n+1 < len(argv):
                    # fetcher data history option
                    options['option'] = argv[n+1]

                elif arg == '--binarize':
                    # use the binarizer
                    options['binarize'] = True

                elif (arg == '--connector' or arg == '-c') and n+1 < len(argv):
                    # connector conf filename
                    options['connector-config'] = argv[n+1]

                elif (arg == '--from' or arg == '-f') and n+1 < len(argv):
                    # if backtest from date (if ommited use whoole data) date format is "yyyy-mm-dd-hh:mm:ss", fetch, binarize to date
                    options['from'] = datetime.datetime.strptime(argv[n+1], '%Y-%m-%dT%H:%M:%S').replace(tzinfo=UTC())
                elif (arg == '--to' or arg == '-t') and n+1 < len(argv):
                    # if backtest to date (can be ommited), fetch, binarize to date
                    options['to'] = datetime.datetime.strptime(argv[n+1], '%Y-%m-%dT%H:%M:%S').replace(tzinfo=UTC())
                elif (arg == '--last' or arg == '-l') and n+1 < len(argv):
                    # fetch the last n data history
                    options['last'] = int(argv[n+1])

                elif (arg == '--market' or arg == '-m') and n+1 < len(argv):
                    # fetch, binarize the data history for this market
                    options['market'] = argv[n+1]

                elif (arg == '--timeframe' or arg == '-s') and n+1 < len(argv):
                    # fetch, binarize base timeframe
                    options['timeframe'] = argv[n+1]
                elif (arg == '--cascaded=' or arg == '-C') and n+1 < len(argv):
                    # fetch cascaded ohlc generation
                    options['cascaded'] = argv[n+1]

                elif arg == '--read-only':
                    options['read-only'] = True
                elif arg == '--check-data':
                    options['check-data'] = True

                elif arg == '--version' or arg == '-v':
                    print('%s %s' % (APP_SHORT_NAME, '.'.join([str(x) for x in APP_VERSION])))
                    sys.exit(0)

                elif arg == '--help' or arg == '-h':
                    display_cmd_line_help()
                    sys.exit(0)

        # replay
        if options.get('replay', False):
            if options.get('from') is None or options.get('to') is None:
                del options['replay']
                print("Replay need from= and to= date time")
                sys.exit(-1)

    if not options['connector-config']:
        print("Connector configuration filename must be specified")
        sys.exit(-1)

    #
    # config
    #

    parse_config(siis_logger, options)
    parse_connector_spec(siis_logger, options)

    #
    # binarizer
    #

    if options.get('binarize'):
        if options.get('market') and options.get('from') and options.get('to') and options.get('connector'):
            do_binarizer(options, siis_logger)
        else:
            display_cmd_line_help()

        sys.exit(0)

    #
    # fetcher mode
    #

    if options.get('fetch'):
        if options.get('market') and options.get('connector') and options.get('timeframe'):
            do_fetcher(options, siis_logger)
        else:
            display_cmd_line_help()

        sys.exit(0)

    #
    # running mode
    #

    print("Starting SIIS simple runner using %s..." % options['connector-config'])
    print("Hit CTRL-C twice to terminate")

    if options.get('replay'):  
        print("Process a replay.")

    # monitoring service
    # print("Starting monitor service...")
    # monitor_service = MonitorService(options)
    # monitor_service.start()

    # database manager
    Database.create(options)
    Database.inst().setup(options)

    print("Starting connector handler...")
    handler = DefaultHandler(options)

    handler.init(options)
    handler.start()

    run(siis_logger, handler)

    print("Terminate...")
    handler.stop()
    handler.terminate()

    handler = None

    print("Saving database...")
    Database.terminate()

    print("Bye!")


if __name__ == "__main__":
    application(sys.argv)
