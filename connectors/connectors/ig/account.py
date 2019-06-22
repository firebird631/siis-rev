# @date 2018-08-25
# @author Frederic SCHERMA
# @license Copyright (c) 2018 Dream Overflow
# Account/user model

import json
import time
import datetime

from trader.account import Account


class IGAccount(Account):

    def __init__(self, connector):
        super().__init__(connector)

        self._account_type = Account.TYPE_MARGIN

        self._currency = 'EUR'
        self._currency_ratio = 1.16  # from USD

        self._last_update = 0.0
        
        self._market_info = None
        self._inv_market_info = None

    def init(self, options):
        pass

    def terminate(self):
        # cleanup
        self._market_info = None
        self._inv_market_info = None

    def initial_fetch(self):
        # exchange rate from account currency to USD if possible
        if self._currency != "USD":
            # fetch a market to known exchange rate between account currency and USD if possible else assume 0.0 (no conversion possible)
            currency_market = "CS.D.%s%s.MINI.IP" % (self._currency, "USD")
            if not self._connector.has_market(currency_market):
                self._connector.fetch_market(currency_market)

            if not self._connector.has_market(currency_market):
                inv_currency_market = "CS.D.%s%s.MINI.IP" % ("USD", self._currency)
                self._connector.fetch_market(inv_currency_market)

                # invert market rate
                self._inv_market_info = self._connector.market(rev_currency_market)
                self._currency_ratio = 1.0 / self._inv_market_info.base_exchange_rate if self._inv_market_info else 0.0
            else:
                # direct market rate
                self._market_info = self._connector.market("CS.D.%s%s.MINI.IP" % (self._currency, "USD"))
                self._currency_ratio = self._market_info.base_exchange_rate if self._market_info else 1.0
        else:
            # account is already in USD
            self._currency = 1.0

        self.update()

    def update(self):
        if self._connector is None or not self._connector.connected:
            return

        # initial update and then one per min, the live updated are done by signal and WS
        now = time.time()
        if now - self._last_update >= 1.0*60.0:
            self._last_update = now

            account = self._connector.ig().fetch_account(self._connector.account_id)
            self._currency = account.get('currency')

            # exchange rate from account currency to USD if possible
            if self._currency != "USD":
                # fetch a market to known exchange rate between account currency and USD if possible else assume 0.0 (no conversion possible)
                if self._market_info:
                    self._currency_ratio = self._market_info.base_exchange_rate
                elif self._inv_market_info:
                    self._currency_ratio = 1.0 / self._inv_market_info.base_exchange_rate
                else:
                    self._currency_ratio = 0.0
            else:
                # account is already in USD
                self._currency = 1.0

            self._username = self._email = self._connector.username
            self._name = account.get('accountName')
            self._account_type = account.get('accountType')  # CFD, PHYSICAL, SPREADBET
            self._account_name = account.get('accountName')

            balance = account.get('balance')
            if balance:
                self._balance = balance.get('balance')
                self._net_worth = balance.get('available')
                self._profit_loss = balance.get('profitLoss')

                self._margin_balance = self._net_worth

                # cannot be computed because leverage depend of the instrument
                self._risk_limit = balance.get('available')
