# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageCancelOrder(messageCore):
    """
    messageCancelOrder Class.
    """
    class_message_name = "messageCancelOrder"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self._order_id = None

    def read(self, message):
        self._order_id = self.readInt32()
           
    def write(self):
        self.writeInt32(self._order_id)

    @property
    def order_id(self):
        return self._order_id

    def set_order_id(self, order_id):
        self._order_id = order_id


messageCancelOrder.register()
            
