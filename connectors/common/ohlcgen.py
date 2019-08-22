# @date 2018-09-05
# @author Frederic SCHERMA
# @license Copyright (c) 2018 Dream Overflow
# Higher OHLC generator.

from datetime import datetime, timedelta
from common.utils import UTC

from instrument.instrument import Candle


class OhlcGenerator(object):

    __slots__ = '_from_tf', '_to_tf', '_ohlc', '_last_timestamp', '_last_consumed'

    def __init__(self, from_tf, to_tf):
        """
        @param to_tf Generated OHLC time unit.
        """
        if from_tf and (int(to_tf) % int(from_tf) != 0):
            raise(ValueError("From timeframe %s must be an integral divider of to timeframe %s" % (from_tf, to_tf)))

        self._from_tf = float(from_tf)
        self._to_tf = float(to_tf)
        self._ohlc = None
        self._last_timestamp = 0
        self._last_consumed = 0

    @property
    def current(self):
        """
        If exists returns the current non closed OHLC.
        """
        return self._ohlc

    @current.setter
    def current(self, ohlc):
        self._ohlc = ohlc

    @property
    def last_timestamp(self):
        return self._last_timestamp

    @property
    def last_consumed(self):
        return self._last_consumed

    @property
    def from_tf(self):
        return self._from_tf
    
    @property
    def to_tf(self):
        return self._to_tf
    
    def generate_from_ohlcs(self, from_ohlcs, ignore_non_ended=True):
        """
        Generate as many higher ohlcs as possible from the array of ohlcs given in parameters.
        @note Non ended ohlcs are ignored because it will false the volume.
        """
        to_ohlcs = []
        self._last_consumed = 0

        for from_ohlc in from_ohlcs:
            to_ohlc = self.update_from_ohlc(from_ohlc)
            if to_ohlc:
                to_ohlcs.append(to_ohlc)

            self._last_consumed += 1

        return to_ohlcs

    def generate_from_ticks(self, from_ticks):
        """
        Generate as many higher ohlcs as possible from the array of ticks given in parameters.
        """
        to_ohlcs = []
        self._last_consumed = 0

        for from_tick in from_ticks:
            to_ohlc = self.update_from_tick(from_tick)
            if to_ohlc:
                to_ohlcs.append(to_ohlc)

            self._last_consumed += 1

        return to_ohlcs

    def basetime(self, timestamp):
        if self._to_tf < 7*24*60*60:
            # simplest
            return int(timestamp / self._to_tf) * self._to_tf
        elif self._to_tf == 7*24*60*60:
            # must find the UTC first day of week
            dt = datetime.utcfromtimestamp(timestamp)
            dt = dt.replace(hour=0, minute=0, second=0, microsecond=0, tzinfo=UTC()) - timedelta(days=dt.weekday())
            return dt.timestamp()
        elif self._to_tf == 30*24*60*60:
            # replace by first day of month at 00h00 UTC
            dt = datetime.utcfromtimestamp(timestamp)
            dt = dt.replace(day=1, hour=0, minute=0, second=0, microsecond=0, tzinfo=UTC())
            return dt.timestamp()

    def update_from_tick(self, from_tick):
        if from_tick is None:
            return None

        if from_tick[0] <= self._last_timestamp:
            # already done (and what if two consecutives ticks have the same timestamp ?)
            return None

        # basetime can be slow, uses only to create a new OHLC
        # base_time = self.basetime(from_tick[0])
        ended_ohlc = None

        # if self._ohlc and self._ohlc.timestamp+self._to_tf <= base_time:
        if self._ohlc and from_tick[0] >= self._ohlc.timestamp+self._to_tf:
            # need to close the ohlc and to open a new one
            self._ohlc.set_consolidated(True)
            ended_ohlc = self._ohlc

            self._ohlc = None

        if self._ohlc is None:
            # open a new one
            base_time = self.basetime(from_tick[0])
            self._ohlc = Candle(base_time, self._to_tf)

            self._ohlc.set_consolidated(False)

            # all open, close, low high from the initial ohlc
            self._ohlc.set_bid(from_tick[1])
            self._ohlc.set_ofr(from_tick[2])

        # update volumes
        self._ohlc._volume += from_tick[3]

        # update bid prices

        # bid high/low
        self._ohlc._bid_high = max(self._ohlc._bid_high, from_tick[1])
        self._ohlc._bid_low = min(self._ohlc._bid_low, from_tick[1])

        # potential close
        self._ohlc._bid_close = from_tick[1]

        # update ofr prices

        # ofr high/low
        self._ohlc._ofr_high = max(self._ohlc._ofr_high, from_tick[2])
        self._ohlc._ofr_low = min(self._ohlc._ofr_low, from_tick[2])

        # potential close
        self._ohlc._ofr_close = from_tick[2]

        # keep last timestamp
        self._last_timestamp = from_tick[0]

        return ended_ohlc


    def update_from_ohlc(self, from_ohlc):
        """
        From a timeframe, create/update ohlc to another timeframe, that must be greater and a multiple of.
        Example of creating/updating hourly ohlc for 1 minute ohlcs.

        Must be called each time a new ohlc of the lesser timeframe is append.
        It only create the last or update the current ohlc.

        A non ended ohlc is ignored because it will false the volume.
        """
        if from_ohlc is None or not from_ohlc.ended:
            return None

        if self._from_tf != from_ohcl.timeframe:
            raise ValueError("From ohlc must be of time unit %s but %s is provided" % (self._from_tf, from_ohlc.timeframe))

        if from_ohlc.timestamp <= self._last_timestamp:
            # already done
            return None

        # base_time = self.basetime(from_ohlc.timestamp)
        ended_ohlc = None

        # if self._ohlc and self._ohlc.timestamp+self._to_tf <= base_time:
        if self._ohlc and from_ohlc.timestamp >= self._ohlc.timestamp+self._to_tf:
            # need to close the ohlc and to open a new one
            self._ohlc.set_consolidated(True)
            ended_ohlc = self._ohlc

            self._ohlc = None

        if self._ohlc is None:
            # open a new one
            base_time = self.basetime(from_ohlc.timestamp)
            self._ohlc = Candle(base_time, self._to_tf)

            self._ohlc.set_consolidated(False)

            # all open, close, low high from the initial ohlc
            self._ohlc.copy_bid(from_ohlc)
            self._ohlc.copy_ofr(from_ohlc)

        # update volumes
        self._ohlc._volume += from_ohlc.volume

        # update bid prices
        self._ohlc._bid_high = max(self._ohlc._bid_high, from_ohlc._bid_high)
        self._ohlc._bid_low = min(self._ohlc._bid_low, from_ohlc._bid_low)

        # potential close
        self._ohlc._bid_close = from_ohlc._bid_close

        # update ofr prices
        self._ohlc._ofr_high = max(self._ohlc._ofr_high, from_ohlc._ofr_high)
        self._ohlc._ofr_low = min(self._ohlc._ofr_low, from_ohlc._ofr_low)

        # potential close
        self._ohlc._ofr_close = from_ohlc._ofr_close

        # keep last timestamp
        self._last_timestamp = from_ohlc.timestamp

        return ended_ohlc
