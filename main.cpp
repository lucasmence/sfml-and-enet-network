#include <iostream>
#include <list>
#include <string>
#include <SFML/Network.hpp>

int main()
{
	unsigned short clientPort = 54122, serverPort = 54123;
	sf::Clock clock;
	sf::Time timer = clock.getElapsedTime();

	std::cout << "Are you a server [y]? ";
	std::string type = "";
	std::cin >> type;

	if (type != "y")
	{
		std::cout << "Gotta a name? ";
		std::string name = "";
		std::cin >> name;

		std::cout << "Server address? ";
		std::string serverAddress = "";
		std::cin >> serverAddress;

		unsigned short clientPort = 54122;

		sf::UdpSocket socket;

		if (socket.bind(clientPort) == sf::Socket::Done)
			socket.setBlocking(false);
		else
		{
			std::cout << "client socket bind error";
			return 1;
		}

		float duration = 15.f, interval = 0.f;
		clock.restart();

		while (duration > 0.f)
		{
			sf::Time timer = clock.getElapsedTime();
			clock.restart();
			float time = timer.asSeconds();
			duration -= time;

			sf::Packet packetReceived;
			unsigned short receivePort;
			sf::IpAddress sender;
			socket.receive(packetReceived, sender, receivePort);
			std::string senderName = "", messageReceived = "";
			packetReceived >> senderName >> messageReceived;
			if (messageReceived != "")
				std::cout << "Got: " << senderName << " -> " << messageReceived << std::endl;

			interval += time;
			if (interval >= 1.f)
			{
				interval = 0.f;
				sf::Packet packetSend;
				packetSend << name << "Sent hi!";
				socket.send(packetSend, serverAddress, serverPort);
			}
		}
	}
	else
	{
		std::list<sf::IpAddress> clients;
		clients.clear();

		sf::UdpSocket socket;

		if (socket.bind(serverPort) == sf::Socket::Done)
		{
			socket.setBlocking(false);
		}
		else
		{
			std::cout << "server socket bind error";
			return 1;
		}

		float duration = 180.f, interval = 0.f;
		clock.restart();

		std::cout << "server ON";

		while (duration > 0.f)
		{
			sf::Time timer = clock.getElapsedTime();
			clock.restart();
			float time = timer.asSeconds();
			duration -= time;

			sf::Packet packetReceived;
			unsigned short receivePort;
			sf::IpAddress sender;

			socket.receive(packetReceived, sender, receivePort);

			int senderPort = 0;

			std::string senderName = "", messageReceived = "";
			packetReceived >> senderName >> messageReceived;

			if (messageReceived != "")
			{
				auto senderIterator = std::find_if(clients.begin(), clients.end(), [sender](const sf::IpAddress senderIndex) {return senderIndex == sender; });
				if (clients.end() == senderIterator)
					clients.emplace_back(sender);

				std::cout << "Got: " << senderName << " -> " << messageReceived << std::endl;

				sf::Packet packetSend;
				packetSend << senderName << messageReceived;

				for (auto& senderIndex : clients)
					socket.send(packetSend, senderIndex, clientPort);
			}
		}
	}

	return 0;
}
