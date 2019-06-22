# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messageOhlc(messageCore):
    """
    messageOhlc Class.
    """
    class_message_name = "messageOhlc"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.marketId = None
        self.timestamp = None
        self.timeframe = None
        self.open = None
        self.high = None
        self.low = None
        self.close = None
        self.volume = None
        self.consolidated = None

    def set(self, marketId, timestamp, timeframe, open_, high, low, close, volume, consolidated)
        self.marketId = marketId
        self.timestamp = timestamp
        self.timeframe = timeframe
        self.open = open_
        self.high = high
        self.low = low
        self.close = close
        self.volume = volume
        self.consolidated = consolidated


    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeString(self.marketId)
        self.writeDouble(self.timestamp)
        self.writeDouble(self.timeframe)
        self.writeDouble(self.open)
        self.writeDouble(self.high)
        self.writeDouble(self.low)
        self.writeDouble(self.close)
        self.writeDouble(self.volume)
        self.writeDouble(self.consolidated)

    def setMarketId(self, marketId):
        self.marketId = marketId

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def settimeframe(self, timeframe):
        self.timeframe = timeframe

    def setOpen(self, open):
        self.open = open

    def setHigh(self, high):
        self.high = high

    def setLow(self, low):
        self.low = low

    def setClose(self, close):
        self.close = close

    def setVolume(self, volume):
        self.volume = volume

    def setConsolidated(self, consolidated):
        self.Consolidated = consolidated


messageOhlc.register()
