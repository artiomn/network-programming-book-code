#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>


int main(int argc, char **argv)
{
    if (SDLNet_Init() < 0)
    {
        SDL_Log("SDLNet_Init() failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i)
    {
        SDL_Log("Looking up %s ...", argv[i]);
        IPaddress addr;
        if (-1 == SDLNet_ResolveHost(&addr, argv[i], 80))
        {
            SDL_Log("Failed to lookup %s: %s", argv[i], SDL_GetError());
        }
        else
        {
            SDL_Log("%s is %s", argv[i], SDLNet_ResolveIP(&addr));
            char *req = nullptr;
            SDL_asprintf(&req, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", argv[i]);
            auto *sock = SDLNet_TCP_Open(&addr);
            if (!sock)
            {
                SDL_Log("Error socket opening [%s]", SDL_GetError());
            }
            if (!req)
            {
                SDL_Log("Out of memory!");
            }
            else if (!sock)
            {
                SDL_Log("Failed to create stream socket to %s: %s\n", argv[i], SDL_GetError());
            }
            else if (SDLNet_TCP_Send(sock, req, SDL_strlen(req)) < 0)
            {
                SDL_Log("Failed to write to %s: %s", argv[i], SDL_GetError());
            }
            else
            {
                char buf[512];
                int br;
                while ((br = SDLNet_TCP_Recv(sock, buf, sizeof(buf))) >= 0)
                {
                    fwrite(buf, 1, br, stdout);
                }

                printf("\n\n\n%s\n\n\n", SDL_GetError());
                fflush(stdout);
            }

            if (sock)
            {
                SDLNet_TCP_Close(sock);
            }

            SDL_free(req);
        }
    }


    SDLNet_Quit();

    return EXIT_SUCCESS;
}
