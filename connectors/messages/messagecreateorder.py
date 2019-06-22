# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

from connector.messages.messagecore import messageCore

class messageCreateOrder(messageCore):
    """
    messageCreateOrder Class.
    """
    class_message_name = "messageCreateOrder"

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self._order_id = None
        self._market_id = None
        self._direction = None
        self._order_type = None
        self._order_price = None
        self._quantity = None
        self._stop_loss_price = None
        self._take_profit_price = None
        self._leverage = None

    def read(self, message):
        self._market_id = self.readString()
        self._direction = self.readInt32()
        self._order_type = self.readInt8()
        self._order_price = self.readDouble()
        self._quantity = self.readDouble()
        self._stop_loss_price = self.readDouble()
        self._take_profit_price = self.readDouble()
        self._leverage = self.readDouble()
           
    def write(self):
        self.writeInt32(self._order_id)

    @property
    def market_id(self):
        return self._market_id

    @property
    def direction(self):
        return self._direction

    @property
    def order_type(self):
        return self._order_type

    @property
    def order_price(self):
        return self._order_price

    @property
    def quantity(self):
        return self._quantity

    @property
    def stop_loss_price(self):
        return self._stop_loss_price

    @property
    def take_profit_price(self):
        return self._take_profit_price

    @property
    def leverage(self):
        return self._leverage

    def set_order_id(self, order_id):
        self._order_id = order_id


messageCreateOrder.register()
