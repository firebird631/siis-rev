# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageListPositions(messageCore):
    """
    messageListPositions Class.
    """
    class_message_name = "messageListPositions"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self._list_positions = None

    def read(self, message):
        # TODO
           
    def write(self):
        # TODO

    @property
    def list_positions(self):
        return self._list_positions

    def set_list_positions(self, list_positions):
        self._list_positions = list_positions


messageListPositions.register()
