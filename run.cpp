#include<iostream>
#include<fstream>
#include<list>
#include<string>
#include<array>
#include<vector>

std::vector<std::pair<std::string,std::string>> sessionPref;
std::vector<std::pair<std::string,std::string>> otPref;
std::vector<std::pair<std::string,std::string>> updatedPref;
std::string prefString = "";
std::string homeDir = getenv("HOME");
std::string holoDir = homeDir + "/.holo-dl/";
int timeInterval = 120;

std::string parseChannelURL(std::string url)
{
    std::size_t occurrence = url.find_last_of("/");
    std::string channelWithSlash = url.substr(occurrence);

    channelWithSlash.erase(0,1);
    return channelWithSlash;
}

void help()
{
    std::ifstream helpFile("help");
    if (helpFile.is_open()) std::cout << helpFile.rdbuf();
}

void add(std::string channelId, std::string nickname)
{
    std::pair<std::string,std::string> tempPair (channelId,nickname);
    sessionPref.push_back(tempPair);
}

void remove(std::string remArg)
{
    std::string removeId = "";
    try
    {
        removeId = parseChannelURL(remArg);
        for(int i = 0; i < sessionPref.size(); i++)
        {
            if(sessionPref[i].first == removeId) sessionPref.erase(sessionPref.begin()+i);
        }
    }
    catch (...)
    {
        removeId = remArg;
        for(int i = 0; i < sessionPref.size(); i++)
        {
            if(sessionPref[i].second == removeId) sessionPref.erase(sessionPref.begin()+i);
        }
    }

}

void list()
{
    if(sessionPref.empty() && otPref.empty())
    {
        std::cout << "Nothing is in the queue" << std::endl;
    }
    else if(otPref.empty())
    {
        for(std::pair<std::string,std::string> currentPair : sessionPref)
            {
                std::cout << currentPair.first << ", " << currentPair.second << std::endl;
            }
    }
    else if(sessionPref.empty())
    {
        for(std::pair<std::string,std::string> currentPair : otPref)
        {
            std::cout << currentPair.first << ", " << currentPair.second << std::endl;
        }
    }
    else
    {
        std::cout << "Saved queue: " << std::endl;
        for(std::pair<std::string,std::string> currentPair : sessionPref)
        {
            std::cout << currentPair.first << ", " << currentPair.second << std::endl;
        }
        std::cout << "Temporary queue: " << std::endl;
        for(std::pair<std::string,std::string> currentPair : otPref)
        {
            std::cout << currentPair.first << ", " << currentPair.second << std::endl;
        }
    }
}

void start()
{
    std::cout << "Starting..." << std::endl;
}

void setInterval(int newIntervalTime)
{
    if(newIntervalTime < 1)
    {
        std::cout << "Interval time is too low. Please enter an integer value greater than zero." << std::endl;
    }
    else
    {
        timeInterval = newIntervalTime;
    }
}

void includeTemp(std::string channelId, std::string nickname)
{
    std::pair<std::string,std::string> tempPair (channelId, nickname);
    otPref.push_back(tempPair);
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
            try {
                std::string channelId = parseChannelURL(argv[i+1]);
                std::string nickname = argv[i+2];
                add(channelId, nickname);
            }
            catch (...)
            {
            std::cout << "The channel URL you've entered is not valid. Please retry." << std::endl;
            }
        }else if (std::string(argv[i]) == "-r" || std::string(argv[i]) == "--list") {
            std::string remArg = argv[i+1];
            remove(remArg);
        }else if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--list") {
            list();
        }else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--start") {
            start();
        }else if (std::string(argv[i]) == "-i" || std::string(argv[i]) == "--interval") {
            std::string tempInterval = argv[i+1];
            setInterval(std::stoi(tempInterval));
        }else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--temp") {
            try {
                std::string tempChannelId = parseChannelURL(argv[i+1]);
                std::string tempNickname = argv[i+2];
                add(tempChannelId, tempNickname);
            }
            catch (...)
            {
            std::cout << "The channel URL you've entered is not valid. Please retry." << std::endl;
            }
        }

    }

}
