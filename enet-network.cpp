#include <enet/enet.h>
#include <iostream>
#include <chrono>

int main() 
{
    
    if (enet_initialize() != 0) 
    {
        std::cerr << "ENet startup error.\n";
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    std::cout << "are you a server? [y]\n";
    std::string isServer = "";
    std::cin >> isServer;

    if (isServer == "y")
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = 54123;

        ENetHost* server;
        server = enet_host_create(&address, 32, 2, 0, 0);

        if (!server) 
        {
            std::cerr << "Error ENet server startup\n";
            return EXIT_FAILURE;
        }

        std::cout << "server ON\n";

        ENetEvent event;
        while (true) 
        {
            while (enet_host_service(server, &event, 1000) > 0) 
            {
                switch (event.type)
                {
                    case ENET_EVENT_TYPE_CONNECT:
                    {
                        std::cout << "Client connection: " << event.peer->address.host << ":" << event.peer->address.port << ".\n";

                        event.peer->data = (void*)"Client connected";

                        break;
                    }
                    case ENET_EVENT_TYPE_RECEIVE:
                    {
                        std::cout << "Client package received " << (char*)event.peer->data << ": '" << (char*)event.packet->data << "'\n";

                        enet_packet_destroy(event.packet);

                        const char* message = "Hi client!";
                        ENetPacket* packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(event.peer, 0, packet);
                        enet_host_flush(server);

                        break;
                    }

                    case ENET_EVENT_TYPE_DISCONNECT:
                    {
                        std::cout << (char*)event.peer->data << " desconnected.\n";

                        event.peer->data = nullptr;

                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
            }
        }

        enet_host_destroy(server);
    }
    else
    {
        ENetHost* client;
        client = enet_host_create(nullptr, 1, 2, 0, 0);
        if (!client) 
        {
            std::cerr << "ENet client startup error.\n";
            return EXIT_FAILURE;
        }

        ENetAddress address;
        enet_address_set_host(&address, "mence.ddns.net");
        address.port = 54123;

        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();

        float interval = 0.f;

        while (true)
        {
            std::chrono::duration<double> duration = end - start;

            end = std::chrono::high_resolution_clock::now();

            interval += duration.count();

            if (interval >= 1.f)
            {
                interval = 0.f;
                ENetPeer* peer;
                peer = enet_host_connect(client, &address, 2, 0);
                if (peer == nullptr)
                {
                    std::cerr << "Server connection failed.\n";
                    enet_host_destroy(client);
                    //return EXIT_FAILURE;
                }

                ENetEvent event;
                if (enet_host_service(client, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
                {
                    std::cout << "Connected to the server.\n";

                    const char* message = "Hi server!";
                    ENetPacket* packet = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(peer, 0, packet);
                    enet_host_flush(client);
                }
                else
                {
                    std::cerr << "Connection failed.\n";

                    enet_peer_reset(peer);
                    enet_host_destroy(client);
                    //return EXIT_FAILURE;
                }

                while (enet_host_service(client, &event, 10000) > 0)
                {
                    switch (event.type)
                    {
                    case ENET_EVENT_TYPE_RECEIVE:
                    {
                        std::cout << "Server response: " << (char*)event.packet->data << "\n";
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT:
                    {
                        std::cout << "Disconnected from server.\n";
                        //return EXIT_SUCCESS;
                    }
                    }
                }

                enet_peer_disconnect(peer, 0);
                enet_host_flush(client);

                while (enet_host_service(client, &event, 10000) > 0)
                {
                    switch (event.type)
                    {
                        case ENET_EVENT_TYPE_RECEIVE:
                        {
                            enet_packet_destroy(event.packet);
                            break;
                        }

                        case ENET_EVENT_TYPE_DISCONNECT:
                        {
                            std::cout << "Successfully disconnected.\n";
                            //return EXIT_SUCCESS;
                        }

                    }
                }

                enet_peer_reset(peer);

            }
        }

        enet_host_destroy(client);

    }

    return EXIT_SUCCESS;
    
}
