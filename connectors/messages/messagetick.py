# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messageTick(messageCore):
    """
    messageTick Class.
    """
    class_message_name = "messageTick"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.marketId = None
        self.timestamp = None
        self.bid = None
        self.ofr_ask = None
        self.volume = None

    def set(self, marketId, timestamp, bid, ofr_ask, volume):
        self.marketId = marketId
        self.timestamp = timestamp
        self.bid = bid
        self.ofr_ask = ofr_ask
        self.volume = volume            

    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeString(self.marketId)
        self.writeDouble(self.timestamp)
        self.writeDouble(self.bid)
        self.writeDouble(self.ofr_ask)
        self.writeDouble(self.volume)

    def setMarketId(self, marketId):
        self.marketId = marketId

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def setOfrAsk(self, ofr_ask):
        self.ofr_ask = ofr_ask

    def setVolume(self, volume):
        self.volume = volume


messageTick.register()
