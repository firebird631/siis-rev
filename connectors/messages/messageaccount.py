# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messageAccount(messageCore):
    """
    messageAccount Class.
    """
    class_message_name = "messageAccount"

    Event = {
        0: 'UPDATED'
    }

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.event = 0
        self.timestamp = None
        self.freeMargin = None
        self.reservedMargin = None
        self.marginFactor = None

    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeInt8(self.event)
        self.writeDouble(self.timestamp)
        self.writeDouble(self.freeMargin)
        self.writeDouble(self.reservedMargin)
        self.writeDouble(self.marginFactor)

    def setEvent(self, event):
        self.event = event

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def setFreeMargin(self, freeMargin):
        self.freeMargin = freeMargin

    def setReservedMargin(self, reservedMargin):
        self.reservedMargin = reservedMargin

    def setMarginFactor(self, marginFactor):
        self.marginFactor = marginFactor


messageAcount.register()
