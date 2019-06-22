# @date 2018-08-07
# @author Frederic SCHERMA
# @license Copyright (c) 2018 Dream Overflow
# Trader base class

import time
import datetime
import copy
import base64
import uuid
import collections
import threading

from market import Market

from .account import Account
from .order import Order
from .position import Position
from .asset import Asset

from common.utils import matching_symbols_set

import logging
logger = logging.getLogger('siis.trader')


class Trader(object):
    """
    Member of a connector, contains & manage all the trading part.
    """

    def __init__(self, connector):
        self._mutex = threading.RLock()  # reentrant locker
        self._connector = connector

        self._account = Account(self._connector)   # account details, margin and balance
        self._assets = {}     # account assets (of non empty balances)
        self._orders = {}     # actives orders
        self._positions = {}  # actives positions
        self._trades = {}     # last executed trades (@todo how many kept ?)

        self._activity = True  # trading activity
        self._timestamp = 0

    def lock(self, blocking=True, timeout=-1):
        self._mutex.acquire(blocking, timeout)

    def unlock(self):
        self._mutex.release()

    @property
    def account(self):
        return self._account

    @property
    def connector(self):
        return self._connector

    @property
    def activity(self):
        """
        Return True if the order must be executed on the broker.
        """
        return self._activity
    
    def set_activity(self, status):
        """
        Enable/disable execution of orders.
        """
        self._activity = status

    def set_timestamp(self, timestamp):
        """
        Used on backtesting by the strategy.
        """
        self._timestamp = timestamp

    @property
    def timestamp(self):
        """
        Current timestamp or backtesting time.
        """
        return time.time()

    def tradeable(self, market_id):
        """
        Return True if the trader accept order and market id is tradeable.
        """
        return False

    @classmethod    
    def authentication_required(cls, fn):
        """
        Annotation for methods that require auth.
        """
        def wrapped(self, *args, **kwargs):
            if not self.authenticated:
                msg = "%s trader : You must be authenticated to use this method" % self._name
                raise Exception(msg)  # errors.AuthenticationError(msg) @todo exceptions classes
            else:
                return fn(self, *args, **kwargs)
    
        return wrapped

    #
    # connector
    #

    def init(self, options):
        self._account.init(options)

    def terminate(self):
        self._account.terminate()

    def initial_fetch(self):
        # account data
        self._account.initial_fetch()

        # asset if supported
        self.fetch_any_assets()

        # positions if supported
        self.fetch_any_positions()

        # actives orders
        self.fetch_any_orders()

        # fetch recents trade but when, how.. ?
        # self.fetch_trades_history(self._last_trade_history)

    def initial_subscriptions(self):
        """
        Defined here initital subscriptions for account, position and trades update data stream.
        """
        pass

    #
    # processing
    #

    def process(self):
        pass

    def has_margin(self, margin):
        """
        Return True for a margin trading if the account have suffisient free margin.
        @note The benefit of this method is it can be overloaded and offers a generic way for a strategy
            to check if an order can be created
        """
        return self.account.margin_balance >= margin

    def has_quantity(self, asset_name, quantity):
        """
        Return True if a given asset has a minimum quantity.
        @note The benefit of this method is it can be overloaded and offers a generic way for a strategy
            to check if an order can be created
        """
        result = False

        self.lock()
        asset = self._assets.get(asset_name)
        result = asset and asset.free >= quantity
        self.unlock()

        return result

    def set_ref_order_id(self, order):
        """
        Generate a new reference order id to be setup before calling create order, else a default one wil be generated.
        Generating it before is a prefered way to correctly manange order in strategy.
        @param order A valid or on to set the ref order id.
        @note If the given order already have a ref order id no change is made.
        """
        if order and not order.ref_order_id:
            # order.set_ref_order_id("siis_" + base64.b64encode(uuid.uuid5(uuid.NAMESPACE_DNS, 'siis.com').bytes).decode('utf8').rstrip('=\n').replace('/', '_').replace('+', '0'))
            order.set_ref_order_id("siis_" + base64.b64encode(uuid.uuid4().bytes).decode('utf8').rstrip('=\n').replace('/', '_').replace('+', '0'))
            return order.ref_order_id

        return None

    #
    # requests
    #

    def create_order(self, order):
        """
        Create an order. The symbol of the order must be on of the trader instruments.
        """
        return False

    def cancel_order(self, order_or_id):
        """
        Cancel an existing order. The symbol of the order must be on of the trader instruments.
        """
        return False

    def close_position(self, position_or_id, market=True, limit_price=None):
        """
        Close an existing position.
        Market is default, take care of the fee.
        Limit price can be defined but then it will create a limit order in the opposite direction in some broker
        and modify the position in some other (depends of the mode hedging...).
        """
        if type(position_or_id) is str:
            position = self._positions.get(position_or_id)
        else:
            position = position_or_id

        if position is None or not position.is_opened:
            return False

        # market stop order
        order = Order(self, position.symbol)
        order.direction = position.close_direction()
        order.order_type = Order.ORDER_MARKET
        order.quantity = position.quantity  # fully close
        order.leverage = position.leverage  # same as open

        # simply create an order in the opposite direction
        return self.create_order(order)

    def modify_position(self, position_id, stop_loss_price=None, take_profit_price=None):
        """
        Modifiy the stop loss or take profit of a position.
        Its a bit complicated, it depends of the mode of the broker, and if hedging or not.
        """
        return False

    #
    # orders
    #

    def list_orders(self):
        """
        List of orders models
        """
        return self._orders

    def fetch_any_orders(self):
        """
        Initial fetch the orders
        @note to be overrided
        """
        pass

    def fetch_order(self, market_id):
        """
        Initial fetch the orders for a particular market_id at subscription
        @note to be overrided
        """
        pass

    #
    # positions
    #

    def list_positions(self, market_id):
        """
        List of positions models
        """
        return self._positions

    def fetch_any_positions(self):
        """
        Initial fetch the orders
        @note to be overrided
        """
        pass

    def fetch_position(self, market_id):
        """
        Initial fetch the positions for a particular market_id at subscription
        @note to be overrided
        """
        pass        

    #
    # trades history
    #

    def list_trades_history(self, market_id):
        """
        List of trade history details
        """
        return self._trades

    def fetch_trades_history(self, market_id, from_date):
        """
        Initial fetch the trade history for a particular market_id at subscription and from a date time
        @param from_date Must be defined
        @note to be overrided
        """
        pass

    #
    # account
    #

    def account(self):
        """
        Account details
        """
        return self._account

    def fetch_account(self):
        """
        Initial fetch the account details
        @note to be overrided
        """
        pass    

    #
    # assets
    #

    def list_assets(self):
        return self._assets

    def asset(self, asset_id):
        return self._assets.get(asset_id)

    def has_asset(self, asset_id):
        """
        Is trader has a specific asset.
        """
        return asset_id in self._assets

    def fetch_any_assets(self):
        """
        Initial fetch the assets balance
        @note to be overrided
        """
        pass

    def fetch_asset(self, asset_id):
        """
        Initial fetch the assets balance for a particular asset
        @note to be overrided
        """
        pass

    #
    # signals
    #

    # def receiver(self, signal):
    #     """
    #     Notifiable listener.
    #     """ 
    #     if signal.source == Signal.SOURCE_SYSTEM:
    #         if signal.signal_type == SIGNAL_WAKE_UP:
    #             # signal of interest
    #             self._signals.append(signal)

    #     elif signal.source == Signal.SOURCE_WATCHER:
    #         if signal.source_name != self._name:
    #             # only interested by the watcher of the same name
    #             return

    #         if signal.signal_type == Signal.SIGNAL_MARKET_DATA:
    #             if not self.has_market(signal.data[0]):
    #                 # non interested by this instrument/symbol
    #                 return

    #         elif signal.signal_type not in (
    #                 Signal.SIGNAL_ACCOUNT_DATA, Signal.SIGNAL_WATCHER_CONNECTED, Signal.SIGNAL_WATCHER_DISCONNECTED,
    #                 Signal.SIGNAL_POSITION_OPENED, Signal.SIGNAL_POSITION_UPDATED, Signal.SIGNAL_POSITION_DELETED, Signal.SIGNAL_POSITION_AMENDED,
    #                 Signal.SIGNAL_ORDER_OPENED, Signal.SIGNAL_ORDER_UPDATED, Signal.SIGNAL_ORDER_DELETED, Signal.SIGNAL_ORDER_REJECTED,
    #                 Signal.SIGNAL_ORDER_CANCELED, Signal.SIGNAL_ORDER_TRADED,
    #                 Signal.SIGNAL_ASSET_UPDATED):
    #             return

    #         # signal of interest
    #         self._signals.append(signal)

    #     elif signal.source == Signal.SOURCE_TRADER:  # in fact it comes from the DB service but request in self trader name
    #         if signal.signal_type not in (Signal.SIGNAL_ASSET_DATA, Signal.SIGNAL_ASSET_DATA_BULK):
    #             # non interested by others signals
    #             return 

    #         # signal of interest
    #         self._signals.append(signal)

    #
    # account slots
    #

    def on_account_updated(self, balance, free_margin, unrealized_pnl, currency, risk_limit):
        """
        Update account details.
        """
        self.lock()

        self._account.set_balance(balance)
        self._account.set_used_margin(balance - free_margin)
        self._account.set_unrealized_profit_loss(unrealized_pnl)

        if currency:
            self._account.set_currency(currency)

        if risk_limit is not None:
            self._account.set_risk_limit(risk_limit)

        self.unlock()

    #
    # positions slots
    #

    def on_position_opened(self, market_id, position_data, ref_order_id):
        # insert it, erase the previous if necessary
        self.lock()

        position = Position(self)
        position.set_position_id(position_data['id'])
        position.set_key(self.service.gen_key())

        position.entry(
            position_data['direction'],
            position_data['symbol'],
            position_data['quantity'],
            position_data.get('take-profit'),
            position_data.get('stop-loss'),
            position_data.get('leverage'),
            position_data.get('trailing-stop'))

        if 'avg-price' in position_data:
            position.entry_price = position_data['avg-price']
        elif 'entry-price' in position_data:
            position.entry_price = position_data['entry-price']
        elif 'exec-price' in position_data:
            position.entry_price = position_data['exec-price']
        # logger.debug("position opened %s size=%s" %(position.symbol, position.quantity))

        self._positions[position.position_id] = position

        market = self._connector.market(position.market_id)
        if market:
            # if not leverage get it from market info
            if not position.leverage:
                position.leverage = 1.0 / market.margin_factor

            position.update_profit_loss(market)

        self.unlock()

    def on_position_updated(self, market_id, position_data, ref_order_id):
        self.lock()

        position = self._positions.get(position_data['id'])

        if position:
            # update
            position.entry(
                position_data['direction'],
                position_data['symbol'],
                position_data['quantity'],
                position_data.get('take-profit'),
                position_data.get('stop-loss'),
                position_data.get('leverage'),
                position_data.get('trailing-stop'))

            if 'avg-price' in position_data:
                position.entry_price = position_data['avg-price']
            elif 'entry-price' in position_data:
                position.entry_price = position_data['entry-price']
            elif 'exec-price' in position_data:
                position.entry_price = position_data['exec-price']
        else:
            # not found, insert and change state 
            position = Position(self)
            position.set_position_id(position_data['id'])
            position.set_key(self.service.gen_key())

            position.entry(
                position_data['direction'],
                position_data['symbol'],
                position_data['quantity'],
                position_data.get('take-profit'),
                position_data.get('stop-loss'),
                position_data.get('leverage'),
                position_data.get('trailing-stop'))

            if 'avg-price' in position_data:
                position.entry_price = position_data['avg-price']
            elif 'entry-price' in position_data:
                position.entry_price = position_data['entry-price']
            elif 'exec-price' in position_data:
                position.entry_price = position_data['exec-price']

            self._positions[position.position_id] = position

        market = self.market(position.symbol)
        if market:
            # if no leverage defined get it from market info
            if not position.leverage:
                position.leverage = 1.0 / market.margin_factor

            position.update_profit_loss(market)

        self.unlock()

    def on_position_amended(self, market_id, position_data, ref_order_id):
        self.lock()

        position = self._positions.get(position_data['id'])

        if position:
            if position_data.get('take-profit'):
                position.take_profit = position_data['take-profit']

            if position_data.get('stop-loss'):
                position.stop_loss = position_data['stop-loss']

            if position_data.get('trailing-stop'):
                position.trailing_stop = position_data['trailing-stop']

            if position_data.get('leverage'):
                position.leverage = position_data['leverage']

        self.unlock()

    def on_position_deleted(self, market_id, position_data, ref_order_id):
        self.lock()

        # delete the position from the dict
        if self._positions.get(position_data['id']):
            del self._positions[position_data['id']]

        self.unlock()

    #
    # order slots
    #

    def on_order_opened(self, market_id, order_data, ref_order_id):
        self.lock()

        market = self._markets.get(order_data['symbol'])
        if market is None:
            # not interested by this market
            self.unlock()
            return

        if order_data['id'] not in self._orders:
            # some are inserted during create_order result
            order = Order(self, order_data['symbol'])
            order.set_order_id(order_data['id'])

            order.created_time = order_data['timestamp']

            order.direction = order_data['direction']
            order.order_type = order_data['type']
            order.time_in_force = order_data['time-in-force']

            order.quantity = order_data['quantity']
            order.order_price = order_data['order-price']
            order.stop_loss = order_data['stop-loss']

            self._orders[order_data['id']] = order

        self.unlock()

    def on_order_updated(self, market_id, order_data, ref_order_id):
        self.lock()

        order = self._orders.get(order_data['id'])
        if order:
            # @todo update price, qty
            pass

        self.unlock()

    def on_order_deleted(self, market_id, order_id, ref_order_id):
        self.lock()

        if order_id in self._orders:
            del self._orders[order_id]

        self.lock()

    def on_order_rejected(self, market_id, ref_order_id):
        pass

    def on_order_canceled(self, market_id, order_id, ref_order_id):
        self.lock()

        if order_id in self._orders:
            del self._orders[order_id]

        self.unlock()

    def on_order_traded(self, market_id, order_data, ref_order_id):
        self.lock()

        order = self._orders.get(order_data['id'])
        if order:
            # update executed qty (filled could be not available from some APIs but having cumulative-filled)
            if order_data.get('filled') is not None:
                order.executed += order_data['filled']
            elif order_data.get('cumulative-filled') is not None:
                order.executed = order_data['cumulative-filled']

        self.unlock()

    #
    # asset slots
    #       

    def on_assets_loaded(self, assets):
        pass

    def on_asset_updated(self, asset_name, locked, free):
        pass
