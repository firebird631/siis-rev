# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messageStatus(messageCore):
    """
    messageStatus Class.
    """
    class_message_name = "messageStatus"

    Event = {
        0: 'UPDATED'
    }

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.event = 0
        self.timestamp = None

    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeInt8(self.event)
        self.writeDouble(self.timestamp)

    def setEvent(self, event):
        self.event = event

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp


messageStatus.register()
