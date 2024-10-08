# @date 2018-08-07
# @author Frederic SCHERMA
# @license Copyright (c) 2018 Dream Overflow
# Installer

import sys
import os
import time
import datetime
import subprocess
import pathlib
import traceback

def files_structure(options):
    """
    Create the initial file structure
    """
    print("Initialize file structure")

    config_path = "./"
    data_path = "./"

    home = pathlib.Path.home()
    if home.exists():
        if sys.platform == "linux":
            config_path = pathlib.Path(home, '.siis', 'config')
            log_path = pathlib.Path(home, '.siis', 'log')
            reports_path = pathlib.Path(home, '.siis', 'reports')
            markets_path = pathlib.Path(home, '.siis', 'markets')
            learning_path = pathlib.Path(home, '.siis', 'learning')
        elif sys.platform == "windows":
            app_data = os.getenv('APPDATA')

            config_path = pathlib.Path(home, app_data, 'siis', 'config')
            log_path = pathlib.Path(home, app_data, 'siis', 'log')
            reports_path = pathlib.Path(home, app_data, 'siis', 'reports')
            markets_path = pathlib.Path(home, app_data, 'siis', 'markets')
            learning_path = pathlib.Path(home, app_data, 'siis', 'learning')
        else:
            config_path = pathlib.Path(home, '.siis', 'config')
            log_path = pathlib.Path(home, '.siis', 'log')
            reports_path = pathlib.Path(home, '.siis', 'reports')
            markets_path = pathlib.Path(home, '.siis', 'markets')
            learning_path = pathlib.Path(home, '.siis', 'learning')
    else:
        # uses cwd
        home = pathlib.Path(os.getcwd())

        config_path = pathlib.Path(home, 'user', 'config')
        log_path = pathlib.Path(home, 'user', 'log')
        reports_path = pathlib.Path(home, 'user', 'reports')
        markets_path = pathlib.Path(home, 'user', 'markets')
        learning_path = pathlib.Path(home, 'user', 'learning')

    # config/
    if not config_path.exists():
        config_path.mkdir(parents=True)

    options['config-path'] = str(config_path)

    # markets/
    if not markets_path.exists():
        markets_path.mkdir(parents=True)

    options['markets-path'] = str(markets_path)

    # reports/
    if not reports_path.exists():
        reports_path.mkdir(parents=True)

    options['reports-path'] = str(reports_path)

    # learning/
    if not learning_path.exists():
        learning_path.mkdir(parents=True)

    options['learning-path'] = str(learning_path)

    # log/
    if not log_path.exists():
        log_path.mkdir(parents=True)

    options['log-path'] = str(log_path)
    
def application(argv):
    options = {
        'config-path': './user/config',
        'log-path': './user/log',
        'reports-path': './user/reports',
        'markets-path': './user/markets',
        'learning-path': './user/learning',
        'log-name': 'siis.log'
    }

    # create initial siis data structure if necessary
    files_structure(options)

    # create database table
    # @todo howto, we need option mysql|postgresql or autodetect with mysql as default
    # if create the db need the admin name and password
    # need dbname, user and password, may prompt them

if __name__ == "__main__":
    application(sys.argv)

