# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2019 SiiS
# Account Message

from connector.messages.messagecore import messageCore

class messageOrder(messageCore):
    """
    messageOrder Class.
    """
    class_message_name = "messageOrder"

    Event = {
        0: 'UPDATED'
    }

    def __init__(self, message_params):
        super().__init__(self, message_params)
        self.event = 0
        self.timestamp = None
        self.orderId = None
        self.orderRefId = None
        self.marketId = None
        self.orderType = None
        self.direction = None
        self.orderQuantity = None
        self.orderPrice = None
        self.stopLossPrice = None
        self.limitPrice = None
        self.timeInForce = None
        self.priceType = None
        self.postOnly = None
        self.closeOnly = None
        self.tradeId = None
        self.execPrice = None
        self.avgPrice = None
        self.filled = None
        self.cumulativeFilled = None
        self.quoteTransacted = None
        self.commissionAmount = None
        self.commissionAsset = None

    # no need to rewrite 'read' function (not used)

    def write(self):
        super().write()
        self.writeInt8(self.event)
        self.writeDouble(self.timestamp)
        self.writeString(self.orderId)
        self.writeString(self.orderRefId)
        self.writeString(self.marketId)
        self.writeInt8(self.orderType)
        self.writeInt32(self.direction)
        self.writeDouble(self.orderQuantity)
        self.writeDouble(self.orderPrice)
        self.writeDouble(self.stopLossPrice)
        self.writeDouble(self.limitPrice)
        self.writeInt8(self.timeInForce)
        self.writeInt8(self.priceType)
        self.writeInt8(self.postOnly)
        self.writeInt8(self.closeOnly)
        self.writeString(self.tradeId)
        self.writeDouble(self.execPrice)
        self.writeDouble(self.avgPrice)
        self.writeDouble(self.filled)
        self.writeDouble(self.cumulativeFilled)
        self.writeDouble(self.quoteTransacted)
        self.writeDouble(self.commissionAsset)
        self.writeString(self.commissionAsset)             

    def setEvent(self, event):
        self.event = event

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def setOrderId(self, orderId):
        self.orderId = orderId

    def setOrderRefId(self, orderRefId):
        self.orderRefId = orderRefId

    def setMarketId(self, marketId):
        self.marketId = marketId

    def setOrderType(self, orderType):
        self.orderType = orderType

    def setDirection(self, direction):
        self.direction = direction

    def setOrderQuantity(self, orderQuantity):
        self.orderQuantity = orderQuantity

    def setOrderPrice(self, orderPrice):
        self.orderPrice = orderPrice

    def setStopLossPrice(self, stopLossPrice):
        self.stopLossPrice = stopLossPrice

    def setLimitPrice(self, limitPrice):
        self.limitPrice = limitPrice

    def setTimeInForce(self, timeInForce):
        self.timeInForce = timeInForce

    def setPriceType(self, priceType):
        self.priceType = priceType

    def setPostOnly(self, postOnly):
        self.postOnly = postOnly

    def setCloseOnly(self, closeOnly):
        self.closeOnly = closeOnly

    def setTradeId(self, tradeId):
        self.tradeId = tradeId

    def setExecPrice(self, execPrice):
        self.execPrice = execPrice

    def setAvgPrice(self, avgPrice):
        self.avgPrice = avgPrice

    def setFilled(self, filled):
        self.filled = filled

    def setCumulativeFilled(self, cumulativeFilled):
        self.cumulativeFilled = cumulativeFilled

    def setQuoteTransacted(self, quoteTransacted):
        self.quoteTransacted = quoteTransacted

    def setCommissionAmount(self, commissionAmount):
        self.commissionAmount = commissionAmount

    def setCommissionAsset(self, commissionAsset):
        self.commissionAsset = commissionAsset


messageOrder.register()
