-module(example_server).
-export([loop/0]).

loop() ->
    receive
	{From, Msg} ->
	    From ! Msg,
	    loop()
end.
