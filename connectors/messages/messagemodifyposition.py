# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageModifyPosition(messageCore):
    """
    messageModifyPosition Class.
    """
    class_message_name = "messageModifyPosition"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self._position_id = None
        self._stop_loss_price = None
        self._take_profit_price = None

    def read(self, message):
        self._position_id = self.readInt32()
        self._stop_loss_price = self.readDouble()
        self._take_profit_price = self.readDouble()
           
    def write(self):
        self.writeInt32(self._position_id)

    @property
    def position_id(self):
        return self._position_id

    @property
    def stop_loss_price(self):
        return self._stop_loss_price

    @property
    def take_profit_price(self):
        return self._take_profit_price

    def set_position_id(self, position_id):
        self._position_id = position_id

    def set_stop_loss_price(self, stop_loss_price):
        self._stop_loss_price = stop_loss_price

    def set_take_profit_price(self, take_profit_price):
        self._take_profit_price = take_profit_price



messageModifyPosition.register()
            
