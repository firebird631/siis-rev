# @date 2019-03-04
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# connector for IG communications

import requests
import urllib
import json
import time
import datetime
import base64
import threading

from collections import deque

from database.database import Database
from common.utils import matching_symbols_set, TIMEFRAME_TO_STR_MAP, basetime

from constants import *

import logging
logger = logging.getLogger('siis.connector')


class Subscription(object):

    def __init__(self, _sub_id, _market_id, _timeframe, _depth):
        self.sub_id = _sub_id
        self.market_id = _market_id
        self.timeframe = _timeframe
        self.depth = _depth
        self.strategies = []

    def add_strategy_client(self, strategy_client):
        self.strategies.append(strategy_client)

    def remove_strategy_client(self, strategy_client):
        del self.strategies[strategy_client]


class Connector(object):
    """
    Base connector interface.
    """

    MSG_UNDEFINED = 0
    MSG_MARKET = 1        # market info change
    MSG_AGG_MARKET = 1    # market info change
    MSG_TICK = 2          # a new tick for a market
    MSG_AGG_TICK = 3      # new tick for any subscribed markets
    MSG_OHLC = 4          # a new ohlc for a market
    MSG_AGG_OHLC = 5      # new ohlc for any subscribed markets
    MSG_ACCOUNT = 6       # account margin/balance update
    MSG_ORDER = 6         # order rejected/created/modified/traded/deleted
    MSG_POSITION = 7      # position updated/modified/deleted
    MSG_ASSET = 8         # asset update for a market
    MSG_AGG_ASSET = 9     # asset update for any subscribed markets

    # ohlc timeframes of interest for storage
    STORED_TIMEFRAMES = (
        # TF_MIN,
        # TF_5MIN,
        # TF_15MIN,
        # TF_HOUR,
        TF_4HOUR,
        TF_DAY,
        TF_WEEK)

    # connection retry delay in seconds
    CONNECTION_RETRY_DELAY = 2.0

    def __init__(self, name, server, options):
        self._mutex = threading.RLock()  # reentrant locker
        self._name = name
        self._connected = False
        self._server = server
        self._trader = None

        # list of configured market-id to be initially fetched and watched
        self._default_instruments = self.configured_symbols(options)
        self._available_instruments = set()  # all avalaibles instruments if listing is possible
        self._watched_instruments = set()    # watched instruments

        self._last_tick = {}  # last tick per market id
        self._last_ohlc = {}  # last ohlc per market id and per timeframe (could have not the same timeframe for the differents markets)

        self._order_book = {}  # order book per market id and eventually by depth @todo howto

        self._markets = {}  # subscribed markets details
        self._subscriptions = {}  # subsriptions details per sub-id

        self._next_sub_id = 1
        self._recycled_sub_id = []

        self._events = deque()  # message FIFO from WebSocket to be send throught the publisher

    def lock(self, blocking=True, timeout=-1):
        self._mutex.acquire(blocking, timeout)

    def unlock(self):
        self._mutex.release()

    @property
    def name(self):
        return self._name

    def add_subscription(self, market_id, timeframe, depth):
        sub_id = -1

        if self._recycled_sub_id:
            sub_id = self._recycled_sub_id.pop()
        else:
            sub_id = self._next_sub_id
            self._next_sub_id += 1

        sub = Subscription(sub_id, market_id, timeframe, depth)
        self._subscriptions[sub_id] = sub

        return sub

    def del_subscription(self, sub_id):
        if sub_id in self._subscriptions:
            del self._subscriptions[sub_id]
            self._recycled_sub_id.append(sub_id)

    def insert_watched_instrument(self, market_id, timeframes):
        """
        Must be called for each subscribed market to initialize the last price data structure.
        """
        if market_id not in self._watched_instruments:
            self._watched_instruments.add(market_id)

        ltimeframes = set.union(set(self.STORED_TIMEFRAMES), set(timeframes))

        for timeframe in ltimeframes:
            if timeframe == TF_TICK:
                if market_id not in self._last_tick:
                    self._last_tick[market_id] = None
            else:
                if market_id not in self._last_ohlc:
                    self._last_ohlc[market_id] = {}

                if timeframe not in self._last_ohlc[market_id]:
                    self._last_ohlc[market_id][timeframe] = None

    def configured_symbols(self, options):
        """
        Configured instruments symbols from config of the watcher.
        @return A set of symbol, can contains '*' or any symbols prefixed by a '*'.
        """
        configured_markets = options['markets']
        return set([market_id for market_id in configured_markets.keys()])

    def matching_symbols_set(self, configured_symbols, available_symbols):
        """
        Special '*' symbol mean every symbol.
        Starting with '!' mean except this symbol.
        Starting with '*' mean every wildchar before the suffix.

        @param available_symbols List containing any supported markets symbol of the broker. Used when a wildchar is defined.
        """
        return matching_symbols_set(configured_symbols, available_symbols)

    def has_instrument(self, instrument):
        return instrument in self._available_instruments

    def is_subscribed_instrument(self, instrument):
        return instrument in self._watched_instruments

    def available_instruments(self):
        """
        All availables instruments.
        """
        return self._available_instruments

    def watched_instruments(self):
        """
        Watched instruments.
        """
        return self._watched_instruments

    #
    # handler
    #

    def init(self, options):
        if self._trader:
            self._trader.init(options)

    def terminate(self):
        self.unsubscribe_all()       

        if self._trader:
            self._trader.terminate()

    def initial_fetch(self):
        # get the full list of availables markets if possible
        self.fetch_market_list()

        # according to the initially configured list of markets fetch them and subscribes
        for market_id in self._default_instruments:
            # first the market info
            self.fetch_market(market_id)

            # default subsrcibe to tick but could be configurable
            self.subscribe_tick(market_id)

        if self._trader:
            self._trader.initial_fetch()

    def initial_subscriptions(self):
        if self._trader:
            self._trader.initial_subscriptions()

    def sync(self, strategies):
        # reconnect if necessary
        if not self.connected:
            self.connect()

            if not self.connected:
                print("Could not reconnect to %s, retry in 2 seconds" % (self._name,))
                time.sleep(Connector.CONNECTION_RETRY_DELAY)
            else:
                self.initial_fetch()

        if self.connected:
            if self._trader:
                self._trader.process()

    #
    # connection
    #

    def connect(self):
        # to be overrided
        if self.connected:
            return

    def disconnect(self):
        # to be overrided
        pass

    @property
    def connected(self):
        # to be overrided
        return False
    
    @property
    def authenticated(self):
        # to be overrided
        return False

    #
    # subscribers
    #

    def subscribe_tick(self, market_id):
        # to be overrided
        return False

    def subscribe_ohlc(self, market_id, timeframe):
        # to be overrided
        return False

    def subscribe_order_book(self, market_id, depth):
        # to be overrided
        return False

    def unsubscribe(self, sub_id):
        # to be overrided
        pass

    def unsubscribe_all(self):
        # to be overrided
        pass

    def has_market(self, market_id):
        return market_id in self._markets

    #
    # markets
    #

    def list_markets(self):
        return self._markets

    def market(self, market_id):
        return self._markets.get(market_id)

    def fetch_market(self, market_id):
        """
        Initial fetch the market details for a particular market at subscription
        @note to be overrided        
        """
        pass

    def fetch_market_list(self):
        """
        Initial fetch the market list if available
        @note to be overrided        
        """
        pass

    #
    # price (ohlc) history (@see with fetcher feature)
    #

    def fetch_ohlc_history_range(self, market_id, timeframe, from_date, to_date):
        # to be overrided
        return []

    def fetch_ohlc_history_last_n(self, market_id, tf, n):
        # to be overrided
        return []

    #
    # market slots
    #

    def on_update_market(self, market_id, tradable,
            last_update_time, bid, ofr, base_exchange_rate,
            contract_size=None, value_per_pip=None,
            vol24h_base=None, vol24h_quote=None):
        """
        Update bid, ofr, base exchange rate and last update time of a market.
        Take care this method is not thread safe. Use it with the trader mutex or exclusively in the same thread.
        """
        self.lock()

        market = self._markets.get(market_id)
        if market is None:
            # create it but will miss lot of details at this time
            # uses market_id as symbol but its not ideal
            market = Market(market_id, market_id)
            self._markets[market_id] = market

        market.bid = bid
        market.ofr = ofr

        if base_exchange_rate is not None:
            market.base_exchange_rate = base_exchange_rate

        if last_update_time is not None:
            market.last_update_time = last_update_time

        if tradable is not None:
            market.is_open = tradable

        if contract_size is not None:
            market.contract_size = contract_size

        if value_per_pip is not None:
            market.value_per_pip = value_per_pip

        if vol24h_base is not None:
            market.vol24h_base = vol24h_base

        if vol24h_quote is not None:
            market.vol24h_quote = vol24h_quote

        self.unlock()

    #
    # ohlc update/generation
    #

    def __update_ohlc(self, ohlc, bid, ofr, volume):
        # ohlc : timestamp, timeframe, bid open, high, low, close, ofr open, high, low, close, volume, ended.
        if volume:
            ohlc[10] += volume

        if bid:
            if not ohlc[2]:
                ohlc[2] = ohlc[3] = ohlc[4] = ohlc[5] = bid

            # update bid prices
            ohlc[3] = max(ohlc[3], bid)
            ohlc[4] = min(ohlc[4], bid)

            # potential close
            ohlc[5] = bid

        if ofr:
            if not ohlc[6]:
                ohlc[6] = ohlc[7] = ohlc[8] = ohlc[9] = ofr

            # update ofr prices
            ohlc[7] = max(ohlc[7], ofr)
            ohlc[8] = min(ohlc[8], ofr)

            # potential close
            ohlc[9] = ofr

    def update_ohlc(self, market_id, tf, ts, bid, ofr, volume):
        # ohlc : timestamp, timeframe, open, high, low, close, volume, ended.
        base_time = basetime(ts, time.time())

        ended_ohlc = None
        ohlc = None

        # last ohlc per market id (defined at subscription containing per stored timeframes)
        last_ohlc_by_timeframe = self._last_ohlc.get(market_id)
        ohlc = last_ohlc_by_timeframe[tf]

        if ohlc and (ohlc[0] + tf <= base_time):
            # later tick data (laggy ?)
            if ts < base_time:
                # but after that next in laggy data will be ignored...
                # its not perfect in laggy cases for storage, but in live we can't deals we later data
                self.__update_ohlc(ohlc, bid, ofr, volume)

            # need to close the ohlc and to open a new one
            ohlc[11] = 1.0
            ended_ohlc = ohlc

            last_ohlc_by_timeframe[tf] = None
            ohlc = None

        if ohlc is None:
            # open a new one
            ohlc = [base_time, tf, bid, bid, bid, bid, ofr, ofr, ofr, ofr, 0.0, 0.0]
            last_ohlc_by_timeframe[tf] = ohlc

        if ts >= ohlc[0]:
            self.__update_ohlc(ohlc, bid, ofr, volume)

        # stored timeframes only
        if ended_ohlc and (tf in self.STORED_TIMEFRAMES):
            # @todo REDIS cache too
            Database.inst().store_market_ohlc((
                self.name, market_id, int(ended_ohlc.timestamp*1000), tf,
                ended_ohlc.bid_open, ended_ohlc.bid_high, ended_ohlc.bid_low, ended_ohlc.bid_close,
                ended_ohlc.ofr_open, ended_ohlc.ofr_high, ended_ohlc.ofr_low, ended_ohlc.ofr_close,
                ended_ohlc.volume))

        return ended_ohlc

    def update_from_tick(self):
        """
        During update processing, update currently opened candles.
        Then notify a signal if a ohlc is generated (and closed).
        """
        for market_id in self._watched_instruments:
            last_ohlc_by_timeframe = self._last_ohlc.get(market_id)
            if not last_ohlc_by_timeframe:
                continue

            # bid = self._last_tick[market_id].bid
            # ofr = self._last_tick[market_id].ofr

            for tf, _ohlc in last_ohlc_by_timeframe.items():
                # for closing candles, generate them
                ohlc = self.update_ohlc(market_id, tf, time.time(), None, None, None)
                if ohlc:
                    self.lock()
                    self._events.append((self.MSG_OHLC, market_id, ohlc))
                    self.unlock()

    #
    # order request from strategies
    #

    def list_orders(self):
        if self._trader:
            self._trader.list_orders

        return []

    def list_positions(self):
        if self._trader:
            self._trader.list_positions

        return []

    def create_order(self, order):
        if self._trader:
            return self._trader.create_order(order)

        return False

    def cancel_order(self, order_id):
        if self._trader:
            return self._trader.cancel_order(order_id)

        return False

    #
    # position request from strategies
    #

    def close_position(self, position_id):
        if self._trader:
            return self._trader.close_position(position_id)

        return False

    def modify_position(self, position_id, stop_loss_price, take_profit_price):
        if self._trader:
            return self._trader.modify_position(position_id, stop_loss_price, take_profit_price)

        return False
