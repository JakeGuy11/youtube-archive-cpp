#include<iostream>
#include<filesystem>
#include<cstdio>
#include<fstream>
#include<vector>
#include<sstream>
#include<chrono>
#include<thread>
#include<functional>
#include<future>
#include <memory>
#include <stdexcept>
#include <array>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

std::vector<std::pair<std::string,std::string>> sessionPref;
std::vector<std::pair<std::string,std::string>> otPref;
std::vector<std::future<void>> procVector;
std::string prefString = "";
std::string homeDir = getenv("HOME");
std::string holoDir = homeDir + "/.holo-dl/";
std::string holoPref = homeDir + "/.holo-dl/.queue";
int timeInterval = 5;

std::string parseChannelURL(std::string url)
{
    std::size_t occurrence = url.find_last_of("/");
    std::string channelWithSlash = url.substr(occurrence);

    channelWithSlash.erase(0,1);
    return channelWithSlash;
}

std::vector<std::string> parsePythonOutput(std::string const str)
{
    std::stringstream tempStream(str);
    std::string foundString;
    std::vector<std::string> tempVec;

    while (std::getline(tempStream, foundString, ';')) {
        tempVec.push_back(foundString);
    }
    return tempVec;
}

std::vector<std::pair<std::string,std::string>> splitIntoVector(std::string const &str)
{
    std::vector<std::pair<std::string,std::string>> returnVector;

    std::stringstream tempStream(str);
    std::string foundString;

    std::vector<std::string> tempVec;
    std::pair<std::string,std::string> tempPair;

    while (std::getline(tempStream, foundString, '|')) {
        tempVec.push_back(foundString);
    }

    bool added = false;

    for(int i = 0; i < tempVec.size(); i++)
    {
        std::stringstream tempStream(tempVec[i]);
        while (std::getline(tempStream, foundString, ';')) {
            if(added)
            {
                tempPair.second = foundString;
                added = false;
            }
            else
            {
                tempPair.first = foundString;
                added = true;
            }
        }
        returnVector.push_back(tempPair);
    }

    return returnVector;

}

std::string saveToString(std::vector<std::pair<std::string,std::string>> vectorToConvert)
{

    std::string returnString;

    for(int i = 0; i < vectorToConvert.size(); i++)
    {
        if(i == 0)
        {
            returnString = vectorToConvert[i].first + ";" + vectorToConvert[i].second;
        }
        else
        {
            returnString += "|" + vectorToConvert[i].first + ";" + vectorToConvert[i].second;
        }
    }
    return returnString;
}

std::string readFromFile(std::string filePath)
{
    std::ifstream ifs(filePath);
    std::string fileContents( (std::istreambuf_iterator<char>(ifs) ),
                       (std::istreambuf_iterator<char>()    ) );
    return fileContents;
}

void writeToFile(std::string filePath, std::string stringToWrite)
{
    const char *filePathChars = filePath.c_str();
    remove(filePathChars);
    std::ofstream outFile(filePath);
    outFile << stringToWrite;
    outFile.close();
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

void removeEntry(std::string remArg)
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

bool fileExists(const std::string& fileName)
{
    std::ifstream f(fileName.c_str());
    return f.good();
}

std::string getCommandOutput(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void startArchive(std::string youtubeURL, std::string saveName, std::string activityName)
{
    //std::cout << "savename: " << saveName << std::endl;
    std::string activityFilePath = holoDir + "." + activityName;
    //std::cout << activityFilePath << std::endl;
    std::ofstream outFile(activityFilePath);
    outFile << "active" << std::endl;
    outFile.close();
    std::string command = "ffmpeg -n -loglevel quiet -i `youtube-dl -f best -g " + youtubeURL + "` " + holoDir + saveName; //-loglevel quiet
    //std::cout << command << std::endl;
    system(command.c_str());
    const char *activityFilePathChars = activityFilePath.c_str();
    remove(activityFilePathChars);
    std::cout << activityName << " stream ended or cancelled" << std::endl;
}

void periodic()
{
    std::cout << "\nStarting checks..." << std::endl;
    for(int i = 0; i < sessionPref.size(); i++)
    {
        if(fileExists(holoDir + "." + sessionPref[i].second))
        {
            std::cout << sessionPref[i].second + " activity file exists, stream is being archived" << std::endl;
        }
        else
        {
            std::cout << sessionPref[i].second + " activity file doesn't exist, checking for stream..." << std::endl;
            std::string arguments = "./parse_youtube_data.py " + sessionPref[i].first + " " + sessionPref[i].second + " date";
            //std::cout << arguments << std::endl;
            std::string pythonOut = getCommandOutput(arguments.c_str());
            //std::cout << pythonOut << std::endl;
            std::vector<std::string> parsedPython = parsePythonOutput(pythonOut);
            try
            {
                //std::future<void> result( std::async(startArchive, parsedPython[0], parsedPython[1], sessionPref[i].second));
                procVector.push_back(std::async(startArchive, parsedPython[0], parsedPython[1], sessionPref[i].second));
                //startArchive(parsedPython[0], parsedPython[1], sessionPref[i].second);
            }
            catch (...)
            {
                std::cout << sessionPref[i].second << " stream does not exist or has not started" << std::endl;
            }
        }
    }
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

    sessionPref = splitIntoVector(readFromFile(holoPref));

    bool run = false;

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
            removeEntry(remArg);
        }else if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--list") {
            list();
        }else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--start") {
            run = true;
        }else if (std::string(argv[i]) == "-i" || std::string(argv[i]) == "--interval") {
            std::string tempInterval = argv[i+1];
            setInterval(std::stoi(tempInterval));
        }else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--temp") {
            try {
                std::string tempChannelId = parseChannelURL(argv[i+1]);
                std::string tempNickname = argv[i+2];
                includeTemp(tempChannelId, tempNickname);
            }
            catch (...)
            {
            std::cout << "The channel URL you've entered is not valid. Please retry." << std::endl;
            }
        }

    }
    writeToFile(holoPref, saveToString(sessionPref));

    if(run)
    {
        int loopCount = 0;
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if(loopCount >= (timeInterval * 1000)/50)
            {
                periodic();
                loopCount = 0;
            }
            else
            {
            loopCount++;
            }
        }
    }

}
