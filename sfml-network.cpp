#include <iostream>
#include <list>
#include <string>
#include <SFML/Network.hpp>

enum class ProtocolType {ptUdp, ptTcp};
enum class UserType {utServer, utClient};

class GameSocket 
{
    public:
        ProtocolType protocolType;
		UserType userType;
        sf::UdpSocket socketUdp;
		sf::TcpSocket socketTcp;
		sf::TcpListener listener;
        unsigned short port;
        bool enabled;

        GameSocket(ProtocolType protocolType, UserType userType, unsigned short port)
        {
            this->protocolType = protocolType;
            this->port = port;
            this->enabled = false;
			this->userType = userType;

            switch (this->protocolType)
            {
                case ProtocolType::ptUdp:
                {
                    this->enabled = (this->socketUdp.bind(this->port) == sf::Socket::Done); 
					this->socketUdp.setBlocking(false);
                    break;
                }

                case ProtocolType::ptTcp:
                {
                    this->enabled = this->userType == UserType::utServer ? this->listener.listen(this->port) == sf::Socket::Done : false;
					this->listener.setBlocking(false);
					this->socketTcp.setBlocking(false);
                    break;
                }
            }
        }

        bool sendPackage(sf::Packet &packet, sf::IpAddress &ipAddress, unsigned short portSender)
        {
            switch (this->protocolType)
            {
                case ProtocolType::ptUdp:
                {
                    this->socketUdp.send(packet, ipAddress, portSender);
                    break;
                }

                case ProtocolType::ptTcp:
                {

					if (!this->enabled && this->userType == UserType::utClient)
						this->enabled = this->socketTcp.connect(ipAddress, portSender) == sf::Socket::Status::Done;

					this->socketTcp.send(packet);
                    break;
                }
            }

            return true;
        }

		void receivePackage(sf::Packet &packetReceived, sf::IpAddress &ipAddressSender, unsigned short portSender)
		{

			 switch (this->protocolType)
            {
                case ProtocolType::ptUdp:
                {
					this->socketUdp.receive(packetReceived, ipAddressSender, portSender);
                    break;
                }

                case ProtocolType::ptTcp:
                {
					if (this->userType == UserType::utClient || this->listener.accept(this->socketTcp))
						this->socketTcp.receive(packetReceived);
                    break;
                }
            }
		}
};
int main()
{
	unsigned short clientPort = 54122, serverPort = 54123;
	sf::Clock clock;
	sf::Time timer = clock.getElapsedTime();

	std::cout << "[t]cp or [u]dp? ";
	std::string protocol = "";
	std::cin >> protocol;

	if (protocol != "t" && protocol != "u")
		return 2;

	std::cout << "Are you a server [y]? ";
	std::string type = "";
	std::cin >> type;

	GameSocket gameSocket(protocol == "t" ? ProtocolType::ptTcp : ProtocolType::ptUdp, 
						  type == "y" ? UserType::utServer : UserType::utClient, 
						  type == "y" ? serverPort : clientPort);

	if (gameSocket.userType == UserType::utClient)
	{
		std::cout << "Gotta a name? ";
		std::string name = "";
		std::cin >> name;

		std::cout << "Server address? ";
		std::string serverAddressValue = "";
		std::cin >> serverAddressValue;
		sf::IpAddress serverAddress(serverAddressValue);

		float duration = 15.f, interval = 0.f;
		clock.restart();

		while (duration > 0.f)
		{
			sf::Time timer = clock.getElapsedTime();
			clock.restart();
			float time = timer.asSeconds();
			duration -= time;

			sf::Packet packetReceived;
			sf::IpAddress sender;
			unsigned short receivePort = 54122;

			gameSocket.receivePackage(packetReceived, sender, receivePort);

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
				gameSocket.sendPackage(packetSend, serverAddress, serverPort);
			}
		}
	}
	else
	{
		std::list<sf::IpAddress> clients;
		clients.clear();

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
			sf::IpAddress sender;
			unsigned short receivePort = 54123;

			gameSocket.receivePackage(packetReceived, sender, receivePort);

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
					gameSocket.sendPackage(packetSend, senderIndex, clientPort);
			}
		}
	}

	return 0;
}
