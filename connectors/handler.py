# @date 2019-03-29
# @author Frederic SCHERMA
# @license Copyright (c) 2019 SIIS
# Application handler interface


class Handler(object):

    def __init__(self):
        pass

    def init(self, options):
        pass

    def process(self):
        pass

    def terminate(self):
        pass
