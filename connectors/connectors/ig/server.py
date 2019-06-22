# @date 2019-03-04
# @author Romain Locci
# @license Copyright (c) 2018 SIIS
# server for connectors communications

import os
from server import ConnectorServer
from collection.ig.connector import Connector

class IgConnectorServer(ConnectorServer):
        """
        Connector Server Class for IG connector.
        """

        def __init__(self, config_file):
            # !! this line in each connector
            ConnectorServer.__init__(
                                     self,
                                     "ig.com",
                                     config_file,
                                     Connector()
                                     )

        pass
        
