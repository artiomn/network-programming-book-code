%%% The echo module provides a simple TCP echo server. Users can telnet
%%% into the server and the sever will echo back the lines that are input
%%% by the user.

%%% Author: Henrik Treadup
%%% Origin: https://github.com/treadup/erlang-echo-server/blob/master/echo.erl

-module(echo).
-export([start/1]).

%% Starts an echo server listening for incoming connections on
%% the given Port.
start(Port) ->
    {ok, Socket} = gen_tcp:listen(Port, [binary, {active, true}, {packet, line}, {reuseaddr, true}]),
    io:format("Echo server listening on port ~p~n", [Port]),
    server_loop(Socket).


%% Accepts incoming socket connections and passes then off to a separate Handler process
server_loop(Socket) ->
    {ok, Connection} = gen_tcp:accept(Socket),
    Handler = spawn(fun() -> echo_loop(Connection) end),
    gen_tcp:controlling_process(Connection, Handler),
    io:format("New connection ~p~n", [Connection]),
    server_loop(Socket).


%% Echoes the incoming lines from the given connected client socket
echo_loop(Connection) ->
    receive
        {tcp, Connection, Data} ->
	    gen_tcp:send(Connection, Data),
	    echo_loop(Connection);
	{tcp_closed, Connection} ->
	    io:format("Connection closed ~p~n", [Connection])
    end.
