-module(example_server).
-export([main/0]).


main() ->
    io:format("Server loop start~n"),
    receive
        {From, Msg} ->
            io:format("process ~p consuming ~p from ~p~n", [self(), Msg, From]),
            From ! {"Message: " ++ Msg},
            main()
    end.
