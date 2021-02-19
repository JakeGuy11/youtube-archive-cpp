#include<iostream>
#include<fstream>

void help()
{
    std::ifstream helpFile("help");
    if (helpFile.is_open()) std::cout << helpFile.rdbuf();
}

void add(std::string channelId, std::string nickname)
{
    std::cout << "Channel URL: " << channelId << std::endl
              << "Nickname: " << nickname << std::endl;
}

void remove(std::string channelId, std::string nickname)
{
    std::cout << "Channel URL: " << channelId << std::endl
              << "Nickname: " << nickname << std::endl;
}

void list()
{
    std::cout << "Print all Channel URLs and Nicknames here" << std::endl;
}

void start()
{
    std::cout << "Starting..." << std::endl;
}

void setInterval(int newIntervalTime)
{
    std::cout << "Set interval time to " << newIntervalTime << std::endl;
}

void includeTemp(std::string channelId)
{
    std::cout << "Temporarily added " << channelId << " to queue" << std::endl;
}

int main(int argc, char **argv)
{

    if(argc < 2)
    {
    std::cerr << "Usage: " << std::endl
        << argv[0] << " --start" << std::endl
        << argv[0] << " --help" << std::endl
        << argv[0] << " --list" << std::endl;
    return 1;
    }

    for(int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            help();
        }else if (std::string(argv[i]) == "-a" || std::string(argv[i]) == "--add") {
            std::string channelId = argv[i+1];
            std::string nickname = argv[i+2];
            add(channelId, nickname);
        }else if (std::string(argv[i]) == "-r" || std::string(argv[i]) == "--list") {
            std::string channelId = argv[i+1];
            std::string nickname = argv[i+2];
            remove(channelId, nickname);
        }else if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--list") {
            list();
        }else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--start") {
            start();
        }else if (std::string(argv[i]) == "-i" || std::string(argv[i]) == "--interval") {
            std::string tempInterval = argv[i+1];
            setInterval(std::stoi(tempInterval));
        }else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--temp") {
            std::string tempChannelId = argv[i+1];
            includeTemp(tempChannelId);
        }

    }

}
