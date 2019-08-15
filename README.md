SIIS Strategy/Scalper Indicator Information System Revisited
============================================================

Abstract
--------

SiiS Revisited is a autotrading bot for forex, indices and crypto currencies markets.
It support machine-learning supervisors, and genetic algorithm optimizers.
It also support semi-automated trading in way to manage your entry and exits
with more possibilities than an exchanges allow.

This version is in development, most of the features are not implemented at this time.
Please look at the [github:siis](https://github.com/dream-overflow/siis) for my previous project,
still in developpment, because I use it for now, until the new version will be more advanced, and I can prototype with.

It is mainly developped in C++, using TA-lib and Tensorflow. The connectors are developped in Python3,
but C++ version will be done later. What is important is to have an engine and a strategy in C++,
to have best backtesting performance, low CPU usage and lower latency in live.

The Web client is planned too, developped in ReactJS.

This project is in far from to be usable, lot of work, any help will be appreciate,
and for now I mainly work on the prototype Python version.


Features and TODO list
----------------------

* Initially developped for Linux, but should build on Window or MacOSX
* Traditionnals and crypto markets brokers for trading are supported
    * Binance (Python WIP, margin planned)
    * BitFinex (planned)
    * Bitmex (Python WIP, C++ planned)
    * Degiro (unofficial, planned)
    * Deribit (planned)
    * HitBTC (planned)
    * IB (planned)
    * IG (Python WIP, C++ planned)
* Some others source of prices/volumes data fetchers
   * HistData (only to import manually downloaded files)
   * AlphaVantage (WIP)
   * Tiingo (WIP)
* Distinct instance per account/broker connector
	* Individual configuration
    * Connection with API key (open-source you can check than yours API keys are safe with SiiS)
	* C++ version of the connectors will be realized
* Partial compatibility with SiiS prototype version
	* Same data model
	* Same communication protocol (WIP, mergin)
* Data storage
	* Fetching of OHLC and ticks/trades history data in a PostgreSQL or MySQL database
* Strategies
    * Multiples strategies instances can run at the same time
	* Many markets can run on a same strategy instance
	* Distincts configurations
    * Works on multiple timeframes
    * Common indicators are supported (RSI, SMA, BBANDS, ATR, STOCH, ask if you want more...)
    * Pure signal strategies are possibles in way to only generating some signals/alerts
	* Machine-learning (WIP)
	* Strategy optimizer using algo-G (WIP)
* Fast backtesting (WIP)
* Paper-mode (simulate a broker for spot and margin trading using live market data)
* Live-mode trading on your broker account
* Web client monitor and manager (planned)
	* Display account details and assets quantities
	* Display tickers and markets informations
 	* Display per strategy current (active or pending) trades, trades history and performance
* Interactive command line interface (WIP)
	* More or less similar as Web client features
	* Desktop notification on Linux via dbus
	* Audible notification on Linux via aplay
* Try as possible to take-care of the spread of the market and the commissions
* Compute the average unit price of owned assets on Binance (WIP)
* Pure signal strategies are possibles in way to only generating some signals/alerts (WIP)
* Notifiers (planned)
	* Basic Discord WebHook notifier (planned)
	* Hangout notifier (planned)
* 4 initials strategies
	* Bitcoin/Ethereum (WIP)
	* Altcoins (WIP)
	* Forex (WIP)
	* Indices (WIP)
	* Social copy (postponned)
* Manual per trade directives (WIP)
    * Add many dynamic stop-loss (trigger level + stop price), useful to schedule how to follow the price
    * Many exits conditions to be implemented
* Manual regions of interest per market strategy to help the bot filtering some signals (WIP)
    * Define a region for trade entry|exit|both in long|short|both direction
    * The strategy then can filters signal to only be processed in your regions of interest
    * Actually two type of regions :
        * Range region : parallels horizontals low and high prices
        * Trend channel region : oblics symetrics or asymmetrics low and high trends
    * Auto-expiration after a predefined delay, or after than a trigger price is reached

### Participate ###

Any help is welcome, if you are a Python, Javascrip or C++ devlopper, or a data scientist contact me if your are
interested in participating seriously into this project.

### Donate ###

If this project helped you out feel free to donate.

* BTC: 1GVdwcVrvvbqBgzNMii6tGNhTYnGvsJZFE
* ETH: 0xd9cbda09703cdd4df9aeabf63f23be8da19ca9bf


Installation
------------

Need Python3.6 or Python3.7, GCC or CLANG compiler and CMAKE on your system.
Tested on Debian, Ubuntu and Fedora.

An important C++ dependency is my other project Objective3D. I use it for all
the work I've done on the core module, tons of features I really need.

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

Python support is needed for the connectors.

```
python -m venv siis.venv
```

### Python dependencies ###

```
source siis.venv/bin/activate
pip install -r connectors/deps/requirements.txt
```

Then depending of which database storage to use :

```
pip install -r connectors/deps/reqspgsql.txt  # if using PostgreSQL (recommended)
pip install -r connectors/deps/reqsmysql.txt  # or if using MySQL
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
* /home/\<username>/.siis on Linux based systems
* C:\Users\\<username>\AppData\Local\siis on Windows
* /Users/\<username>/.siis on MacOSX

The directory will contains 4 sub-directories:

* config/ contains important configurations files (described belows)
* log/ contains siis.log the main log and evantually some others logs files (client.log...)
* markets/ contains sub-directories for each configured brokers (detailes belows)
* reports/ contains the reports of the backtesting, per datetime, broker name, 3 files per reports (data.py, report.log, trades.log)

### config ###

#### <.siis>/config/strategy.json ####

Follow the instructions from the file strategies/README.md.


#### <.siis/>config/connectors/ ####

Follow the instructions from the file connectors/README.md.


Running connectors
------------------

Follow the instructions from the file connectors/README.md.


Running strategies
------------------

Follow the instructions from the file strategies/README.md.


About data storage
------------------

The tick or trade data (price, volume) are stored during the running or when fetching data at the tick timeframe.
The OHLC data are stored in the SQL database. But only the 4h, 1D, 1W candle are kept forever :

* Weekly, daily, 4h and 3h OHLC are always kept and store in the SQL DB.
* 2h, 1h and 45m OHLC are kept for 90 days (if the cleaner is executed).
* 30m, 15m, 10m are kept for 21 days.
* 5m, 3m, 1m are kept for 8 days.
* 1s, 10s are never kept.

The cleaner is executed frequently by running instance of SiiS. It is necessary to clean some OHLC, else the DB
will become to big. In addition OHLC are used for live mode, to initially feed the indicators of the strategies,
and to avoid to request the broker API for data history.

Why not requesting the broker API ? Because depending of the broker, but it take lot of time, especially when you have
a lot of markets, it could consumes lot of API call credits, or your are candles count limited like with IG (10k candles per week per account).

About the file containing the ticks, this C++ version could read millions of ticks/trades per seconds, its more performant than any
timestamp based DB engine. I've choosen to have 1 file per month (per market), and the problem is about temporal consistency
of the data. I don't made any check of the timestamp before appending, then fetching could append to a file containing some more recent data,
and maybe with some gaps. For now if I need correct data set, I delete the months of the markets I want to be clean,
and I fetch them completely.

Where it is more problematic its with IG broker, where it's impossible to get history at tick level. So missed data are forever missing.
For this case I realize the backtesting using other dataset. Else you have to run without interuption a connector during many month to have
all the ticks.


Troubles
--------

Fetching historical data is slow : It depends of the exchance and the timeframe. Fetching history trades from BitMex takes a lot of time,
be more patient, this is due to theirs API limitations.

Please understands than I develop this project during my free time, and for free, only your donations could help me.


Disclaimer
----------

The authors are not responsible of the losses on your trading accounts you will made using SiiS Revisited,
nethier of the data loss, corruption, computer crash or physicial dommages on your computers or on the cloud you uses.

The authors are not responsible of the loss due to the lack of the security of your systems.

Use SiiS at your own risk, backtest strategies many time before running them on a live account. Test the stability,
test the efficiency, take in account the potential execution slippage and latency caused by the network, the broker or
by having an inadequate system.
