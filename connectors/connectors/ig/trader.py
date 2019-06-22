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

from .account import IGAccount

from trader.order import Order
from trader.position import Position
from trader.asset import Asset

from trader.trader import Trader
from common.utils import matching_symbols_set

import logging
logger = logging.getLogger('siis.trader')


class IGTrader(Trader):
    """
    IG trader implementation.
    """

    def __init__(self, connector):
        super().__init__(connector)

        self._account = IGAccount(connector)    # account details, margin and balance

    def tradeable(self, market_id):
        """
        Return True if the trader accept order and market id is tradeable.
        """
        return False

    #
    # connector
    #

    def init(self, options):
        self._account.init(options)

    def terminate(self):
        self._account.terminate()

    def initial_subscriptions(self):
        # subscribe for account and trades to have a reactive feedback and don't saturate the REST API
        self._connector._ig_subscribe_account(self._connector.account_id)
        self._connector._ig_subscribe_trades(self._connector.account_id)

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

    def fetch_any_orders(self):
        """
        Initial fetch the orders
        @note to be overrided
        """
        orders = self._connector.ig().fetch_working_orders()

        for order in orders:
            pass  # @todo

    def fetch_order(self, market_id):
        """
        Initial fetch the orders for a particular market_id at subscription
        @note to be overrided
        """
        pass

    #
    # positions
    #

    def fetch_any_positions(self):
        """
        Initial fetch the orders
        @note to be overrided
        """
        positions = self._connector.ig().fetch_open_positions()

        for position in positions:
            pass  # @todo

    def fetch_position(self, market_id):
        """
        Initial fetch the positions for a particular market_id at subscription
        @note to be overrided
        """
        pass        

    #
    # trades history
    #

    def fetch_trades_history(self, market_id, from_date):
        """
        Initial fetch the trade history for a particular market_id at subscription and from a date time
        @param from_date Must be defined
        @note to be overrided
        """
        transactions = self._connector.ig().fetch_transaction_history(
                trans_type='ALL_DEAL', from_date=from_date, to_date=datetime.datetime.now())

        for transaction in transactions:
            pass  # @todo

    #
    # account
    #

    def fetch_account(self):
        """
        Initial fetch the account details
        @note to be overrided
        """
        self._connector.ig().fetch_account(self.account_id)

    #
    # assets
    #

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
    # processing
    #

    def process(self):
        pass
