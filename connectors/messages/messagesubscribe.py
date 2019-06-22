# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageSubscribe(messageCore):
    """
    messageSubscribe Class.
    """
    class_message_name = "messageSubscribe"

    Type = {
        0: 'TYPE_TICK',
        1: 'TICK_OHLC_MID',
        2: 'TICK_OHLC_BID',
        3: 'TICK_OHLC_OFR',
        4: 'TICK_ORDER_BOOK'
    }

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.type_sub = None
        self.marketId = None
        self.timeframe = None
        self.bookdepth = None
        self.subId = None

    def read(self, message):
        super().read(self, message)
        self.type_sub = self.readInt8()
        self.marketId = self.readString()
        self.timeframe = self.readDouble()

        if messageSubscribe.Type[self.type_sub] == 'TICK_ORDER_BOOK':
            self.bookdepth = self.readInt32()

    def write(self):
        super().write()
        self.writeInt32(self.subId)

    def setSubId(self, subId):
        self.subId = subId


messageSubscribe.register()
