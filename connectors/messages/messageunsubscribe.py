# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageUnsubscribe(messageCore):
    """
    messageUnsubscribe Class.
    """
    class_message_name = "messageUnsubscribe"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.subId = None
        self.marketId = None

    def read(self, message):
        super().read(self, message)
        self.subInt = self.readInt32()
        self.marketId = self.readString()
       
    def write(self):
        super().write()
        self.writeInt32(self.subId)

    def subId(self):
        return self.subId


messageUnsubscribe.register()
