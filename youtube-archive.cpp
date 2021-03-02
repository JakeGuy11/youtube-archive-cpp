#include<iostream>
#include<fstream>
#include<vector>
#include<sstream>
#include<chrono>
#include<thread>
#include<future>
#include<algorithm>
#include<cstring>

//Create a vector of pairs of strings to hold our saved and added download entries
std::vector<std::pair<std::string,std::string>> sessionQueue;
//Create a vector of pairs of strings to hold our temporary download queue
std::vector<std::pair<std::string,std::string>> otQueue;
//Create a vector of async processes so we can have multiple running at once
std::vector<std::future<void>> procVector;
//Create a string to hold the home directory (~/)
std::string homeDir = getenv("HOME");
//Append our working directory so we don't have to add it every time
std::string archiveDir = homeDir + "/.archive-dl/";
//Append our queue file so we don't have to add it every time
std::string archiveQueue = archiveDir + ".queue";
//Set the default periodic interval time
int timeInterval = 45;
//Set the default run state to do not run
bool run = false;
//Set the default video format
std::string finalFormat = "ts";
//Set the final destination of the streams
std::string moveLocation = "";

//Create a method that returns the channel id from its url
std::string parseChannelURL(std::string url)
{
    //Find where the last / is, it will be followed by the id
    std::size_t occurrence = url.find_last_of("/");
    //Take only the last part of the url, which will be /id
    std::string channelWithSlash = url.substr(occurrence);
    //Remove from the slash from the id
    channelWithSlash.erase(0,1);
    //Return the id
    return channelWithSlash;
}

//Parse the python output, which will be formatted as url;saveName
std::vector<std::string> parsePythonOutput(std::string const pythonOut)
{
    //Turn our python output into a stringstream so we can use getline on it
    std::stringstream tempStream(pythonOut);
    //Create a temporary string that will store each part of the returned string
    std::string foundString;
    //Create a vector for us to return
    std::vector<std::string> retVec;

    //This will loop until the end of the string and will add foundString to the vector every time it finds the regex
    while (std::getline(tempStream, foundString, ';')) {
        //Add the part of the string to our return vector
        retVec.push_back(foundString);
    }
    //Return the return vector
    return retVec;
}

//Parse the queue string, searching for '|' and ';' to separate the elements
std::vector<std::pair<std::string,std::string>> splitIntoVector(std::string const &str)
{
    //Create a vector of pairs of strings that will hold the queue of channel ids and nicknames
    std::vector<std::pair<std::string,std::string>> returnVector;
    //Create a stringstream from the string to be parsed
    std::stringstream tempStream(str);
    //Create a blank string for our formatted pairs
    std::string foundString;
    //Create a blank vector to hold all of our formatted pairs
    std::vector<std::string> tempVec;
    //Create a temporary pair which will be pushed onto the return vector
    std::pair<std::string,std::string> tempPair;

    //Search the string for the element separator, '|'
    while (std::getline(tempStream, foundString, '|')) {
        //Add url;name to the back of the temporary vector
        tempVec.push_back(foundString);
    }

    //This will track whether we're on the first element of the pair or second
    bool added = false;

    //For each element of the temporary vector holding strings of url;name
    for(int i = 0; i < tempVec.size(); i++)
    {
        //Create a stringstream from the current element of the vector
        std::stringstream tempStream(tempVec[i]);
        //Split the formatted pair by the known separator, ';'
        while (std::getline(tempStream, foundString, ';')) {
            //If we've already added the first element
            if(added)
            {
                //This will be the name (after the ';'), so add it to the second element of the pair
                tempPair.second = foundString;
                //The next thing we'll be adding to is the first element, so set added to false
                added = false;
            }
            //We haven't added the first element
            else
            {
                //Add the url to the first element of the pair
                tempPair.first = foundString;
                //We've added the first element, so next time we want to add it to the second
                added = true;
            }
        }
        //Add the completed pair to the return vector
        returnVector.push_back(tempPair);
    }
    //Return the vector of pairs
    return returnVector;
}

//Convert our vector of pairs of to a string that is readable from splitIntoVector()
std::string saveToString(std::vector<std::pair<std::string,std::string>> vectorToConvert)
{
    //Create a string to return
    std::string returnString;
    //For every pair
    for(int i = 0; i < vectorToConvert.size(); i++)
    {
        //If it's the first one
        if(i == 0)
        {
            //We don't want a | at the start or else we'll have a blank pair at the beginning
            //Other than that, add the first element, ';', and the second
            returnString = vectorToConvert[i].first + ";" + vectorToConvert[i].second;
        }
        //It's not the first element
        else
        {
            //Add the element separating character '|', then the first pair element, then ';' then the second pair element
            returnString += "|" + vectorToConvert[i].first + ";" + vectorToConvert[i].second;
        }
    }
    //Return the generated string
    return returnString;
}

//Grab the entire contents of a file
std::string readFromFile(std::string filePath)
{
    //Create an input file stream from the given path
    std::ifstream ifs(filePath);
    //Get the contents of the file
    std::string fileContents( (std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>()));
    //Return the contents
    return fileContents;
}

//Write a string to a file
void writeToFile(std::string filePath, std::string stringToWrite)
{
    //Convert the path to a char array
    const char *filePathChars = filePath.c_str();
    //Delete the file
    remove(filePathChars);
    //Create an outfile stream from our filepath
    std::ofstream outFile(filePath);
    //Write the string to the file
    outFile << stringToWrite;
    //Close the file
    outFile.close();
}

//Print the help page
void help()
{
    //Create an infile stream from the file named help
    std::ifstream helpFile("/opt/youtube-archive/help");
    //Print the contents of help
    std::cout << helpFile.rdbuf();
}

//Add an entry to the queue
void add(std::string channelId, std::string nickname)
{
    //Create a pair from the two arguments passed
    std::pair<std::string,std::string> tempPair (channelId,nickname);
    //Add that pair to the session queue
    sessionQueue.push_back(tempPair);
}

//Remove an entry from the queue
void removeEntry(std::string remArg)
{
    //Create a blank string that'll hold the removal identifier
    std::string removeId = "";
    //We don't know if it's the nickname or id, so try parsing it as a channel url
    try
    {
        //Parse it as a channel url
        //If it's not a url, it'll throw an error
        removeId = parseChannelURL(remArg);
        //For every element in our queue vector
        for(int i = 0; i < sessionQueue.size(); i++)
        {
            //If the channel id of this element matches the one that should be removed, remove it
            if(sessionQueue[i].first == removeId) sessionQueue.erase(sessionQueue.begin()+i);
        }
    }
    //It's not a parsable url, so check the channel nicknames
    catch (...)
    {
        //The removal identifier will be the same as the argument passed
        removeId = remArg;
        //For every element in our queue vector
        for(int i = 0; i < sessionQueue.size(); i++)
        {
            //If the name in this element of the vector matches the removal id, remove the element
            if(sessionQueue[i].second == removeId) sessionQueue.erase(sessionQueue.begin()+i);
        }
    }

}

//List everything in the temporary queue and the saved queue
void list()
{
    //If both queues are empty
    if(sessionQueue.empty() && otQueue.empty())
    {
        //Print that they're both empty
        std::cout << "Nothing is in the queue" << std::endl;
    }
    //If only the one time queue is empty
    else if(otQueue.empty())
    {
        //For every element of the saved queue
        for(std::pair<std::string,std::string> currentPair : sessionQueue)
            {
                //Print the nickname then the channel id
                std::cout << currentPair.second << ", " << currentPair.first << std::endl;
            }
    }
    //If only the session queue is empty
    else if(sessionQueue.empty())
    {
        //For every element of the temporary queue
        for(std::pair<std::string,std::string> currentPair : otQueue)
        {
            //Print the nickname then the channel id
            std::cout << currentPair.second << ", " << currentPair.first << std::endl;
        }
    }
    //Neither are empty
    else
    {
        //Print all the saved queue first
        std::cout << "Saved queue: " << std::endl;
        for(std::pair<std::string,std::string> currentPair : sessionQueue)
        {
            std::cout << currentPair.first << ", " << currentPair.second << std::endl;
        }
        //Now print all the temporary queue
        std::cout << "Temporary queue: " << std::endl;
        for(std::pair<std::string,std::string> currentPair : otQueue)
        {
            std::cout << currentPair.first << ", " << currentPair.second << std::endl;
        }
    }
}

//Set the periodic interval time
void setInterval(int newIntervalTime)
{
    //If it's less than 1, don't let it be set
    if(newIntervalTime < 1) std::cout << "Interval time is too low. Please enter an integer value greater than zero." << std::endl;
    //It's not less than 1, set the interval time
    else timeInterval = newIntervalTime;
}

//Add an entry to the temporary queue
void includeTemp(std::string channelId, std::string nickname)
{
    std::pair<std::string,std::string> tempPair (channelId, nickname);
    otQueue.push_back(tempPair);
}

//Check if file exists
bool fileExists(const std::string& fileName)
{
    std::ifstream f(fileName.c_str());
    return f.good();
}

//Return the output of a terminal command
//Not quite sure what's happening here, just got it off of stackexchange
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
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

//Start the archive. This will be run asynchronously
void startArchive(std::string youtubeURL, std::string saveName, std::string activityName)
{
    //Create the activity file, a file that will indicate that a stream is being archived
    std::string activityFilePath = archiveDir + "." + activityName;
    std::ofstream outFile(activityFilePath);
    outFile << "active" << std::endl;
    outFile.close();
    //Generate and execute the command to download the livestream
    std::string dlCommand = "ffmpeg -loglevel -8 -y -i `youtube-dl -f best -g " + youtubeURL + "` " + archiveDir + saveName + ".ts";
    system(dlCommand.c_str());
    //Re-encode the video if the option is enabled
    if (finalFormat != "ts")
    {
        try
        {
            std::cout << "Finished downloading " << activityName << " stream. Converting to desired format " << finalFormat << "..." << std::endl;
            //Generate the re-encoding command
            std::string convertCommand = "ffmpeg -loglevel -8 -y -i " + archiveDir + saveName + ".ts " + archiveDir + saveName + "." + finalFormat; //.substr(0, saveName.find(".ts"))
            //Execute the re-encoding command
            system(convertCommand.c_str());
            std::cout << "Removing original " << activityName << " stream..." << std::endl;
            //Generate the removal command
            std::string removeCommand = "rm " + archiveDir + saveName + ".ts";
            //Execute the removal command
            system(removeCommand.c_str());
        }
        catch (...)
        {
            std::cout << "Error while converting to " << finalFormat << " (is the target format supported by ffmpeg?)" << std::endl;
        }
    }
    //Move the video if the option is enabled
    if(moveLocation != "")
    {
        try
        {
            std::cout << "Moving to location " << moveLocation << std::endl;
            //Generate the move command
            std::string moveCommand = "mv " + archiveDir + saveName + "." + finalFormat + " " + moveLocation;
            //Execute the move command
            system(moveCommand.c_str());
        }
        catch (...)
        {
            std::cout << "Error while moving stream (is the target location a directory?)" << std::endl;
        }
    }
    //The download is done, remove the activity file
    const char *activityFilePathChars = activityFilePath.c_str();
    remove(activityFilePathChars);
    std::cout << activityName << " stream ended or cancelled" << std::endl;
}

//Do everything we need to do every so often
void periodic()
{
    std::cout << "Starting checks..." << std::endl;
    //For every element of the saved queue
    for(int i = 0; i < sessionQueue.size(); i++)
    {
        //If the activity file exists, the stream is being downloaded so do nothing
        if(fileExists(archiveDir + "." + sessionQueue[i].second))
        {
            std::cout << sessionQueue[i].second + " activity file exists, stream is being archived" << std::endl;
        }
        //The activity file doesn't exist, so check if anything needs to be downloaded
        else
        {
            std::cout << sessionQueue[i].second + " activity file doesn't exist, checking for stream..." << std::endl;
            //Generate the python command, execute it, get the output and parse it
            std::string arguments = "/opt/youtube-archive/parse_youtube_data.py " + sessionQueue[i].first + " " + sessionQueue[i].second;
            std::string pythonOut = getCommandOutput(arguments.c_str());
            std::vector<std::string> parsedPython = parsePythonOutput(pythonOut);
            try
            {
                //Start the archive asynchronously
                procVector.push_back(std::async(startArchive, parsedPython[0], parsedPython[1], sessionQueue[i].second));
            }
            catch (...)
            {
                std::cout << sessionQueue[i].second << " stream does not exist or has not started" << std::endl;
            }
        }
    }
    std::cout << "\n";
}

void periodicCaller()
{
    //Create a loop count. It will overall take intervalTime but we don't want to do a huge wait, so we do it in small intervals
    int loopCount = 0;
    periodic();
    //Until it's cancelled by the user (ctrl+c is the only way for now)
    while(true)
    {
        //If the time in milliseconds divided by the periodic wait time is larger than our loops
        if(loopCount >= (timeInterval * 1000)/50)
        {
            //Do the periodic, set the loop count to 0
            periodic();
            loopCount = 0;
        }
        else
        {
        loopCount++;
        }
        
        //Wait 50 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

//Main function
int main(int argc, char **argv)
{	
    //If there are no extra arguments, print the usage
    if(argc < 2)
    {
    std::cerr << "Usage: " << std::endl
        << argv[0] << " [OPTIONS]" << std::endl;
        return 1;
    }

    //Get the saved queue
    sessionQueue = splitIntoVector(readFromFile(archiveQueue));
    //For every cli arg
    for(int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            //If it's help, print the help message
            help();
        }else if (std::string(argv[i]) == "-a" || std::string(argv[i]) == "--add") {
            //Try parsing the url and adding it to the queue
            try {
                std::string channelId = parseChannelURL(argv[i+1]);
                std::string nickname = argv[i+2];
                add(channelId, nickname);
            }
            catch (...)
            {
            std::cout << "The information you've entered is not valid. Please retry." << std::endl;
            }
        }else if (std::string(argv[i]) == "-r" || std::string(argv[i]) == "--remove") {
            //Remove the entry
            std::string remArg = argv[i+1];
            removeEntry(remArg);
        }else if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--list") {
            //List all the entries
            list();
        }else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--start") {
            //Set the start flag to true
            run = true;
        }else if (std::string(argv[i]) == "-i" || std::string(argv[i]) == "--interval") {
            //Set the interval time
            std::string tempInterval = argv[i+1];
            setInterval(std::stoi(tempInterval));
        }else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--temp") {
            //Try parsing the url and add it to the temp queue
            try {
                std::string tempChannelId = parseChannelURL(argv[i+1]);
                std::string tempNickname = argv[i+2];
                includeTemp(tempChannelId, tempNickname);
            }
            catch (...)
            {
            std::cout << "The information you've entered is not valid. Please retry." << std::endl;
            }
        }else if (std::string(argv[i]) == "-f" || std::string(argv[i]) == "--format") {
            //Change the format of the video
            finalFormat = argv[i+1];
        }else if (std::string(argv[i]) == "-m" || std::string(argv[i]) == "--move") {
            moveLocation = argv[i+1];
        }

    }
    //All the options have been completed except for run, so write the final saved queue to file
    writeToFile(archiveQueue, saveToString(sessionQueue));

    //If we want to run the actual archiver
    if(run)
    {
        periodicCaller();
    }

}
