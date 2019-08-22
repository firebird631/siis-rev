# main and shared siis library

set(LIB_NAME siis)
set(EXEC_NAME strategy)

set(STRATEGY_LIB_CXX
    collection.cpp
    dataarray.cpp
    market.cpp
    poolworker.cpp
    strategy.cpp
    terminal.cpp
    worker.cpp
    analysers/analyser.cpp
    analysers/stdanalyser.cpp
    backtest/backtest.cpp
    cache/cache.cpp
    cache/redis/rediscache.cpp
    cache/redis/redistradecache.cpp
    config/config.cpp
    config/jsonparser.cpp
    config/strategyconfig.cpp
    connector/connector.cpp
    connector/localconnector.cpp
    connector/zmqconnector.cpp
    connector/traderproxy.cpp
    connector/messages/connectormessagecore.cpp
    connector/messages/connectormessagesubscribe.cpp
    connector/messages/connectormessageunsubscribe.cpp
    connector/messages/connectormessagereceiveohlc.cpp
    connector/messages/connectormessagereceiveohlcarray.cpp
    connector/messages/connectormessagereceivetick.cpp
    connector/messages/connectormessagereceivetickaggreged.cpp
    connector/messages/connectormessagereceivetickarray.cpp
    connector/messages/connectormessagesignals.cpp
    connector/messages/connectormessagecancelorder.cpp
    connector/messages/connectormessagecloseposition.cpp
    connector/messages/connectormessagecreateorder.cpp
    connector/messages/connectormessagelistorders.cpp
    connector/messages/connectormessagelistpositions.cpp
    connector/messages/connectormessagemodifyposition.cpp
    database/database.cpp
    database/marketdb.cpp
    database/ohlcdb.cpp
    database/ohlcstream.cpp
    database/tickstream.cpp
    database/tradedb.cpp
    database/mysql/mysql.cpp
    database/mysql/mysqltradedb.cpp
    database/mysql/mysqlmarketdb.cpp
    database/mysql/mysqlohlcdb.cpp
    database/pgsql/pgsql.cpp
    database/pgsql/pgsqltradedb.cpp
    database/pgsql/pgsqlmarketdb.cpp
    database/pgsql/pgsqlohlcdb.cpp
    display/displayer.cpp
    display/ncursesdisplayer.cpp
    display/ttydisplayer.cpp
    indicators/atr/atr.cpp
    indicators/bbands/bbands.cpp
    indicators/bbawe/bbawe.cpp
    indicators/donchian/donchian.cpp
    indicators/ema/ema.cpp
    indicators/hma/hma.cpp
    indicators/macd/macd.cpp
    indicators/mama/mama.cpp
    indicators/mmt/mmt.cpp
    indicators/price/price.cpp
    indicators/pivotpoint/pivotpoint.cpp
    indicators/rsi/rsi.cpp
    indicators/sar/sar.cpp
    indicators/sinewave/sinewave.cpp
    indicators/sma/sma.cpp
    indicators/stoch/stoch.cpp
    indicators/stochrsi/stochrsi.cpp
    indicators/td9/td9.cpp
    indicators/triangle/triangle.cpp
    indicators/volume/volume.cpp
    indicators/vwma/vwma.cpp
    indicators/wma/wma.cpp
    indicators/zigzag/zigzag.cpp
    learning/learning.cpp
    learning/optimizer.cpp
    learning/stdsupervisor.cpp
    learning/supervisor.cpp
    live/live.cpp
    optimization/optimization.cpp
    trade/assettrade.cpp
    trade/indmargintrade.cpp
    trade/margintrade.cpp
    trade/stdtrademanager.cpp
    trade/trade.cpp
    trade/tradeoperation.cpp
    monitor/monitor.cpp
    monitor/redismonitor.cpp
    utils/common.cpp
    utils/ohlcgen.cpp)

#add_definitions(-fPIC)

set(EXEC_LIB_CXX
    collection.cpp
    main.cpp
    poolworker.cpp
    worker.cpp
    backtest/backtest.cpp
    config/config.cpp
    config/jsonparser.cpp
    config/strategyconfig.cpp
    learning/learning.cpp
    live/live.cpp
    optimization/optimization.cpp)
    #utils/common.cpp)

# target_include_directories(${LIB_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(O3D_LIB_EXT "-dbg")
elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
    set(O3D_LIB_EXT "-odbg")
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(O3D_LIB_EXT "")
endif()

add_library(${LIB_NAME} SHARED ${STRATEGY_LIB_CXX})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${LIB_NAME}
        pthread
        rt
        pq  # ${PostgreSQL_LIBRARIES}
        o3dmysql${O3D_LIB_EXT}
        hiredis
        # o3dpgsql${O3D_LIB_EXT}
        ${OBJECTIVE3D_LIBRARY}
        ${TA_LIBRARIES})
endif()

add_executable(strategy ${EXEC_LIB_CXX})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${EXEC_NAME}
    	${LIB_NAME}
        pthread
        rt
        ncurses
        pq  # ${PostgreSQL_LIBRARIES}
        hiredis
        ${OBJECTIVE3D_LIBRARY}
        ${ZMQ_LIBRARIES}
        ${JSONCPP_LIBRARIES}
        ${TA_LIBRARIES})
endif()

install (TARGETS ${LIB_NAME}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
    COMPONENT library)

install (TARGETS ${EXEC_NAME}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
    COMPONENT library)

# strategies & supervisors

add_subdirectory(strategies)
add_subdirectory(supervisors)

