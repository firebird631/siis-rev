Monitoring Web service and trading client
=========================================

Strategies and connectors can send theirs data to this service.
Locally its managed throught a unix file socket or TCP socket, and it offers
a REST API + a WebSocket protocol to communicate with the externals Web clients.

The main goal is to display the status of the connectors, strategies, pending orders,
and actives trades. Also it will be possible to manually creator, cancle or close trades,
and defining some strategies constraints.
