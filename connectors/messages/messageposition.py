# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messagePosition(messageCore):
    """
    messagePosition Class.
    """
    class_message_name = "messagePosition"

    Event = {
        0: 'UPDATED'
    }

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.event = 0
        self.timestamp = None
        self.positionId = None
        self.marketId = None
        self.direction = None
        self.quantity = None
        self.avgPrice = None
        self.execPrice = None
        self.stopLossPrice = None
        self.limitPrice = None
        self.profitCurrency = None
        self.profitLoss = None
        self.filled = None
        self.cumulativeFilled = None
        self.liquidationPrice = None
        self.commission = None            

    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeInt8(self.event)
        self.writeDouble(self.timestamp)
        self.writeString(self.positionId)
        self.writeString(self.marketId)
        self.writeInt32(self.direction)
        self.writeDouble(self.quantity)
        self.writeDouble(self.avgPrice)
        self.writeDouble(self.execPrice)
        self.writeDouble(self.stopLossPrice)
        self.writeDouble(self.limitPrice)
        self.writeString(self.profitCurrency)
        self.writeDouble(self.profitLoss)
        self.writeDouble(self.filled)
        self.writeDouble(self.cumulativeFilled)
        self.writeDouble(self.liquidationPrice)
        self.writeDouble(self.commission)

    def setEvent(self, event):
        self.event = event

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def setPositionId(self, positionId):
        self.positionId = positionId

    def setMarketId(self, marketId):
        self.marketId = marketId

    def setDirection(self, direction):
        self.direction = direction

    def setQuantity(self, quantity):
        self.quantity = quantity

    def setAvgPrice(self, avgPrice):
        self.avgPrice = avgPrice

    def setexecPrice(self, execPrice):
        self.execPrice = execPrice

    def setstopLossPrice(self, stopLossPrice):
        self.stopLossPrice = stopLossPrice

    def setLimitPrice(self, limitPrice):
        self.limitPrice = limitPrice

    def setProfitCurrency(self, profitCurrency):
        self.profitCurrency = profitCurrency

    def setProfitLoss(self, profitLoss):
        self.profitLoss = profitLoss

    def setFilled(self, filled):
        self.filled = filled

    def setCumulativeFilled(self, cumulativeFilled):
        self.cumulativeFilled = cumulativeFilled

    def setLiquidationPrice(self, liquidationPrice):
        self.liquidationPrice = liquidationPrice

    def setCommission(self, commission):
        self.commission = commission


messagePosition.register()
