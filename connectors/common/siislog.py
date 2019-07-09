# @date 2018-08-24
# @author Frederic SCHERMA
# @license Copyright (c) 2018 Dream Overflow
# Siis logger

import copy
import logging
import colorama


class ColoredFormatter(logging.Formatter):

    def __init__(self, msg, use_color = True):
        logging.Formatter.__init__(self, msg)
        self.use_color = use_color

    def colors(self, style):
        return {
            'DEFAULT': colorama.Style.RESET_ALL,
            'ERROR': colorama.Fore.RED,
            'WARNING': colorama.Back.YELLOW + colorama.Fore.WHITE,
            'ACTION': colorama.Fore.YELLOW,
            'NOTICE': colorama.Fore.CYAN,
            'HIGH': colorama.Fore.GREEN,
            'LOW': colorama.Fore.MAGENTA,
            'NEUTRAL':colorama.Fore.WHITE,
            'HIGHLIGHT': colorama.Style.BRIGHT}

    def format(self, record):
        colors = self.colors("uterm")

        if record.levelno == logging.ERROR and self.use_color:
            record.name = colors['ERROR'] + '- ' + copy.copy(record.name) + colors["DEFAULT"] + ' '
            record.levelname = colors['ERROR'] + copy.copy(record.levelname) + colors["DEFAULT"]
            record.msg = colors['ERROR'] + copy.copy(str(record.msg)) + colors["DEFAULT"]
            return logging.Formatter.format(self, record)

        elif record.levelno == logging.WARNING and self.use_color:
            record.name = colors["WARNING"] + '- ' + copy.copy(record.name) + colors["DEFAULT"] + ' '
            record.levelname = colors["WARNING"] + '- ' + copy.copy(record.levelname) + colors["DEFAULT"] + ' '
            record.msg = colors["WARNING"] + copy.copy(str(record.msg)) + colors["DEFAULT"]
            return logging.Formatter.format(self, record)

        elif record.levelno == logging.INFO and self.use_color:
            record.name = ''
            record.levelname = colors["HIGHLIGHT"] + '- ' + copy.copy(record.levelname) + colors["DEFAULT"] + ' '
            record.msg = colors["HIGHLIGHT"] + copy.copy(str(record.msg)) + colors["DEFAULT"]
            return logging.Formatter.format(self, record)

        elif record.levelno == logging.DEBUG and self.use_color:
            record.name = colors["HIGHLIGHT"] + '- ' + copy.copy(record.name) + colors["DEFAULT"] + ' '
            record.levelname = colors["HIGHLIGHT"] + '- ' + copy.copy(record.levelname) + colors["DEFAULT"] + ' '
            record.msg = colors["HIGHLIGHT"] + copy.copy(str(record.msg)) + colors["DEFAULT"]
            return logging.Formatter.format(self, record)

        else:
            return logging.Formatter.format(self, record)


class TerminalHandler(logging.StreamHandler):

    def __init__(self):
        logging.StreamHandler.__init__(self)

    def emit(self, record):
        msg = self.format(record)

        if record.levelno == logging.ERROR:
            print(str(msg))
        elif record.levelno == logging.WARNING:
            print(str(msg))
        elif record.levelno == logging.INFO:
            print(str(msg))
        elif record.levelno == logging.DEBUG:
            print(str(msg))
        else:
            print(str(msg))


class SiisLog(object):
    """
    Siis logger initialized based on python logger.
    """

    def __init__(self, options, style=''):
        # if init before terminal
        colorama.init()

        # stderr to terminal in info level
        self.console = TerminalHandler()  #  logging.StreamHandler()
        self.console.setLevel(logging.INFO)

        # self.term_formatter = logging.Formatter('- %(name)-12s: %(levelname)-8s %(message)s')
        self.term_formatter = ColoredFormatter('%(name)-s%(message)s', style)
        self.console.setFormatter(self.term_formatter)

        # add the handler to the root logger
        logging.getLogger('').addHandler(self.console)

        # default log file formatter
        self.file_formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')

        # and a siis logger with siis.log
        self.file_logger = logging.FileHandler(options['log-path'] + '/' + options['log-name'])
        self.file_logger.setFormatter(self.file_formatter)

        self.my_logger = self.add_file_logger('siis', self.file_logger)

    def upgrade(self, options):
        """
        Replace generic siis.log by dedicated loggers for this specific connector configuration.
        """
        conn = options.get('connector', {}).get('name')

        self.my_logger.removeHandler(self.file_logger)
        self.file_logger = None

        # a siis logger with <connector>.connector.siis.log
        self.file_logger = logging.FileHandler(options['log-path'] + '/' + "%s.%s" % (conn, options['log-name']))
        self.file_logger.setFormatter(self.file_formatter)
        self.file_logger.setLevel(logging.INFO)

        self.add_file_logger('siis', self.file_logger)

        # a siis logger with exec.<connector>.exec.connector.siis.log
        self.exec_file_logger = logging.FileHandler(options['log-path'] + '/' + "%s.exec.%s" % (conn, options['log-name']))
        self.exec_file_logger.setFormatter(self.file_formatter)
        self.exec_file_logger.setLevel(logging.INFO)

         # don't propagate execution to siis logger
        self.add_file_logger('siis.exec', self.exec_file_logger, False)

        # a siis logger with error.<connector>.error.connector.siis.log
        self.error_file_logger = logging.FileHandler(options['log-path'] + '/' + "%s.error.%s" % (conn, options['log-name']))
        self.error_file_logger.setFormatter(self.file_formatter)
        self.error_file_logger.setLevel(logging.INFO)

        # don't propagate error trade to siis logger
        self.add_file_logger('siis.error', self.error_file_logger, False)

    def add_file_logger(self, name, handler, level=logging.DEBUG, propagate=True):
        my_logger = logging.getLogger(name)

        my_logger.addHandler(handler)
        my_logger.setLevel(level)
        my_logger.propagate = propagate

        return my_logger
