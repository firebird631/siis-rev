# @date 2018-08-07
# @author Frederic SCHERMA
# @license Copyright (c) 2018 Dream Overflow
# Storage service, postgresql implementation

import os
import json
import time
import datetime
import threading
import copy
import traceback
import pathlib

from market import Market
from trader.asset import Asset

from database.tickdb import TickStorage, TickStreamer
from database.ohlcdb import OhlcStorage, OhlcStreamer

from database.database import Database, DatabaseException

import logging
logger = logging.getLogger('siis.database.pgsql')
error_logger = logging.getLogger('siis.error.database.pgsql')


class PgSql(Database):
    """
    Storage service, postgresql implementation.
    ://www.postgresqltutorial.com/postgresql-python/connect/

    PosgreSQL DB creation :

    CREATE DATABASE siis;
    CREATE USER siis WITH ENCRYPTED PASSWORD 'siis';
    GRANT ALL PRIVILEGES ON DATABASE siis TO siis;

    @todo for ohlc storage use a bulk insert !
    """
    def __init__(self):
        super().__init__()
        self._db = None
        self._conn_str = ""
        self.psycopg2 = None

        try:
            self.psycopg2 = import_module('psycopg2', package='')
        except ModuleNotFoundError as e:
            logger.error(repr(e))

    def connect(self, config):
        if self.psycopg2:
            if config['siis'].get('host'):
                # hostname provided
                self._conn_str = "dbname=%s user=%s password=%s host=%s port=%i" % (
                    config.get('name', "siis"),
                    config.get('user', "siis"),
                    config.get('password', "siis"),
                    config.get('host', "localhost"),
                    config.get('port', 5432))
            else:
                # local unix socket
                self._conn_str = "dbname=%s user=%s password=%s" % (
                    config.get('name', "siis"),
                    config.get('user', "siis"),
                    config.get('password', "siis"))

            self._db = self.psycopg2.connect(self._conn_str)

        if not self._db:
            raise DatabaseException("Unable to connect to postgresql database ! Verify you have psycopg2 installed and your user database.json file.")

    def disconnect(self):
        # postresql db
        if self._db:
            self._db.close()
            self._db = None
            self._conn_str = ""

    def setup_market_sql(self):
        cursor = self._db.cursor()

        # market table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS market(
                id SERIAL PRIMARY KEY,
                broker_id VARCHAR(255) NOT NULL, market_id VARCHAR(255) NOT NULL, symbol VARCHAR(32) NOT NULL,
                market_type INTEGER NOT NULL DEFAULT 0, unit_type INTEGER NOT NULL DEFAULT 0, contract_type INTEGER NOT NULL DEFAULT 0,
                trade_type INTEGER NOT NULL DEFAULT 0, orders INTEGER NOT NULL DEFAULT 0,
                base VARCHAR(32) NOT NULL, base_display VARCHAR(32) NOT NULL, base_precision VARCHAR(32) NOT NULL,
                quote VARCHAR(32) NOT NULL, quote_display VARCHAR(32) NOT NULL, quote_precision VARCHAR(32) NOT NULL,
                expiry VARCHAR(32) NOT NULL, timestamp BIGINT NOT NULL,
                lot_size VARCHAR(32) NOT NULL, contract_size VARCHAR(32) NOT NULL, base_exchange_rate VARCHAR(32) NOT NULL,
                value_per_pip VARCHAR(32) NOT NULL, one_pip_means VARCHAR(32) NOT NULL, margin_factor VARCHAR(32) NOT NULL DEFAULT '1.0',
                min_size VARCHAR(32) NOT NULL, max_size VARCHAR(32) NOT NULL, step_size VARCHAR(32) NOT NULL,
                min_notional VARCHAR(32) NOT NULL, max_notional VARCHAR(32) NOT NULL, step_notional VARCHAR(32) NOT NULL,
                min_price VARCHAR(32) NOT NULL, max_price VARCHAR(32) NOT NULL, step_price VARCHAR(32) NOT NULL,
                maker_fee VARCHAR(32) NOT NULL DEFAULT '0', taker_fee VARCHAR(32) NOT NULL DEFAULT '0',
                maker_commission VARCHAR(32) NOT NULL DEFAULT '0', taker_commission VARCHAR(32) NOT NULL DEFAULT '0',
                UNIQUE(broker_id, market_id))""")

        self._db.commit()

    def setup_userdata_sql(self):
        cursor = self._db.cursor()

        # asset table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS asset(
                id SERIAL PRIMARY KEY,
                broker_id VARCHAR(255) NOT NULL, account_id VARCHAR(255) NOT NULL, asset_id VARCHAR(255) NOT NULL,
                last_trade_id VARCHAR(32) NOT NULL, timestamp BIGINT NOT NULL, 
                quantity VARCHAR(32) NOT NULL, price VARCHAR(32) NOT NULL, quote_symbol VARCHAR(32) NOT NULL,
                UNIQUE(broker_id, account_id, asset_id))""")

        # trade table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS user_trade(
                id SERIAL PRIMARY KEY,
                broker_id VARCHAR(255) NOT NULL, account_id VARCHAR(255) NOT NULL, market_id VARCHAR(255) NOT NULL,
                appliance_id VARCHAR(255) NOT NULL,
                trade_id INTEGER NOT NULL,
                trade_type INTEGER NOT NULL,
                data TEXT NOT NULL DEFAULT '{}',
                operations TEXT NOT NULL DEFAULT '{}',
                UNIQUE(broker_id, account_id, market_id, appliance_id, trade_id))""")

        # trader table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS user_trader(
                id SERIAL PRIMARY KEY,
                broker_id VARCHAR(255) NOT NULL, account_id VARCHAR(255) NOT NULL, market_id VARCHAR(255) NOT NULL,
                appliance_id VARCHAR(255) NOT NULL,
                activity INTEGER NOT NULL DEFAULT 1,
                data TEXT NOT NULL DEFAULT '{}',
                regions TEXT NOT NULL DEFAULT '[]',
                alerts TEXT NOT NULL DEFAULT '[]',
                UNIQUE(broker_id, account_id, market_id, appliance_id))""")

        self._db.commit()

    def setup_ohlc_sql(self):
        cursor = self._db.cursor()

        # ohlc table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS ohlc(
                id SERIAL PRIMARY KEY,
                broker_id VARCHAR(255) NOT NULL, market_id VARCHAR(255) NOT NULL,
                timestamp BIGINT NOT NULL, timeframe INTEGER NOT NULL,
                bid_open VARCHAR(32) NOT NULL, bid_high VARCHAR(32) NOT NULL, bid_low VARCHAR(32) NOT NULL, bid_close VARCHAR(32) NOT NULL,
                ask_open VARCHAR(32) NOT NULL, ask_high VARCHAR(32) NOT NULL, ask_low VARCHAR(32) NOT NULL, ask_close VARCHAR(32) NOT NULL,
                volume VARCHAR(48) NOT NULL,
                UNIQUE(broker_id, market_id, timestamp, timeframe))""")

        # liquidation
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS liquidation(
                id SERIAL PRIMARY KEY,
                broker_id VARCHAR(255) NOT NULL, market_id VARCHAR(255) NOT NULL,
                timestamp BIGINT NOT NULL,
                direction INTEGER NOT NULL,
                price VARCHAR(32) NOT NULL,
                quantity VARCHAR(32) NOT NULL)""")

        self._db.commit()

    def create_ohlc_streamer(self, broker_id, market_id, timeframe, from_date, to_date, buffer_size=8192):
        """
        Create a new ohlc streamer.
        """
        return OhlcStreamer(self._db, timeframe, broker_id, market_id, from_date, to_date, buffer_size)

    #
    # Processing
    #

    def process_market(self):
        #
        # insert market info
        #

        self.lock()
        mki = self._pending_market_info_insert
        self._pending_market_info_insert = []
        self.unlock()

        if mki:
            try:
                cursor = self._db.cursor()

                for mi in mki:
                    if mi[16] is None:
                        # margin factor is unavailable when market is down, so use previous value if available
                        cursor.execute("""SELECT margin_factor FROM market WHERE broker_id = '%s' AND market_id = '%s'""" % (mi[0], mi[1]))
                        row = cursor.fetchone()

                        if row:
                            # replace by previous margin factor from the DB
                            margin_factor = row[0]
                            mi = list(mi)
                            mi[16] = margin_factor
                        else:
                            mi[16] = "1.0"

                        cursor.execute("""INSERT INTO market(broker_id, market_id, symbol,
                                        market_type, unit_type, contract_type,
                                        trade_type, orders,
                                        base, base_display, base_precision,
                                        quote, quote_display, quote_precision,
                                        expiry, timestamp,
                                        lot_size, contract_size, base_exchange_rate,
                                        value_per_pip, one_pip_means, margin_factor,
                                        min_size, max_size, step_size,
                                        min_notional, max_notional, step_notional,
                                        min_price, max_price, step_price,
                                        maker_fee, taker_fee, maker_commission, taker_commission) 
                                    VALUES(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                                    ON CONFLICT (broker_id, market_id) DO UPDATE SET symbol = EXCLUDED.symbol,
                                        market_type = EXCLUDED.market_type, unit_type = EXCLUDED.unit_type, contract_type = EXCLUDED.contract_type,
                                        trade_type = EXCLUDED.trade_type, orders = EXCLUDED.orders,
                                        base = EXCLUDED.base, base_display = EXCLUDED.base_display, base_precision = EXCLUDED.base_precision,
                                        quote = EXCLUDED.quote, quote_display = EXCLUDED.quote_display, quote_precision = EXCLUDED.quote_precision,
                                        expiry = EXCLUDED.expiry, timestamp = EXCLUDED.timestamp,
                                        lot_size = EXCLUDED.lot_size, contract_size = EXCLUDED.contract_size, base_exchange_rate = EXCLUDED.base_exchange_rate,
                                        value_per_pip = EXCLUDED.value_per_pip, one_pip_means = EXCLUDED.one_pip_means, margin_factor = EXCLUDED.margin_factor,
                                        min_size = EXCLUDED.min_size, max_size = EXCLUDED.max_size, step_size = EXCLUDED.step_size,
                                        min_notional = EXCLUDED.min_notional, max_notional = EXCLUDED.max_notional, step_notional = EXCLUDED.step_notional,
                                        min_price = EXCLUDED.min_price, max_price = EXCLUDED.max_price, step_price = EXCLUDED.step_price,
                                        maker_fee = EXCLUDED.maker_fee, taker_fee = EXCLUDED.taker_fee, maker_commission = EXCLUDED.maker_commission, taker_commission = EXCLUDED.taker_commission""",
                                    (*mi,))

                self._db.commit()
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_market_info_insert = mki + self._pending_market_info_insert
                self.unlock()

            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_market_info_insert = mki + self._pending_market_info_insert
                self.unlock()

        #
        # select market info
        #

        self.lock()
        mis = self._pending_market_info_select
        self._pending_market_info_select = []
        self.unlock()

        if mis:
            try:
                cursor = self._db.cursor()

                for mi in mis:
                    cursor.execute("""SELECT symbol,
                                        market_type, unit_type, contract_type,
                                        trade_type, orders,
                                        base, base_display, base_precision,
                                        quote, quote_display, quote_precision,
                                        expiry, timestamp,
                                        lot_size, contract_size, base_exchange_rate,
                                        value_per_pip, one_pip_means, margin_factor,
                                        min_size, max_size, step_size,
                                        min_notional, max_notional, step_notional,
                                        min_price, max_price, step_price,
                                        maker_fee, taker_fee, maker_commission, taker_commission FROM market
                                    WHERE broker_id = '%s' AND market_id = '%s'""" % (
                                        mi[1], mi[2]))

                    row = cursor.fetchone()

                    if row:
                        market_info = Market(mi[2], row[0])

                        market_info.is_open = True

                        market_info.market_type = row[1]
                        market_info.unit_type = row[2]
                        market_info.contract_type = row[3]

                        market_info.trade = row[4]
                        market_info.orders = row[5]

                        market_info.set_base(row[6], row[7], int(row[8]))
                        market_info.set_quote(row[9], row[10], int(row[11]))

                        market_info.expiry = row[12]
                        market_info.last_update_time = row[13] * 0.001

                        market_info.lot_size = float(row[14])
                        market_info.contract_size = float(row[15])
                        market_info.base_exchange_rate = float(row[16])
                        market_info.value_per_pip = float(row[17])
                        market_info.one_pip_means = float(row[18])

                        if row[19] is not None or row[19] is not 'None':
                            if row[19] == '-':  # not defined mean 1.0 or no margin
                                market_info.margin_factor = 1.0
                            else:
                                market_info.margin_factor = float(row[19])

                        market_info.set_size_limits(float(row[20]), float(row[21]), float(row[22]))
                        market_info.set_notional_limits(float(row[23]), float(row[24]), float(row[25]))
                        market_info.set_price_limits(float(row[26]), float(row[27]), float(row[28]))

                        market_info.maker_fee = float(row[29])
                        market_info.taker_fee = float(row[30])

                        market_info.maker_commission = float(row[31])
                        market_info.taker_commission = float(row[32])
                    else:
                        market_info = None

                    # notify
                    mi[0].notify(Signal.SIGNAL_MARKET_INFO_DATA, mi[1], (mi[2], market_info))
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_market_info_select = mis + self._pending_market_info_select
                self.unlock()

            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_market_info_select = mis + self._pending_market_info_select
                self.unlock()

        #
        # select market list
        #

        self.lock()
        mls = self._pending_market_list_select
        self._pending_market_list_select = []
        self.unlock()

        if mls:
            try:
                cursor = self._db.cursor()

                for m in mls:
                    cursor.execute("""SELECT market_id, symbol, base, quote FROM market WHERE broker_id = '%s'""" % (m[1],))

                    rows = cursor.fetchall()

                    market_list = []

                    for row in rows:
                        market_list.append(row)

                    # notify
                    m[0].notify(Signal.SIGNAL_MARKET_LIST_DATA, m[1], market_list)
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_market_list_select = mls + self._pending_market_list_select
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_market_list_select = mls + self._pending_market_list_select
                self.unlock()

    def process_userdata(self):
        #
        # inset asset
        #
        self.lock()
        uai = self._pending_asset_insert
        self._pending_asset_insert = []
        self.unlock()

        if uai:
            try:
                cursor = self._db.cursor()

                for ua in uai:
                    cursor.execute("""
                        INSERT INTO asset(broker_id, account_id, asset_id, last_trade_id, timestamp, quantity, price, quote_symbol)
                            VALUES(%s, %s, %s, %s, %s, %s, %s, %s)
                        ON CONFLICT (broker_id, account_id, asset_id) DO UPDATE SET 
                            last_trade_id = EXCLUDED.last_trade_id, timestamp = EXCLUDED.timestamp, quantity = EXCLUDED.quantity, price = EXCLUDED.price, quote_symbol = EXCLUDED.quote_symbol""", (*ua,))

                self._db.commit()
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_asset_insert = uai + self._pending_asset_insert
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_asset_insert = uai + self._pending_asset_insert
                self.unlock()

        #
        # select asset
        #

        self.lock()
        uas = self._pending_asset_select
        self._pending_asset_select = []
        self.unlock()

        if uas:
            try:
                cursor = self._db.cursor()

                for ua in uas:
                    cursor.execute("""SELECT asset_id, last_trade_id, timestamp, quantity, price, quote_symbol FROM asset
                        WHERE broker_id = '%s' AND account_id = '%s'""" % (ua[2], ua[3]))

                    rows = cursor.fetchall()

                    assets = []

                    for row in rows:
                        asset = Asset(ua[1], row[0])

                        # only a sync will tell which quantity is free, which one is locked
                        asset.update_price(float(row[2]) * 0.001, row[1], float(row[4]), row[5])
                        asset.set_quantity(0.0, float(row[3]))

                        assets.append(asset)

                    # notify
                    ua[0].notify(Signal.SIGNAL_ASSET_DATA_BULK, ua[2], assets)
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_asset_select = uas + self._pending_asset_select
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_asset_select = uas + self._pending_asset_select
                self.unlock()

        #
        # inset user_trade
        #

        self.lock()
        uti = self._pending_user_trade_insert
        self._pending_user_trade_insert = []
        self.unlock()

        if uti:
            try:
                cursor = self._db.cursor()           

                query = ' '.join((
                    "INSERT INTO user_trade(broker_id, account_id, market_id, appliance_id, trade_id, trade_type, data, operations) VALUES",
                    ','.join(["('%s', '%s', '%s', '%s', %i, %i, '%s', '%s')" % (ut[0], ut[1], ut[2], ut[3], ut[4], ut[5],
                            str(ut[6]).replace("'", "''"), str(ut[7]).replace("'", "''")) for ut in uti]),
                    "ON CONFLICT (broker_id, account_id, market_id, appliance_id, trade_id) DO UPDATE SET trade_type = EXCLUDED.trade_type, data = EXCLUDED.data, operations = EXCLUDED.operations"
                ))

                cursor.execute(query)

                self._db.commit()
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_user_trade_insert = uti + self._pending_user_trade_insert
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_user_trade_insert = uti + self._pending_user_trade_insert
                self.unlock()

        #
        # select user_trade
        #

        self.lock()
        uts = self._pending_user_trade_select
        self._pending_user_trade_select = []
        self.unlock()

        if uts:
            try:
                cursor = self._db.cursor()

                for ut in uts:
                    cursor.execute("""SELECT market_id, trade_id, trade_type, data, operations FROM user_trade WHERE
                        broker_id = '%s' AND account_id = '%s' AND appliance_id = '%s'""" % (ut[2], ut[3], ut[4]))

                    rows = cursor.fetchall()

                    user_trades = []

                    for row in rows:
                        user_trades.append((row[0], row[1], row[2], json.loads(row[3]), json.loads(row[4])))

                    # notify
                    ut[0].notify(Signal.SIGNAL_STRATEGY_TRADE_LIST, ut[4], user_trades)
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_user_trade_select = uts + self._pending_user_trade_select
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_user_trade_select = uts + self._pending_user_trade_select
                self.unlock()

        #
        # insert user_trader
        #

        self.lock()
        uti = self._pending_user_trader_insert
        self._pending_user_trader_insert = []
        self.unlock()

        if uti:
            try:
                cursor = self._db.cursor()

                query = ' '.join((
                    "INSERT INTO user_trader(broker_id, account_id, market_id, appliance_id, activity, data, regions, alerts) VALUES",
                    ','.join(["('%s', '%s', '%s', '%s', %i, '%s', '%s')" % (ut[0], ut[1], ut[2], ut[3], 1 if ut[4] else 0,
                            str(ut[5]).replace("'", "''"),
                            str(ut[6]).replace("'", "''"),
                            str(ut[7]).replace("'", "''")) for ut in uti]),
                    "ON CONFLICT (broker_id, market_id, appliance_id) DO UPDATE SET activity = EXCLUDED.activity, data = EXCLUDED.data, regions = EXCLUDED.regions, alerts = EXCLUDED.alerts"
                ))

                cursor.execute(query)

                self._db.commit()
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_user_trader_insert = uti + self._pending_user_trader_insert
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_user_trader_insert = uti + self._pending_user_trader_insert
                self.unlock()

        #
        # select user_trader
        #

        self.lock()
        uts = self._pending_user_trader_select
        self._pending_user_trader_select = []
        self.unlock()

        if uts:
            try:
                cursor = self._db.cursor()

                for ut in uts:
                    cursor.execute("""SELECT market_id, activity, data, regions, alerts FROM user_trader WHERE
                        broker_id = '%s' AND account_id = '%s' AND appliance_id = '%s'""" % (ut[2], ut[3], ut[4]))

                    rows = cursor.fetchall()

                    user_traders = []

                    for row in rows:
                        user_traders.append((row[0], row[1] > 0, json.loads(row[2]), json.loads(row[3]), json.loads(row[4])))

                    # notify
                    ut[0].notify(Signal.SIGNAL_STRATEGY_TRADER_LIST, ut[4], user_traders)
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_user_trader_select = uts + self._pending_user_trader_select
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_user_trader_select = uts + self._pending_user_trader_select
                self.unlock()

    def process_ohlc(self):       
        #
        # select market ohlcs
        #

        self.lock()
        mks = self._pending_ohlc_select
        self._pending_ohlc_select = []
        self.unlock()

        if mks:
            try:
                cursor = self._db.cursor()

                for mk in mks:
                    if mk[6]:
                        # last n
                        cursor.execute("""SELECT COUNT(*) FROM ohlc WHERE broker_id = '%s' AND market_id = '%s' AND timeframe = %s""" % (mk[1], mk[2], mk[3]))
                        count = int(cursor.fetchone()[0])
                        offset = max(0, count - mk[6])

                        # LIMIT should not be necessary then
                        cursor.execute("""SELECT timestamp, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume FROM ohlc
                                        WHERE broker_id = '%s' AND market_id = '%s' AND timeframe = %s ORDER BY timestamp ASC LIMIT %i OFFSET %i""" % (
                                            mk[1], mk[2], mk[3], mk[6], offset))
                    elif mk[4] and mk[5]:
                        # from to
                        cursor.execute("""SELECT timestamp, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume FROM ohlc
                                        WHERE broker_id = '%s' AND market_id = '%s' AND timeframe = %s AND timestamp >= %i AND timestamp <= %i ORDER BY timestamp ASC""" % (
                                            mk[1], mk[2], mk[3], mk[4], mk[5]))
                    elif mk[4]:
                        # from to now
                        cursor.execute("""SELECT timestamp, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume FROM ohlc
                                        WHERE broker_id = '%s' AND market_id = '%s' AND timeframe = %s AND timestamp >= %i ORDER BY timestamp ASC""" % (
                                            mk[1], mk[2], mk[3], mk[4]))
                    elif mk[5]:
                        # to now
                        cursor.execute("""SELECT timestamp, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume FROM ohlc
                                        WHERE broker_id = '%s' AND market_id = '%s' AND timeframe = %s AND timestamp <= %i ORDER BY timestamp ASC""" % (
                                            mk[1], mk[2], mk[3], mk[5]))
                    else:
                        # all
                        cursor.execute("""SELECT timestamp, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume FROM ohlc
                                        WHERE broker_id = '%s' AND market_id = '%s' AND timeframe = %s ORDER BY timestamp ASC""" % (
                                            mk[1], mk[2], mk[3]))

                    rows = cursor.fetchall()

                    ohlcs = []

                    for row in rows:
                        timestamp = float(row[0]) * 0.001  # to float second timestamp
                        ohlc = Candle(timestamp, mk[3])

                        ohlc.set_bid_ohlc(float(row[1]), float(row[2]), float(row[3]), float(row[4]))
                        ohlc.set_ofr_ohlc(float(row[5]), float(row[6]), float(row[7]), float(row[8]))

                        # if float(row[9]) <= 0:
                        #   # prefer to ignore empty volume ohlc because it can broke volume signal and it is a no way but it could be
                        #   # a lack of this information like on SPX500 of ig.com. So how to manage that cases...
                        #   continue

                        ohlc.set_volume(float(row[9]))

                        if ohlc.timestamp >= Instrument.basetime(mk[3], time.time()):
                            ohlc.set_consolidated(False)  # current

                        ohlcs.append(ohlc)

                    # notify
                    mk[0].notify(Signal.SIGNAL_CANDLE_DATA_BULK, mk[1], (mk[2], mk[3], ohlcs))
            except self.psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_ohlc_select = mks + self._pending_ohlc_select
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_ohlc_select = mks + self._pending_ohlc_select
                self.unlock()

        #
        # insert market ohlcs
        #

        if time.time() - self._last_ohlc_flush >= 60 or len(self._pending_ohlc_insert) > 500:
            self.lock()
            mkd = self._pending_ohlc_insert
            self._pending_ohlc_insert = []
            self.unlock()

            if mkd:
                try:
                    cursor = self._db.cursor()

                    elts = []
                    data = set()

                    for mk in mkd:
                        if (mk[0], mk[1], mk[2], mk[3]) not in data:
                            elts.append("('%s', '%s', %i, %i, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')" % (mk[0], mk[1], mk[2], mk[3], mk[4], mk[5], mk[6], mk[7], mk[8], mk[9], mk[10], mk[11], mk[12]))
                            data.add((mk[0], mk[1], mk[2], mk[3]))

                    query = ' '.join(("INSERT INTO ohlc(broker_id, market_id, timestamp, timeframe, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume) VALUES",
                                ','.join(elts),
                                "ON CONFLICT (broker_id, market_id, timestamp, timeframe) DO UPDATE SET bid_open = EXCLUDED.bid_open, bid_high = EXCLUDED.bid_high, bid_low = EXCLUDED.bid_low, bid_close = EXCLUDED.bid_close, ask_open = EXCLUDED.ask_open, ask_high = EXCLUDED.ask_high, ask_low = EXCLUDED.ask_low, ask_close = EXCLUDED.ask_close, volume = EXCLUDED.volume"))

                    # query = ' '.join((
                    #     "INSERT INTO ohlc(broker_id, market_id, timestamp, timeframe, bid_open, bid_high, bid_low, bid_close, ask_open, ask_high, ask_low, ask_close, volume) VALUES",
                    #     ','.join(["('%s', '%s', %i, %i, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')" % (mk[0], mk[1], mk[2], mk[3], mk[4], mk[5], mk[6], mk[7], mk[8], mk[9], mk[10], mk[11], mk[12]) for mk in mkd]),
                    #     "ON CONFLICT (broker_id, market_id, timestamp, timeframe) DO UPDATE SET bid_open = EXCLUDED.bid_open, bid_high = EXCLUDED.bid_high, bid_low = EXCLUDED.bid_low, bid_close = EXCLUDED.bid_close, ask_open = EXCLUDED.ask_open, ask_high = EXCLUDED.ask_high, ask_low = EXCLUDED.ask_low, ask_close = EXCLUDED.ask_close, volume = EXCLUDED.volume"
                    # ))

                    cursor.execute(query)

                    self._db.commit()
                except self.psycopg2.OperationalError as e:
                    self.try_reconnect(e)

                    # retry the next time
                    self.lock()
                    self._pending_ohlc_insert = mkd + self._pending_ohlc_insert
                    self.unlock()
                except Exception as e:
                    self.on_error(e)

                    # retry the next time
                    self.lock()
                    self._pending_ohlc_insert = mkd + self._pending_ohlc_insert
                    self.unlock()

                self._last_ohlc_flush = time.time()

        #
        # insert market liquidation
        #

        self.lock()
        mkd = self._pending_liquidation_insert
        self._pending_liquidation_insert = []
        self.unlock()

        if mkd:
            try:
                cursor = self._db.cursor()

                elts = []
                data = set()

                for mk in mkd:
                    if (mk[0], mk[1], mk[2]) not in data:
                        elts.append("('%s', '%s', %i, %i, '%s', '%s')" % (mk[0], mk[1], mk[2], mk[3], mk[4], mk[5]))
                        data.add((mk[0], mk[1], mk[2]))

                # query = ' '.join(("INSERT INTO liquidation(broker_id, market_id, timestamp, direction, price, quantity) VALUES",
                #             ','.join(elts),
                #             "ON CONFLICT (broker_id, market_id, timestamp) DO NOTHING"))
                query = ' '.join(("INSERT INTO liquidation(broker_id, market_id, timestamp, direction, price, quantity) VALUES", ','.join(elts)))

                cursor.execute(query)

                self._db.commit()
            except psycopg2.OperationalError as e:
                self.try_reconnect(e)

                # retry the next time
                self.lock()
                self._pending_liquidation_insert = mkd + self._pending_liquidation_insert
                self.unlock()
            except Exception as e:
                self.on_error(e)

                # retry the next time
                self.lock()
                self._pending_liquidation_insert = mkd + self._pending_liquidation_insert
                self.unlock()

        #
        # clean older ohlcs
        #

        if time.time() - self._last_ohlc_clean >= OhlcStorage.CLEANUP_DELAY:
            try:
                now = time.time()
                cursor = self._db.cursor()

                for timeframe, timestamp in OhlcStorage.CLEANERS:
                    ts = int(now - timestamp) * 1000
                    # @todo make a count before
                    cursor.execute("DELETE FROM ohlc WHERE timeframe <= %i AND timestamp < %i" % (timeframe, ts))

                self._db.commit()
            except psycopg2.OperationalError as e:
                self.try_reconnect(e)
            except Exception as e:
                self.on_error(e)

            self._last_ohlc_clean = time.time()

    def on_error(self, e):
        logger.error(repr(e))  # + '\n' + e.pgerror)
        time.sleep(5.0)

    def try_reconnect(self, e):
        logger.error(repr(e))
        time.sleep(5.0)

        self._db = None

        if self._conn_str:
            n = 10  # max retry
            while n > 0:
                try:
                    self._db = self.psycopg2.connect(self._conn_str)
                except Exception as e:
                    time.sleep(5.0)

                n -= 1

    @property
    def connected(self):
        return self._db != None
