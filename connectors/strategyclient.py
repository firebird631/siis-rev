# @date 2019-04-02
# @author Frederic SCHERMA
# @license Copyright (c) 2019 SIIS
# Strategy client for server connector

import time
import traceback
import threading
import zmq

import logging
logger = logging.getLogger('siis.connector.strategyclient')


class StrategyClientWorker(threading.Thread):

    def __init__(self, workder_id, context):
        threading.Thread.__init__ (self)
        self._workder_id = workder_id
        self._context = context
        self._running = True

    def stop(self):
        self._running = False

    def run(self):
        worker = self._context.socket(zmq.DEALER)
        worker.connect("inproc://backend")
        logger.info("Strategy worker started")

        poller = zmq.Poller()
        poller.register(worker, zmq.POLLIN)

        # poller.register(worker, zmq.POLLOUT)

        while self._running:
            socks = dict(poller.poll())
            if worker in socks and socks[worker] == zmq.POLLIN: 
                ident, msg = worker.recv(zmq.DONTWAIT)
                print("Worker %s received msg from %s" % (self._workder_id, ident,))

            # @todo
            # for msg in self._send_queue:
            #    #worker.send([ident, msg])

        worker.close()


class StrategyClient(object):
    """
    Strategy client for server connector
    """

    def __init__(self, identifier, host):
        self._identifier = identifier
        self._host = host

        self._subscriptions_ids = []

    @property
    def identifier(self):
        """Integer unique identifier"""
        return self._identifier

    @property
    def host(self):
        """Str distant host name"""
        return self._host
    
    def add_subscribtion_id(self, sub_id):
        self._subscriptions_ids.append(sub_id)

    def remove_subscription_id(self, sub_id):
        if sub_id in self._subscriptions_ids:
            del self._subscriptions_ids[sub_id]

    def subscribtions_ids(self):
        return self._subscriptions_ids
