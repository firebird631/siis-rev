# @date 2019-03-04
# @author Romain Locci
# @author Frederic SCHERMA
# @license Copyright (c) 2019 SIIS
# server for connectors communications

import time
import json
import zmq
import threading
import traceback

from zmq.utils.monitor import recv_monitor_message
from strategyclient import StrategyClient, StrategyClientWorker

import logging
logger = logging.getLogger('siis.connector.server')


class ConnectorServer(object):
    """
    Connector Server Class.
    Is a class to manage each connector
    """

    NUM_WORKERS = 2

    DEFAULT_USE_BENCH = False
    MAX_BENCH_SAMPLES = 30

    SUBSCRIBE = 1
    UNSUBSCRIBE = 2
    SEND_TICK = 3
    SEND_OHLC = 4
    SEND_TICK_ARRAY = 5
    SEND_TICK_AGGREGGED = 6
    SEND_OHLC_ARRAY = 7
    SEND_OHLC_AGGREGGED = 8
    SEND_ORDER_BOOK = 9

    STATUS_SIGNAL = 20
    ACCOUNT_SIGNAL = 21
    ASSET_SIGNAL = 22
    ASSET_AGGREGED_SIGNAL = 22
    MARKET_SIGNAL = 23
    POSITION_SIGNAL = 24
    ORDER_SIGNAL = 25

    LIST_ORDERS = 40
    LIST_POSITIONS = 41
    CREATE_ORDER = 42
    CANCEL_ORDER = 43
    CLOSE_POSITION = 44
    MODIFY_POSITION = 45

    def __init__(self, options):
        self._name = options['connector-config'].rstrip('.json')
        self._connector = options['connector']['name']
        self._connectorClass = None

        self._running = False
        self._quit = False
        self._thread = threading.Thread(name="siis.connector", target=self.run)
        self._error = None

        self._context = None
        self._frontend = None
        self._monitor = None
        self._backend = None

        self._bench = self.DEFAULT_USE_BENCH
        self._last_time = []
        self._worst_time = 0
        self._avg_time = 0

        self._workers = []
        self._strategy_clients = []
        self._next_sid = 1

    def set_connectorClass(self, connectorClass):
        self._connectorClass = connectorClass

    def init(self, options):
        """
        Init RPC server and Stream server
        """
        self._context = zmq.Context()

        # socket for listening
        self._frontend = self._context.socket(zmq.ROUTER)
        self._frontend.bind("%s://%s:%s" % (
                options['strategy'].get('protocol', "tcp"),
                options['strategy'].get('host', "127.0.0.1"),
                options['strategy'].get('port', 5555)))

        self._monitor = self._frontend.get_monitor_socket(zmq.EVENT_CONNECTED | zmq.EVENT_DISCONNECTED)

        # socket for dispatch
        self._backend = self._context.socket(zmq.DEALER)
        self._backend.bind('inproc://backend')

    def terminate(self):
        if self._running:
            self.stop()
            self._thread.join()

        self._poller = None

        if self._monitor:
            if self._frontend:
                self._frontend.disable_monitor()

            self._monitor.close()
            self._monitor = None

        if self._frontend:
            self._frontend.close()
            self._frontend = None

        if self._backend:
            self._backend.close()
            self._backend = None

        if self._context:
            self._context.term()
            self._context = None

    def start(self):
        if not self._running:
            self._running = True
            try:
                self._thread.start()
            except Exception as e:
                self._running = False
                logger.error(repr(e))
                return False

            return True
        else:
            return False

    def stop(self):
        self._quit = True

    def __process_once_bench(self):
        begin = time.time()

        self.process(self)

        self._last_time.append(time.time() - begin)
        self._worst_time = max(self._worst_time, self._last_time[-1])
        self._avg_time = sum(self._last_time) / len(self._last_time)

        if len(self._last_time) > Runnable.MAX_BENCH_SAMPLES:
            self._last_time.pop(0)

    def __process_once(self):
        self.process()

    def process(self):
        return
        socks = dict(self._poller.poll())

        # received a RPC server request
        if self._socketRep in socks and socks[self._socketRep] == zmq.POLLIN:
            message = self._socketRep.recv()
            func_id = int(message[0])
            
            # Receive SUBSCRIBE message
            if func_id == self.SUBSCRIBE:
                
                msg = messageSubscribe(None)
                msg.read(message)

                if messageSubscribe.Type[msg.type_sub] == 'TYPE_TICK':
                    msg.subId = self._connectorClass.subscribe_tick(msg.marketId)
                
                elif (messageSubscribe.Type[msg.type_sub] == 'TICK_OHLC_MID') or (messageSubscribe.Type[msg.type_sub] == 'TICK_OHLC_BID') or (messageSubscribe.Type[msg.type_sub] == 'TICK_OHLC_OFR'):
                    msg.subId = self._connectorClass.subscribe_ohlc(msg.marketId, msg.timeframe)
                
                elif messageSubscribe.Type[msg.type_sub] == 'TICK_ORDER_BOOK':
                    msg.subId = self._connectorClass.subscribe_order_book(msg.marketId, msg.bookdepth)
                

                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive UNSUBSCRIBE message
            elif func_id == self.UNSUBSCRIBE:
                
                msg = messageUnsubscribe(None)
                msg.read(message)
                msg.subId = self._connectorClass.unsubscribe(msg.marketId)
                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive LISTORDERS message
            elif func_id == self.LISTORDERS:
                
                msg = messageListOrders(None)
                msg.read(message)
                msg.set_list_orders(self._connectorClass.list_positions())
                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive LISTPOSITIONS message
            elif func_id == self.LIST_POSITIONS:
                
                msg = messageListPositions(None)
                msg.read(message)
                msg.set_list_positions(self._connectorClass.list_positions())
                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive CREATEORDER message
            elif func_id == self.CREATE_ORDER:
                
                msg = messageCreateOrder(None)
                msg.read(message)
                self._connectorClass.create_order(msg.market_id(), msg.direction(), msg.order_type(), msg.order_price(), msg.quantity(), msg.stop_loss_price(), msg.take_profit_price(), msg.leverage())
                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive CANCELORDER message
            elif func_id == self.CANCEL_ORDER:
                
                msg = messageCancelOrder(None)
                msg.read(message)
                self._connectorClass.cancel_order(msg.order_id())
                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive CLOSEPOSITION message
            elif func_id == self.CLOSE_POSITION:
                
                msg = messageClosePosition(None)
                msg.read(message)
                self._connectorClass.cancel_position(msg.position_id())
                msg.write()
                self._socketRep.send(msg.messageReturn)

            # Receive MODIFYPOSITION message
            elif func_id == self.MODIFY_POSITION:

                msg = messageModifyPosition(None)
                msg.read(message)
                self._connectorClass.modify_position(msg.position_id(), msg.stop_loss_price(), msg.take_profit_price())
                msg.write()
                self._socketRep.send(msg.messageReturn)

        # need to stream a message (or not)
        if self._socketPub in socks and socks[self._socketPub] == zmq.POLLOUT:
            pass    
            #send a message like ohlc, tick, ... if it's necessary
            #type_message = "TICK"
            #messagedata = "...."
            #self._socketPub.send(("%s %s" % (type_message, messagedata)).encode('utf8'))

        time.sleep(0.001)  # yield (@todo adjust)

    def send_tick(self, marketId, timestamp, bid, ofr_ask, volume):
        msg = messageTick(None)
        msg.set(marketId, timestamp, bid, ofr_ask, volume)
        msg.write()
        self._socketPub.send(msg.messageReturn)

    def send_ohlc(self, marketId, timestamp, timeframe, open_, high, low, close, volume, consolidated):
        msg = messageOhlc(None)
        msg.set(marketId, timestamp, timeframe, open_, high, low, close, volume, consolidated)
        msg.write()
        self._socketPub.send(msg.messageReturn)

    def monitor(self):
        while self._monitor.poll():
            evt = recv_monitor_message(self._monitor)
            print(evt)

            if evt['event'] == zmq.EVENT_CONNECTED:
                print("conn")
            elif evt['event'] == zmq.EVENT_DISCONNECTED:
                print("disc")
           
            if evt['event'] == zmq.EVENT_MONITOR_STOPPED:
                break

            print("tttttttttttt")

        print("tutititi")

    def run(self):
        self._quit = False

        for i in range(self.NUM_WORKERS):
            worker = StrategyClientWorker(i, self._context)
            worker.start()

            self._workers.append(worker)

        # self._frontend.monitor("inproc://events", zmq.EVENT_CONNECTED | zmq.EVENT_DISCONNECTED);

        self._monitor_thread = threading.Thread(target=self.monitor)
        self._monitor_thread.start()

        # zmq.proxy(self._frontend, self._backend)
        print("iiiiii")

        # don't waste with try/catch, do it only at last level
        # restart the loop if exception thrown
        if self._bench:
            while self._running:
                try:
                    while self._running:
                        self.__process_once_bench()

                        if self._quit:
                            if self.check_exit():
                                self._running = False

                except Exception as e:
                    logger.error(traceback.format_exc())
                    print(repr(e))
                    self._error = e
        else:
            while self._running:
                print("oooooo")
                try:
                    while self._running:
                        self.__process_once()

                        if self._quit:
                            if self.check_exit():
                                self._running = False

                except Exception as e:
                    logger.error(traceback.format_exc())
                    print(repr(e))
                    self._error = e

        for worker in self._workers:
            worker.stop()

        self._running = False

    def check_exit(self):
        # @todo check if req message to process before kill and then :
        return True

    def sync(self):
        """
        Do the synchrone stuff with the handler thread.
        """
        pass

    #
    # strategy client
    #

    def add_strategy_client(self, host):
        # sid = @todo how to know used socket port ?
        sid = self._next_sid
        self._next_sid += 1
        self._strategy_clients[s_id] = StrategyClient(sid, host)

    def remove_strategy_client(self, host):
        for strategy_client in self._strategy_clients:
            if strategy_client.host == host:
                del self._strategy_clients[strategy_client.identifier]
                break
