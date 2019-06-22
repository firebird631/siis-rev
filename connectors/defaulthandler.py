# @date 2019-03-29
# @author Frederic SCHERMA
# @license Copyright (c) 2019 SIIS
# Application handler interface

import time

from importlib import import_module
from handler import Handler
from connectorserver import ConnectorServer
# from monitorserver import MonitorServer

import logging
logger = logging.getLogger('siis.handler')


class DefaultHandler(Handler):

    def __init__(self, options):
        super().__init__()

        self._strategy_server = ConnectorServer(options)
        # self._monitor_server = MonitorServer(options)
        self._connector = self.build_connector(options, options['connector'].get('name', ""))
        self._strategies_clients = []

    def init(self, options):
        self._strategy_server.init(options)
        # self._monitor_server.init(options)
        self._connector.init(options)

    def start(self):
        # communication servers
        self._strategy_server.start()
        # self._monitor_server.start()

        # exchance connector
        self._connector.connect()

        if self._connector.connected:
            self._connector.initial_fetch()
            self._connector.initial_subscriptions()

    def stop(self):
        self._strategy_server.stop()

        if self._connector.connected:
            self._connector.disconnect()

    def terminate(self):
        self._strategy_server.terminate()
        # self._monitor_server.terminate()
        self._connector.terminate()

    def run_once(self):
        self._strategy_server.sync()
        self._connector.sync(self._strategies_clients)

        time.sleep(0.001)  # yield (@todo adjust)

    def build_connector(self, options, name):
        connector = options['connectors'].get(name)
        if not connector:
            logger.error("Connector %s not found !" % name)
            return None

        # retrieve the classname and instanciate it
        parts = connector.get('classpath').split('.')

        module = import_module('.'.join(parts[:-1]))
        Clazz = getattr(module, parts[-1])

        return Clazz(self, options)
