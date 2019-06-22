# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageListOrders(messageCore):
    """
    messageListOrders Class.
    """
    class_message_name = "messageListOrders"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self._list_orders = None

    def read(self, message):
        super().read(self, message)
           
    def write(self):
        # TODO

    @property
    def list_orders(self):
        return self._list_orders

    def set_list_orders(self, list_orders):
        self._list_orders = list_orders


messageListOrders.register()
