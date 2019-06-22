# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messageAsset(messageCore):
    """
    messageAsset Class.
    """
    class_message_name = "messageAsset"

    Event = {
        0: 'UPDATED'
    }

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.event = 0
        self.timestamp = None
        self.symbol = None
        self.freeQuantity = None
        self.lockedQuantity = None

    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeInt8(self.event)
        self.writeDouble(self.timestamp)
        self.writeDouble(self.symbol)
        self.writeDouble(self.freeQuantity)
        self.writeDouble(self.lockedQuantity)

    def setEvent(self, event):
        self.event = event

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def setSymbol(self, symbol):
        self.symbol = symbol

    def setFreeQuantity(self, freeQuantity):
        self.freeQuantity = freeQuantity

    def setLockedQuantity(self, lockedQuantity):
        self.lockedQuantity = lockedQuantity


messageAsset.register()
