SIIS Strategy/Scalper Indicator Information System Revisited
============================================================

**The redaction of this document is in progress.**

Abstract
--------

SiiS Revisited is a autotrading bot for forex, indices and crypto currencies markets.
It support machine-learning supervisors, and genetic algorithm optimizers.
It also support semi-automated trading in way to manage your entry and exits
with more possibilities than an exchanges allow.

This version is more robust, performant and evolved version the the prototype Python3 version
It is mainly developped in C++, using TA-lib and Tensorflow. The connectors are developped in Python3,
but some C++ version could be done in the futur.

A webclient will be developped in ReactJS or AngularJS.


Features
--------

* Initially developped for Linux, but should build on Window or MacOSX
* Traditionnals and crypto markets brokers (traders) are supported
    * Binance
    * HitBTC (planned)
    * Bitmex
    * IG
    * BitFinex (planned)
    * Degiro (unofficial, planned)
    * IB (planned)
* Most of the SiiS prototype version features
* ...

### Donation ###

* BTC: 1GVdwcVrvvbqBgzNMii6tGNhTYnGvsJZFE
* ETH: 0xd9cbda09703cdd4df9aeabf63f23be8da19ca9bf


Installation
------------

Need Python3.6 or Python3.7, GCC or CLANG compiler and CMAKE on your system.
Tested on Debian, Ubuntu and Fedora.

An important C++ dependency is my other project Objective3D. I use it for all
the work I've done on the core module (not for the rest of course).
Its like QtCore or a boost library, tons of features I really need.

There is more details on the strategy/ directory.

### Create a C++ virtual env ###

Follow the instructions contained in file cmake/README.
And then activate it before building.

Then build and install TA-Lib :

```
cd third/ta-lib
./configure
make
make install
```

Eventually to exit of the environment (or open another terminal) :

(this deactivate bash function is not implemanted for now)

```
deactivate
```

### Create a PIP virtual env ###

```
python -m venv siis.venv
```

### Python dependencies ###

```
source siis.venv/bin/activate
pip install -r requirements.txt
```

Eventually to exit of the environment :

```
deactivate
```


### Database ###

Prefers the PostgreSQL database server. For now SiiS does not bulk data insert, the performance
with PostgreSQL are OK, but lesser on MySQL.

The sql/ directory contains the SQL script for the two databases and the first line of comment
in these files describe a possible way to install them.

The PostgreSQL support will be the priority. MySQL is postponed.


### Cache ###

Redis is used for communication and data cache. You need then a configured Redis server.
The default configuration will suffise.


Configuration
-------------

First running will try to create a data structure on your local user.
* /home/<username>/.siis on Linux based systems
* C:\Users\<username\AppData\Local\siis on Windows
* /Users/<username>/.siis on MacOSX

The directory will contains 4 sub-directories:

* config/ contains important configurations files (described belows)
* log/ contains siis.log the main log and evantually some others logs files (client.log...)
* markets/ contains sub-directories for each configured brokers (detailes belows)
* reports/ contains the reports of the backtesting, per datetime, broker name, 3 files per reports (data.py, report.log, trades.log)

### config ###

#### <.siis/config/>strategy.json ####

Follow the instructions from the file strategies/README.


#### <.siis/>connectors/ ####

Follow the instructions from the file connectors/README.


Running connectors
------------------

Follow the instructions from the file connectors/README.


Running strategies
------------------

Follow the instructions from the file strategies/README.


Troubles
--------

...


Disclaimer
----------

The authors are not responsible of the losses on your trading accounts you will made using SiiS Revisited,
nethier of the data loss, corruption, computer crash or physicial dommages on your computers or on the cloud you uses.

The authors are not responsible of the loss due to the lack of the security of your systems.

Use SiiS at your own risk, backtest strategies many time before running them on a live account. Test the stability,
test the efficiency, take in account the potential execution slippage and latency caused by the network, the broker or
by having an inadequate system.
