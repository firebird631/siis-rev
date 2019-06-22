# @date 2019-03-23
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

class messageCore(object):
    """
    Message Core Class.
    Is a main class to manage message
    """
    class_message_name = None
    class_message_list = {}
    class_message_default = 'messageCore'

    @classmethod
    def register(cls):
        messageCore.class_message_list[cls.class_message_name] = cls

    @classmethod
    def get_inst(cls, class_message_name):
        return messageCore.class_message_list.get(class_message_name, messageCore.class_message_default)

    def __init__(self, message_params):
        self.message_params = message_params
        self.message = None
        self.pos_msg = 0
        self.idFunc = 0
        self.messageReturn = None
        self.idFuncReturn = 0

    def readCore(self, type_t, size_t):
        (val,) = struct.unpack(type_t, self.message[self.pos_msg:self.pos_msg+size_t])
        self.pos_msg = self.pos_msg + size_t
        return val
    
    def readInt8(self):
        return self.readCore('B', 1)

    def readInt32(self):
        return self.readCore('I', 4)

    def readDouble(self):
        return self.readCore('d', 8)

    def readString(self):
        # read the size of string
        size = readInt32()
        return self.readCore(str(size) + 's', size).decode("utf-8")

    def read(self, message):
        self.pos_msg = 0
        self.message = message
        self.idFunc = self.readInt8()

    def writeCore(self, type_t, val):            
        self.messageReturn = self.messageReturn + struct.pack(type_t, val)

    def writeInt8(self, val):
        self.writeCore('B', val)
    
    def writeInt32(self, val):
        self.writeCore('I', val)

    def writeDouble(self, val):
        self.writeCore('d', val)

    def writeString(self, val):
        self.writeInt32(len(len))
        self.writeCore(str(len(len)) + 's', val)

    def write(self):
        self.messageReturn = b''
        self.writeInt8(self.idFuncReturn)
