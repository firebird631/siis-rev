# @date 2019-03-04
# @author Frederic SCHERMA
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# connector for IG communications

import requests
import urllib
import json
import datetime
import base64
import math
import traceback

from constants import *
from connector import Connector

from market import Market
from database.database import Database

from .rest import IGService
from .lightstreamer import LSClient, Subscription as LSSubscription
from .trader import IGTrader

from trader.order import Order

import logging
logger = logging.getLogger('siis.connector.ig')


class IGConnector(Connector):
    """
    Connector for IG communications.
    @todo could add a REST request limit per minute

    @ref https://labs.ig.com/rest-trading-api-reference
    @ref https://labs.ig.com/streaming-api-reference

    @todo to complete and trader
    """

    # siis to IG supported timeframes
    TF_MAP = {
        TF_SEC: 'SECOND',
        TF_MIN: 'MINUTE',
        TF_2MIN: 'MINUTE_2',
        TF_3MIN: 'MINUTE_3',
        TF_5MIN: 'MINUTE_5',
        TF_10MIN: 'MINUTE_10',
        TF_15MIN: 'MINUTE_15',
        TF_30MIN: 'MINUTE_30',
        TF_HOUR: 'HOUR',
        TF_2HOUR: 'HOUR_2',
        TF_3HOUR: 'HOUR_3',
        TF_4HOUR: 'HOUR_4',
        TF_DAY: 'DAY',
        TF_WEEK: 'WEEK',
        TF_MONTH: 'MONTH'
    }

    def __init__(self, server, options):
        Connector.__init__(self, "ig.com", server, options)

        self._host = "ig.com"
        self._base_url = "/api/v2/"
        self._timeout = 7
        self._connected = False
        self._server = server

        # config
        self._host = options['connector'].get('host', "ig.com")
        self._account_id = options['connector'].get('accountId', "")
        self.__username = options['connector'].get('username', "")
        self.__password = options['connector'].get('password', "")
        self.__api_key = options['connector'].get('apiKey', "")

        self._session = None
        self._ig_service = None
        self._client_id = None
        self._lightstreamer = None

        self._account_type = "LIVE" if self._host == "api.ig.com" else "DEMO"

        self._cached_tick = {}  # last cached tick when none value
        self._ig_tick_subscriptions = {}   # tick subscriptions id per market id

        # trader capacity, instanciate it
        self._trader = IGTrader(self)

    #
    # overrides
    #

    def connect(self):
        if self.connected:
            return

        # REST session and authentication
        self._session = requests.Session()

        self._ig_service = IGService(
            self.__username,
            self.__password,
            self.__api_key,
            self._account_type,
            self._session)

        try:
            res = self._ig_service.create_session()
            self._client_id = res.get('clientId')  # keep clientId for lightstream service
        except Exception as e:
            logger.error(repr(e))
            self._session = None
            self._ig_service = None

        # streaming
        if self._ig_service:
            # from CST and XST
            password = "CST-%s|XST-%s" % (self._ig_service.cst, self._ig_service.xst)

            if self._lightstreamer:
                # destroy previous connection
                self._lightstreamer.destroy()

            self._lightstreamer = LSClient(
                self._ig_service.lightstreamer_endpoint,  # "https://push.lightstreamer.com",
                adapter_set="DEFAULT",
                user=self._client_id,
                password=password)

            self._lightstreamer.connect()

    def disconnect(self):
        if self._lightstreamer:
            self._lightstreamer.disconnect()
            self._lightstreamer = None

        self._ig_service = None
        self._session = None

    @property
    def connected(self):
        return self._session is not None and self._ig_service is not None and self._ig_service.connected

    @property
    def authenticated(self):
        # to be overrided
        return self._session is not None and self._client_id

    @property
    def username(self):
        return self.__username

    #
    # subscribers
    #

    def subscribe_tick(self, market_id):
        """
        Subscribe to an instrument tick updates.
        @note Threadsafe method.
        """
        self.lock()

        self.insert_watched_instrument(market_id, [0])

        if market_id not in self._ig_tick_subscriptions:
            ig_sub_key = self._ig_subscribe_tick(market_id)
        else:
            ig_sub_key = self._ig_tick_subscriptions[market_id]

        sub = self.add_subscription(market_id, 0, -1)
        # sub.count += 1  # ref counter
        # @todo on doit check si on garde ou non un tick sur IG
        # mais on doit aussi check si on garde ou non un ohlc local (timeframe)
        # aussi pour tick si un ohlc l'utilise on doit le garder
        # et on doit garder un tick et son ohlc si il est dans les default wachted

        self.unlock()

    def subscribe_ohlc(self, instrument, timeframe):
        """
        Subscribe to an instrument. Timeframe must be greater than 0.
        """
        pass  # @todo
    #     self.insert_watched_instrument(market_id, [timeframe])

    #     fields = [
    #         "BID_OPEN", "OFR_OPEN",
    #         "BID_CLOSE", "OFR_CLOSE",
    #         "BID_HIGH", "OFR_HIGH",
    #         "BID_LOW", "OFR_LOW",
    #         "LTP", "LTV", "TTV", "UTM",
    #         "CONS_END"
    #     ]

    #     if timeframe == TF_SEC:
    #         tf = "SECOND"
    #     elif timeframe == TF_MIN:
    #         tf = "1MINUTE"
    #     elif timeframe == TF_5MIN:
    #         tf = "5MINUTE"
    #     elif timeframe == TF_HOUR:
    #         tf = "HOUR"
    #     else:
    #         return

    #     subscription = LSSubscription(
    #         mode="MERGE",
    #         items=["CHART:"+instrument+":"+tf],
    #         fields=fields,
    #         adapter="")

    #     sub_key = self._lightstreamer.subscribe(subscription)
    #     @todo store the sub_key : market+timeframe

    #     subscription.addlistener(self, IGConnector._on_ohlc_update)

    def unsubscribe(self, sub_id):
        """
        @note Threadsafe method.
        """
        self.lock()

        sub = self._subscriptions.get(sub_id)
        if sub:
            if sub.count <= 1:
                self.del_subscription(sub_id)


        if market_id in self._ig_tick_subscriptions:
            # only of not a always watched market
            if market_id not in self._default_instruments:
                self._lightstreamer.unsubscribe(self._ig_tick_subscriptions[market_id])
                del self._ig_tick_subscriptions[market_id]


            self.lock()
            market = self._markets.get(market_id)
            if market:
                 market.timeframes

    def unsubscribe_all(self):
        """
        All excepted configured markets that we always want to watch.
        """
        # @todo
        pass

    #
    # markets
    #

    def fetch_market(self, market_id):
        """
        Fetch and cache it.
        @param market_id Is the IG epic name
        """
        market_info = self._ig_service.fetch_market_by_epic(market_id)
        if not market_info:
            return

        instrument = market_info['instrument']
        snapshot = market_info['snapshot']
        dealing_rules = market_info['dealingRules']

        market = Market(market_id, instrument['marketId'])

        # cannot interpret this value because IG want it as it is
        market.expiry = instrument['expiry']

        # not perfect but IG does not provides this information
        if instrument["marketId"].endswith(instrument["currencies"][0]["name"]):
            base_symbol = instrument["marketId"][:-len(instrument["currencies"][0]["name"])]
        else:
            base_symbol = instrument["marketId"]

        market.base_exchange_rate = instrument['currencies'][0]['baseExchangeRate']   # "exchangeRate": 0.77

        market.one_pip_means = float(instrument['onePipMeans'].split(' ')[0])
        market.value_per_pip = float(instrument['valueOfOnePip'])
        market.contract_size = float(instrument['contractSize'])
        market.lot_size = float(instrument['lotSize'])

        # @todo how to determine base precision ?
        market.set_base(base_symbol, base_symbol)
        market.set_quote(
            instrument["currencies"][0]["name"],
            instrument["currencies"][0]['symbol'],
            -int(math.log10(market.one_pip_means)))  # "USD", "$", precision

        if snapshot:
            market.is_open = snapshot["marketStatus"] == "TRADEABLE"
            market.bid = snapshot['bid']
            market.ofr = snapshot['offer']

        # "marginFactorUnit": "PERCENTAGE" not avalaible if market is down
        if instrument.get('marginFactor') and market.is_open:
            market.margin_factor = float(instrument['marginFactor'])
            margin_factor = instrument['marginFactor']
        elif instrument.get('margin') and market.is_open:
            market.margin_factor = 0.1 / float(instrument['margin'])
            margin_factor = str(market.margin_factor)
        else:
            # we don't want this when market is down because it could overwrite the previous stored value
            margin_factor = None

        if instrument['unit'] == 'AMOUNT':
            market.unit_type = Market.UNIT_AMOUNT
        elif instrument['unit'] == 'CONTRACTS':
            market.unit_type = Market.UNIT_CONTRACTS
        elif instrument['unit'] == 'SHARES':
            market.unit_type = Market.UNIT_SHARES

        # BINARY OPT_* BUNGEE_* 
        if instrument['type'] == 'CURRENCIES':
            market._market_type = Market.TYPE_CURRENCY
        elif instrument['type'] == 'INDICES':
            market._market_type = Market.TYPE_INDICE
        elif instrument['type'] == 'COMMODITIES':
            market._market_type = Market.TYPE_COMMODITY
        elif instrument['type'] == 'SHARES':
            market._market_type = Market.TYPE_STOCK
        elif instrument['type'] == 'RATES':
            market._market_type = Market.TYPE_RATE
        elif instrument['type'] == 'SECTORS':
            market._market_type = Market.TYPE_SECTOR

        market.trade = Market.TRADE_MARGIN
        market.contract_type = Market.CONTRACT_CFD

        # take minDealSize as tick size
        market.set_size_limits(dealing_rules["minDealSize"]["value"], 0.0, dealing_rules["minDealSize"]["value"])
        # @todo there is some limits in contract size
        market.set_notional_limits(0.0, 0.0, 0.0)
        # @todo maybe decimal_place of onePipMeans for tick_size
        market.set_price_limits(0.0, 0.0, 0.0)

        # commission for stocks
        commission = "0.0"
        # @todo

        # store it
        self.lock()
        self._markets[market_id] = market
        self.unlock()

        # store market info
        Database.inst().store_market_info((self.name, market_id, market.symbol,
            market.market_type, market.unit_type, market.contract_type,  # type
            market.trade, market.orders,  # type
            market.base, market.base_display, market.base_precision,  # base
            market.quote, market.quote_display, market.quote_precision,  # quote
            market.expiry, int(market.last_update_time * 1000.0),  # expiry, timestamp
            instrument['lotSize'], instrument['contractSize'], str(market.base_exchange_rate),
            instrument['valueOfOnePip'], instrument['onePipMeans'].split(' ')[0], margin_factor,
            dealing_rules["minDealSize"]["value"], "0.0", dealing_rules["minDealSize"]["value"],  # size limits
            "0.0", "0.0", "0.0",  # notional limits
            "0.0", "0.0", "0.0",  # price limits
            "0.0", "0.0", commission, commission)  # fees
        )

    def fetch_market_list(self):
        # not possible, but can fetch by a watch-list
        pass

    #
    # price (ohlc) history (@see with fetcher feature)
    #

    def fetch_ohlc_history_range(self, market_id, timeframe, from_date, to_date):
        # @todo
        ## history = self._ig_service.fetch_historical_prices_by_epic(market_id, IGConnector.TF_MAP[tf], from_date, to_date)  # V3 format
        # history = self._ig_service.fetch_historical_prices_by_epic_and_date_range(market_id, IGConnector.TF_MAP[tf], from_date, to_date)
        # history
        # @see fetcher to format history data, store, cascaded...
        return []

    def fetch_ohlc_history_last_n(self, market_id, tf, n):
        # @todo
        # history = self._ig_service.fetch_historical_prices_by_epic_and_num_points(market_id, IGConnector.TF_MAP[tf], n)
        # return history        
        # @see fetcher to format history data, store, cascaded...
        return []

    #
    # internal
    #

    def ig(self):
        return self._ig_service 

    @property
    def account_id(self):
        return self._account_id

    def _ig_subscribe_account(self, account_id):
        fields = ["PNL", "AVAILABLE_TO_DEAL", "MARGIN", "FUNDS", "AVAILABLE_CASH"]

        subscription = LSSubscription(
                mode="MERGE",
                items=["ACCOUNT:"+account_id],
                fields=fields,
                adapter="")

        sub_key = self._lightstreamer.subscribe(subscription)
        subscription.addlistener(self, IGConnector._on_account_update)

    def _ig_subscribe_trades(self, account_id):
        fields = ["CONFIRMS", "WOU", "OPU"]

        subscription = LSSubscription(
                mode="DISTINCT",
                items=["TRADE:"+account_id],
                fields=fields,
                adapter="")

        sub_key = self._lightstreamer.subscribe(subscription)
        subscription.addlistener(self, IGConnector._on_trade_update)

    def _ig_subscribe_tick(self, instrument):
        # need to subscribe to lightstream tick for this market
        fields = ["BID", "OFR", "LTP", "LTV", "TTV", "UTM"]

        subscription = LSSubscription(
            mode="DISTINCT",
            items=["CHART:"+instrument+":TICK"],
            fields=fields,
            adapter="")

        sub_key = self._lightstreamer.subscribe(subscription)
        subscription.addlistener(self, IGConnector._on_tick_update)

        self._ig_tick_subscriptions[instrument] = sub_key

        return sub_key

    @staticmethod
    def _on_account_update(self, item_update):
        name = item_update.get('name', '').split(':')

        try:
            if len(name) == 2 and name[0] == 'ACCOUNT' and name[1] == self._account_id:
                # live account updates
                values = item_update['values']

                account_data = (float(values['FUNDS']), float(values['AVAILABLE_TO_DEAL']), float(values['PNL']), None, None)
                # self.service.notify(Signal.SIGNAL_ACCOUNT_DATA, self.name, account_data)
        except Exception as e:
            logger.error(repr(e))
            logger.error(traceback.format_exc())                

    @staticmethod
    def _on_market_update(self, item_update):
        name = item_update.get('name', '').split(':')

        try:
            if len(name) == 2 and name[0] == 'MARKET':
                # market data instrument by epic
                values = item_update['values']
                epic = name[1]

                ready = values['MARKET_STATE'] == 'TRADEABLE'

                # date of the event 20:36:01 without Z
                if ready:
                    update_time = datetime.datetime.strptime(values['UPDATE_TIME'], '%H:%M:%S').timestamp()
                    market_data = (name[1], True, update_time, float(values["BID"]), float(values["OFFER"]), None, None, None, None, None)
                else:
                    update_time = 0
                    market_data = (name[1], False, 0, 0.0, 0.0, None, None, None, None, None)

                # self.service.notify(Signal.SIGNAL_MARKET_DATA, self.name, market_data)
        except Exception as e:
            logger.error(repr(e))
            logger.error(traceback.format_exc())                

    @staticmethod
    def _on_tick_update(self, item_update):
        name = item_update.get('name', '').split(':')

        try:
            if len(name) == 3 and name[0] == 'CHART' and name[2] == 'TICK':
                values = item_update['values']
                market_id = name[1]

                bid = None
                ofr = None
                utm = None
                ltv = None

                if values['UTM']:
                    utm = values['UTM']
                elif market_id in self._cached_tick:
                    utm = self._cached_tick[market_id][0]

                if values['BID']:
                    bid = values['BID']
                elif market_id in self._cached_tick:
                    bid = self._cached_tick[market_id][1]

                if values['OFR']:
                    ofr = values['OFR']
                elif market_id in self._cached_tick:
                    ofr = self._cached_tick[market_id][2]

                if values['LTV']:
                    ltv = values['LTV']
                elif market_id in self._cached_tick:
                    ltv = self._cached_tick[market_id][3]

                if utm is None or bid is None or ofr is None:
                    # need all informations, wait the next one
                    return

                # cache for when a value is not defined
                self._cached_tick[market_id] = (utm, bid, ofr, ltv)

                tick = (float(utm) * 0.001, float(bid), float(ofr), float(ltv or "0"))

                self.lock()
                self._last_tick[market_id] = tick
                self._events.append(self.MSG_TICK)
                self.unlock()

                for tf, _ohlc in self._last_ohlc[market_id].items():
                    # generate candle per each tf
                    self.lock()
                    candle = self.update_ohlc(market_id, tf, tick[0], tick[1], tick[2], tick[3])
                    self.unlock()

                    if candle is not None:
                        pass  # self.service.notify(Signal.SIGNAL_CANDLE_DATA, self.name, (market_id, candle)) @todo

                # tick storage
                Database.inst().store_market_trade((self.name, market_id, int(utm), bid, ofr, ltv or "0"))

        except Exception as e:
            logger.error(repr(e))
            logger.error(traceback.format_exc())                    

    # @staticmethod
    # def _on_ohlc_update(self, item_update):
    #     name = item_update.get('name', '').split(':')

    #     try:
    #         if len(name) == 3 and name[0] == 'CHART':
    #             values = item_update['values']
    #             if values['CONS_END'] == '0':
    #                 # get only consolidated candles
    #                 # @warning It is rarely defined, so many close could be missing, prefers using tick to rebuild ohlc locally
    #                 return

    #             # timeframe
    #             if name[2] == 'SECOND':
    #                 tf = TF_SEC
    #             elif name[2] == '1MINUTE':
    #                 tf = TF_MIN
    #             elif name[2] == '5MINUTE':
    #                 tf = TF_5MIN
    #             elif name[2] == 'HOUR':
    #                 tf = TF_HOUR
 
    #             # one of the value could be missing, use the previous from the cache if we have it
    #             if (values['UTM'] is None or values['LTV'] is None or
    #                 values['OFR_OPEN'] is None or values['OFR_HIGH'] is None or values['OFR_LOW'] is None or values['OFR_CLOSE'] is None or 
    #                     values['BID_OPEN'] is None or values['BID_HIGH'] is None or values['BID_LOW'] is None or values['BID_CLOSE'] is None):

    #                 if name[1] not in self._cached_ohlc or tf not in self._cached_ohlc[name[1]]:
    #                     logger.warning("no value and cache miss for %s ohlc in %s (%s)" % (name[1], tf, values))

    #                 if values['UTM'] is None:
    #                     utm = self._cached_ohlc[name[1]][tf][0]
    #                 if values['LTV'] is None:
    #                     ltv = self._cached_ohlc[name[1]][tf][9]

    #             utm = values['UTM']
    #             ltv = values['LTV']

    #             candle = Candle(float(utm) * 0.001, tf)

    #             # if incomplete candle replace ofr by bid or bid by ofr @todo but must be value from previous candle
    #             # but and if we don't have to previous... ok for 1 min but for 1h ? ...
    #             bid_open = values['BID_OPEN'] or values['OFR_OPEN']
    #             bid_high = values['BID_HIGH'] or values['OFR_HIGH']
    #             bid_low = values['BID_LOW'] or values['OFR_LOW']
    #             bid_close = values['BID_CLOSE'] or values['OFR_CLOSE']

    #             ofr_open = values['OFR_OPEN'] or values['BID_OPEN']
    #             ofr_high = values['OFR_HIGH'] or values['BID_HIGH']
    #             ofr_low = values['OFR_LOW'] or values['BID_LOW']
    #             ofr_close = values['OFR_CLOSE'] or values['BID_CLOSE']

    #             candle.set_bid_ohlc(float(bid_open), float(bid_high), float(bid_low), float(bid_close))
    #             candle.set_ofr_ohlc(float(ofr_open), float(ofr_high), float(ofr_low), float(ofr_close))
    #             candle.set_volume(float(values['LTV']) if values['LTV'] else 0.0)
    #             candle.set_consolidated(values['CONS_END'] == '1')

    #             self.service.notify(Signal.SIGNAL_CANDLE_DATA, self.name, (name[1], candle))

    #             if values['CONS_END'] == '1' and not self._read_only:
    #                 # write only consolidated candles. values are string its perfect if not last traded volume then 0
    #                 Database.inst().store_market_ohlc((
    #                     self.name, name[1], int(utm), tf,
    #                     bid_open, bid_high, bid_low, bid_close,
    #                     ofr_open, ofr_high, ofr_low, ofr_close,
    #                     values['LTV'] or "0"))

    #             # cache for when a value is not defined
    #             self._cached_ohlc[name[1]][tf] = (utm, bid_open, bid_high, bid_low, bid_close, ofr_open, ofr_high, ofr_low, ofr_close, ltv)

    #     except Exception as e:
    #         logger.error(repr(e))
    #         logger.error(traceback.format_exc())             

    @staticmethod
    def _on_trade_update(self, item_update):
        name = item_update.get('name', '').split(':')

        try:
            if len(name) == 2 and name[0] == 'TRADE' and name[1] == self._account_id:
                # live trade updates
                values = item_update['values']

                #
                # order confirms (accepted/rejected)
                #

                if values.get('CONFIRMS'):
                    # not use them because we only want CRUD operations => OPU only so
                    data = json.loads(values.get('CONFIRMS'))
                    epic = data.get('epic')

                    level = float(data['level']) if data.get('level') is not None else None
                    quantity = float(data['size']) if data.get('size') is not None else None

                    if data['dealStatus'] == 'REJECTED':
                        ref_order_id = data['dealReference']

                        # if data['reason'] == 'INSUFFICIENT_BALANCE':
                        #   reason = 'insufficient balance'

                        # @todo self.service.notify(Signal.SIGNAL_ORDER_REJECTED, self.name, (epic, ref_order_id))

                    elif data['dealStatus'] == 'ACCEPTED':
                        # deal confirmed and accepted
                        order_id = data['dealId']
                        ref_order_id = data['dealReference']

                        logger.warning("ig 538 'CONFIRMS' %s" % str(data))

                        # date 2018-09-13T20:36:01.096 without Z
                        event_time = datetime.datetime.strptime(data['date'], '%Y-%m-%dT%H:%M:%S.%f').timestamp()

                        if data['direction'] == 'BUY':
                            direction = LONG
                        elif data['direction'] == 'SELL':
                            direction = SHORT
                        else:
                            direction = LONG

                        quantity = float(data.get('size')) if data.get('size') is not None else 0.0

                        # don't send event because create_order return True in that case

                        if quantity and level:
                            # signal of updated order
                            order_data = {
                                'id': order_id,
                                # @todo important we want TRADED and UPDATED distinct
                            }

                            # @todo to be completed before signal, but not really necessary we can work with position update
                            # self.service.notify(Signal.SIGNAL_ORDER_TRADED, self.name, (epic, order_data, ref_order_id))

                #
                # active waiting order (open/updated/deleted)
                #

                if values.get('WOU'):
                    data = json.loads(values.get('WOU'))

                    order_id = data['dealId']
                    ref_order_id = data['dealReference']

                    epic = data['epic']
                    logger.warning("ig l608 'WOU' %s" % str(data))

                    # level = float(data['level']) if data.get('level') is not None else None
                    # stop_level = float(data['stopLevel']) if data.get('stopLevel') is not None else None
                    # limit_level = float(data['limitLevel']) if data.get('limitLevel') is not None else None
                    # stop_distance = float(data['stopDistance']) if data.get('stopDistance') is not None else None
                    # limit_distance = float(data['limitDistance']) if data.get('limitDistance') is not None else None
                    # profit_loss = float(data['profit']) if data.get('profit') is not None else 0.0
                    # epic, level, guaranteedStop, currency, timeInForce (GOOD_TILL_CANCELLED, GOOD_TILL_DATE)
                    # goodTillDate, size, timestamp, stopDistance, limitDistance

                    # if data['orderType'] == "LIMIT":
                    #   pass
                    # elif data['orderType'] == "STOP":
                    #   pass

                    # status OPEN, DELETED, FULLY_CLOSED
                    if data['status'] == "OPEN":
                        order_data = {
                            'id': order_id,
                            # @todo important
                        }

                        # @todo self.service.notify(Signal.SIGNAL_ORDER_OPENED, self.name, (epic, order_data, ref_order_id))

                    elif data['status'] == "UPDATED":
                        # signal of updated order
                        order_data = {
                            'id': order_id,
                            # @todo important we want TRADED and UPDATED distinct
                        }

                        # @todo self.service.notify(Signal.SIGNAL_ORDER_UPDATED, self.name, (epic, order_data, ref_order_id))

                    elif data['status'] == "DELETED":
                        # signal of deleted order
                        # @todo self.service.notify(Signal.SIGNAL_ORDER_DELETED, self.name, (epic, order_id, ref_order_id))
                        pass

                    elif data['status'] == "FULLY_CLOSED":
                        # @todo ??
                        pass

                #
                # active position (open/updated/deleted)
                #

                if values.get('OPU'):
                    data = json.loads(values.get('OPU'))

                    if data.get('direction', '') == 'BUY':
                        direction = LONG
                    elif data.get('direction', '') == 'SELL':
                        direction = SHORT
                    else:
                        direction = LONG

                    position_id = data['dealId']
                    ref_order_id = data['dealReference']

                    logger.warning("ig 619 'OPU' %s" % str(data))

                    epic = data.get('epic')
                    quantity = float(data.get('size')) if data.get('size') is not None else 0.0

                    level = float(data['level']) if data.get('level') is not None else None
                    stop_level = float(data['stopLevel']) if data.get('stopLevel') is not None else None
                    limit_level = float(data['limitLevel']) if data.get('limitLevel') is not None else None
                    stop_distance = float(data['stopDistance']) if data.get('stopDistance') is not None else None
                    limit_distance = float(data['limitDistance']) if data.get('limitDistance') is not None else None                        
                    profit_loss = float(data['profit']) if data.get('profit') is not None else 0.0

                    # date of the event 2018-09-13T20:36:01.096 without Z
                    event_time = datetime.datetime.strptime(data['timestamp'], '%Y-%m-%dT%H:%M:%S.%f').timestamp()

                    # status OPEN, UPDATED, DELETED
                    if data['status'] == "OPEN":
                        # signal of opened position
                        position_data = {
                            'id': position_id,
                            'symbol': epic,
                            'direction': direction,
                            'timestamp': event_time,
                            'quantity': quantity,
                            'exec-price': level,
                            'stop-loss': stop_distance,
                            'take-profit': limit_level,
                            'profit-loss': profit_loss,
                            'cumulative-filled': quantity,
                            'filled': None,  # no have
                            'liquidation-price': None  # no have
                        }

                        # @todo self.service.notify(Signal.SIGNAL_POSITION_OPENED, self.name, (epic, position_data, ref_order_id))

                    elif data['status'] == "UPDATED":
                        # signal of updated position
                        position_data = {
                            'id': position_id,
                            'symbol': epic,
                            'direction': direction,
                            'timestamp': event_time,
                            'quantity': quantity,
                            'exec-price': level,
                            'stop-loss': stop_distance,
                            'take-profit': limit_level,
                            # 'profit-currency': '', 'profitCurrency'
                            'profit-loss': profit_loss,
                            # @todo trailingStep, trailingStopDistance, guaranteedStop
                            'cumulative-filled': quantity,
                            'filled': None,  # no have
                            'liquidation-price': None  # no have
                        }

                        # @todo self.service.notify(Signal.SIGNAL_POSITION_UPDATED, self.name, (epic, position_data, ref_order_id))

                    elif data['status'] == "DELETED":
                        # signal of updated position
                        position_data = {
                            'id': position_id,
                            'symbol': epic,
                            'direction': direction,
                            'timestamp': event_time,
                            'quantity': quantity,
                            'exec-price': level,
                            'stop-loss': stop_distance,
                            'take-profit': limit_level,
                            # 'profit-currency': '', 'profitCurrency'
                            'profit-loss': profit_loss,
                            # @todo trailingStep, trailingStopDistance, guaranteedStop
                            'cumulative-filled': quantity,
                            'filled': None,  # no have
                            'liquidation-price': None  # no have
                        }

                        # @todo self.service.notify(Signal.SIGNAL_POSITION_DELETED, self.name, (epic, position_data, ref_order_id))
                    else:
                        logger.warning("ig l703 'OPU' %s" % str(data))

        except Exception as e:
            logger.error(repr(e))
            logger.error(traceback.format_exc())
