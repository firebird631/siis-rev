# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageClosePosition(messageCore):
    """
    messageClosePosition Class.
    """
    class_message_name = "messageClosePosition"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self._position_id = None

    def read(self, message):
        self._position_id = self.readInt32()
           
    def write(self):
        self.writeInt32(self._position_id)

    @property
    def position_id(self):
        return self._position_id

    def set_position_id(self, position_id):
        self._position_id = position_id


messageClosePosition.register()
            