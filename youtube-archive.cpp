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
std::string finalFormat = "mp4";
//Set the final destination of the streams
std::string moveLocation = "";
//Set the default print level to 0. -n is quiet, +1 is debug
int printLvl = 0;
//Create a default download quality string
std::string qualityArgs = "worst[height=480]";
//Create a boolean for whether or not we want to add the UIDs
bool UIDEnabled = false;
//Create an integer for the unique IDs so that nothing gets overwritten
int UID = 1;
//A string for the title
std::string unparsedSaveName = "%D-%N";
//A bool for whether or not the user is enabling the login feature
bool loginEnabled = false;
//Strings for if the username and password are enabled
std::string userUsername = "";
std::string userPassword = "";

void print(int level, auto msg)
{
    std::string prefix = "";

    if(level == -1) prefix = "[ERROR]: ";
    else if(level == 1) prefix = "[DEBUG]: ";

    if(level <= printLvl)
    {
        std::cout << prefix << msg << std::endl;
    }
}

//Create a method that returns the channel id from its url
std::string parseChannelURL(std::string url)
{
    print(1, "Parsing channel url: " + url);
    //Find where the last / is, it will be followed by the id
    std::size_t occurrence = url.find_last_of("/");
    //Take only the last part of the url, which will be /id
    std::string channelWithSlash = url.substr(occurrence);
    //Remove from the slash from the id
    channelWithSlash.erase(0,1);
    //Return the id
    print(1, "Parsed channel url, id: " + channelWithSlash);
    return channelWithSlash;
}

//Parse the python output, which will be formatted as url;videoTitle;date
std::vector<std::string> parsePythonOutput(std::string const pythonOut)
{
    print(1, "Parsing python output: " + pythonOut);
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
        print(1, "Found python return segment: " + foundString);
    }
    //Return the return vector
    return retVec;
}

//Parse the queue string, searching for '|' and ';' to separate the elements
std::vector<std::pair<std::string,std::string>> splitIntoVector(std::string const &str)
{
    print(1, "Splitting \"" + str + "\" into vector");
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
        print(1, "Found primary segment: " + foundString);
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
                print(1, "Found secondary element(nickname): " + foundString);
            }
            //We haven't added the first element
            else
            {
                //Add the url to the first element of the pair
                tempPair.first = foundString;
                //We've added the first element, so next time we want to add it to the second
                added = true;
                print(1, "Found secondary element(channel id): " + foundString);
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
    print(1, "Converting vector to string with delimetors '|' and ';'");
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
            print(1, "First element, adding " + returnString);
        }
        //It's not the first element
        else
        {
            //Add the element separating character '|', then the first pair element, then ';' then the second pair element
            returnString += "|" + vectorToConvert[i].first + ";" + vectorToConvert[i].second;
            print(1, "Appending \"|" + vectorToConvert[i].first + ";" + vectorToConvert[i].second + "\"");
        }
    }
    //Return the generated string
    print(1, "Generated string: " + returnString);
    return returnString;
}

//Grab the entire contents of a file
std::string readFromFile(std::string filePath)
{
    print(1, "Reading contents of " + filePath);
    //Create an input file stream from the given path
    std::ifstream ifs(filePath);
    //Get the contents of the file
    std::string fileContents( (std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>()));
    //Return the contents
    print(1, "File contents: " + fileContents);
    return fileContents;
}

//Write a string to a file
void writeToFile(std::string filePath, std::string stringToWrite)
{
    print(1, "Writing \"" + stringToWrite + "\" to " + filePath);
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
    print(1, "Written sucessfully");
}

//Print the help page
void help()
{
    print(1, "Printing help");
    //Create an infile stream from the file named help
    std::ifstream helpFile("/opt/youtube-archive/help");
    //Print the contents of help
    print(-1, helpFile.rdbuf());
}

//Add an entry to the queue
void add(std::string channelId, std::string nickname)
{
    print(1, "Adding " + channelId + ", " + nickname + " to session queue");
    //Create a pair from the two arguments passed
    std::pair<std::string,std::string> tempPair (channelId,nickname);
    //Add that pair to the session queue
    sessionQueue.push_back(tempPair);
    print(1, "Sucessfully added to queue");
}

//Remove an entry from the queue
void removeEntry(std::string remArg)
{
    bool entryRemoved = false;
    print(1, "Removing entry with id " + remArg);
    //Create a blank string that'll hold the removal identifier
    std::string removeId = "";
    //We don't know if it's the nickname or id, so try parsing it as a channel url
    try
    {
        print(1, "Checking if the removal argument is a channel url or nickname");
        //Parse it as a channel url
        //If it's not a url, it'll throw an error
        removeId = parseChannelURL(remArg);
        print(1, "The removal argument is a channel url");
        //For every element in our queue vector
        for(int i = 0; i < sessionQueue.size(); i++)
        {
            print(1, "Checking element " + std::to_string(i) + " of the queue, " + sessionQueue[i].first);
            //If the channel id of this element matches the one that should be removed, remove it
            if(sessionQueue[i].first == removeId){
                print(1, "Entry matches argument, removing it");
                entryRemoved = true;
                sessionQueue.erase(sessionQueue.begin()+i);
            }
            else
            {
                print(1, "Entry does not match argument");
            }
        }
    }
    //It's not a parsable url, so check the channel nicknames
    catch (...)
    {
        print(1, "The removal argument is not a channel url");
        //The removal identifier will be the same as the argument passed
        removeId = remArg;
        //For every element in our queue vector
        for(int i = 0; i < sessionQueue.size(); i++)
        {
            print(1, "Checking element " + std::to_string(i) + " of the queue, " + sessionQueue[i].second);
            //If the name in this element of the vector matches the removal id, remove the element
            if(sessionQueue[i].second == removeId){
                print(1, "Entry matches argument, removing it");
                entryRemoved = true;
                sessionQueue.erase(sessionQueue.begin()+i);
            }
            else
            {
                print(1, "Entry does not match argument");
            }
        }
    }

    if(!entryRemoved) print (0, "Identifier not found, nothing removed");
}

//List everything in the temporary queue and the saved queue
void list()
{
    print(1, "Listing queue");
    //If both queues are empty
    if(sessionQueue.empty() && otQueue.empty())
    {
        //Print that they're both empty
        print(0, "Nothing is in the queue");
    }
    //If only the one time queue is empty
    else if(otQueue.empty())
    {
        print(1, "Temporary queue is empty, printing session queue");
        //For every element of the saved queue
        for(std::pair<std::string,std::string> currentPair : sessionQueue)
            {
                //Print the nickname then the channel id
                print(0, currentPair.first + ", " + currentPair.second);
            }
    }
    //If only the session queue is empty
    else if(sessionQueue.empty())
    {
        print(1, "Session queue is empty, printing temporary queue");
        //For every element of the temporary queue
        for(std::pair<std::string,std::string> currentPair : otQueue)
        {
            //Print the nickname then the channel id
            print(0, currentPair.first + ", " + currentPair.second);
        }
    }
    //Neither are empty
    else
    {
        print(1, "Both queues contain elements, printing both");
        //Print all the saved queue first
        print(0, "Saved queue: ");
        for(std::pair<std::string,std::string> currentPair : sessionQueue)
        {
            print(0, currentPair.first + ", " + currentPair.second);
        }
        //Now print all the temporary queue
        print(0, "Temporary queue: ");
        for(std::pair<std::string,std::string> currentPair : otQueue)
        {
            print(0, currentPair.first + ", " + currentPair.second);
        }
    }
}

//Set the periodic interval time
void setInterval(int newIntervalTime)
{
    print(1, "Changing interval time");
    //If it's less than 1, don't let it be set
    if(newIntervalTime < 1)
    {
        print(-1, "Interval time is too low. Please enter an integer value greater than zero.");
    }
    //It's not less than 1, set the interval time
    else
    {
        print(1, "Interval time is greater than 1, setting it");
        timeInterval = newIntervalTime;
    }
}

//Add an entry to the temporary queue
void includeTemp(std::string channelId, std::string nickname)
{
    print(1, "Adding " + channelId + ", " + nickname + " to temporary queue");
    std::pair<std::string,std::string> tempPair (channelId, nickname);
    otQueue.push_back(tempPair);
}

//Check if file exists
bool fileExists(const std::string& fileName)
{
    print(1, "Checking if " + fileName + " exists");
    std::ifstream f(fileName.c_str());
    if (f.good()) print(1, "File exists");
    else print(1, "File does not exist");
    return f.good();
}

std::string replaceCharacters(std::string inStr, std::string findStr, std::string replaceStr, int length)
{
    std::size_t resPos = inStr.find(findStr);
    while(resPos != std::string::npos)
    {
        inStr.replace(resPos, length, replaceStr);
        resPos = inStr.find(findStr);
    }
    return inStr;
}

void parseQArgs(std::string qualityString)
{

    std::stringstream tempStream(qualityString);
    std::vector<std::string> qualityArgumentVector;
    std::string foundString = "";

    //This will loop until the end of the string and will add foundString to the vector every time it finds the regex
    while (std::getline(tempStream, foundString, ',')) {
        qualityArgumentVector.push_back(foundString);
        print(1, "Found element of quality vector: " + foundString);
    }

    //Print out the elements raw of the array
    for(int i = 0; i < qualityArgumentVector.size(); i++)
    {
        print(1, "Current unparsed element of quality arguments: " + qualityArgumentVector[i]);
        print(1, "Replacing any \"res\" with \"height\"");
        qualityArgumentVector[i] = replaceCharacters(qualityArgumentVector[i], "res", "height", 3);
    }

    print(1, "Generating quality arguments...");
    qualityArgs = "\"worst";
    for(int j = 0; j < qualityArgumentVector.size(); j++)
    {
        qualityArgs = qualityArgs + "[" + qualityArgumentVector[j] + "]";
    }
    print(1, "Generated quality arguments: " + qualityArgs);
    print(1, "Appending defaults to quality arguments...");
    qualityArgs = qualityArgs + "/worst[height=480]/worst\"";
    print(1, "Finished generation of quality arguments: " + qualityArgs);
    

    //If they want best quality, overwrite it with best
    if(qualityString == "best")
    {
        qualityArgs = "best";
    }
}

//Return the output of a terminal command
//Not quite sure what's happening here, just got it off of stackexchange
std::string getCommandOutput(const char* cmd)
{
    print(1, "Getting command output");
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        print(-1, "Popen failed");
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        print(1, "Appending data to return string");
        result += buffer.data();
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    print(1, "Removing new lines");
    print(1, "Returning " + result);
    return result;
}

std::string generateTitle(std::string titleRegex, std::string idStr, std::string nameStr, std::string dateStr, std::string titleStr)
{
    print(1, "Parsing title expressions");
    print(1, "Replacing %I with " + idStr);
    titleRegex = replaceCharacters(titleRegex, "%I", idStr, 2);
    print(1, "Replacing %N with " + nameStr);
    titleRegex = replaceCharacters(titleRegex, "%N", nameStr, 2);
    print(1, "Replacing %D with " + dateStr);
    titleRegex = replaceCharacters(titleRegex, "%D", dateStr, 2);
    print(1, "Replacing %T with " + titleStr);
    titleRegex = replaceCharacters(titleRegex, "%T", titleStr, 2);
    print(1, "Final title string: " + titleRegex);
    return titleRegex;
}

//Start the archive. This will be run asynchronously
void startArchive(std::string youtubeURL, std::string saveName, std::string activityName)
{
    print(1, "Starting archive of " + activityName);
    print(1, "YouTube URL to download: " + youtubeURL);
    //Create the activity file, a file that will indicate that a stream is being archived
    std::string activityFilePath = archiveDir + "." + activityName;
    print(1, "Activity file path and name: " + activityFilePath);
    std::ofstream outFile(activityFilePath);
    //Change the save path and name based on whether or not the UID is enabled
    std::string savePath = "";
    if(UIDEnabled)
    {
        savePath = archiveDir + saveName + "-" + std::to_string(UID);
        print(1, "Used UID " + std::to_string(UID));
        UID++;
    }
    else
    {
        savePath = archiveDir + saveName;
    }
    print(1, "Save Path: " + savePath);
    outFile << "active" << std::endl;
    outFile.close();
    //Generate and execute the command to download the livestream
    std::string dlCommand = "ffmpeg -loglevel -8 -y -i `youtube-dl -f " + qualityArgs + " -u \"" + userUsername + "\" -p \"" + userPassword + "\" --abort-on-unavailable-fragment --no-continue --quiet -g " + youtubeURL + "` -c copy \"" + savePath + ".mp4\"";
    print(1, "Download command for " + activityName + ": " + dlCommand);
    system(dlCommand.c_str());
    //Re-encode the video if the option is enabled
    if (finalFormat != "mp4")
    {
        print(1, "Final format is not .mp4, converting to " + finalFormat);
        try
        {
            print(0, "Finished downloading " + activityName + " stream. Converting to desired format " + finalFormat + "...");
            //Generate the re-encoding command
            std::string convertCommand = "ffmpeg -loglevel -8 -y -i \"" + savePath + ".mp4\" \"" + savePath + "." + finalFormat + "\"";
            print(1, "Conversion command for " + activityName + ": " + convertCommand);
            //Execute the re-encoding command
            system(convertCommand.c_str());
            print(0, "Removing original " + activityName + " stream...");
            //Generate the removal command
            std::string removeCommand = "rm \"" + savePath + ".mp4\"";
            print(1, "Removal command for " + activityName + ": " + removeCommand);
            //Execute the removal command
            system(removeCommand.c_str());
        }
        catch (...)
        {
            print(0, "Error while converting to " + finalFormat + " (is the target format supported by ffmpeg?)");
        }
    }
    //Move the video if the option is enabled
    if(moveLocation != "")
    {
        print(1, "Move location is not default, moving to " + moveLocation);
        try
        {
            print(0, "Moving to location " + moveLocation);
            //Generate the move command
            std::string moveCommand = "mv \"" + savePath + "." + finalFormat + "\" " + moveLocation;
            print(1, "Move command for " + activityName + ": " + moveCommand);
            //Execute the move command
            system(moveCommand.c_str());
        }
        catch (...)
        {
            print(-1, "Error while moving stream (is the target location a directory?)");
        }
    }
    print(1, "Removing activity file " + activityFilePath);
    //The download is done, remove the activity file
    const char *activityFilePathChars = activityFilePath.c_str();
    remove(activityFilePathChars);
    print(0, activityName + " stream ended or cancelled");
}

//Do everything we need to do every so often
void periodic()
{
    print(0, "Starting periodic checks...");
    //For every element of the saved queue
    for(int i = 0; i < sessionQueue.size(); i++)
    {
        print(1, "On element " + std::to_string(i) + '(' + sessionQueue[i].second + ") of session queue");
        //If the activity file exists, the stream is being downloaded so do nothing
        print(1, "Checking for activity file " + archiveDir + "." + sessionQueue[i].second);
        if(fileExists(archiveDir + "." + sessionQueue[i].second))
        {
            print(0, sessionQueue[i].second + " activity file exists, stream is being archived");
        }
        //The activity file doesn't exist, so check if anything needs to be downloaded
        else
        {
            print(0, sessionQueue[i].second + " activity file doesn't exist, checking for stream...");
            //Generate the python command, execute it, get the output and parse it
            std::string arguments = "/opt/youtube-archive/parse_youtube_data.py " + sessionQueue[i].first;
            print(1, "Generated python command: " + arguments);
            std::string pythonOut = getCommandOutput(arguments.c_str());
            print(1, "Python output: " + pythonOut);
            try
            {
                print(1, "Parsing python output...");
                std::vector<std::string> parsedPython = parsePythonOutput(pythonOut);
                print(1, "Parsed python output");
                if(parsedPython.size() < 3) throw (1);
                print(1, "Parsing title regex");
                std::string saveTitle = generateTitle(unparsedSaveName, std::to_string(UID), sessionQueue[i].second, parsedPython[2], parsedPython[1]); //date title
                print(1, "Parsed title: " + saveTitle);
                print(1, "Adding process asynchronously: startArchive with arguments " + parsedPython[0] + ", " + saveTitle + ", " + sessionQueue[i].second); 
                //Start the archive asynchronously
                procVector.push_back(std::async(startArchive, parsedPython[0], saveTitle, sessionQueue[i].second));
            }
            catch (...)
            {
                print(0, sessionQueue[i].second + " stream does not exist or has not started");
            }
        }
    }
    print(0, "");
}

void periodicCaller()
{
    print(1, "Calling periodic");
    //Create a loop count. It will overall take intervalTime but we don't want to do a huge wait, so we do it in small intervals
    int loopCount = 0;
    periodic();
    //Until it's cancelled by the user (ctrl+c is the only way for now)
    while(true)
    {
        //If the time in milliseconds divided by the periodic wait time is larger than our loops
        if(loopCount >= (timeInterval * 1000)/50)
        {
            print(1, "Starting periodic checks");
            //Do the periodic, set the loop count to 0
            periodic();
            print(1, "Resetting loop counter");
            loopCount = 0;
        }
        else
        {
        loopCount++;
        }
        
        //Wait 50 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    print(1, "periodicCaller is over (force cancelled)");
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
    
    //Check for debugging mode first
    for(int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "-p" || std::string(argv[i]) == "--print-level") {
            std::string tempPrintLvl = argv[i+1];
            printLvl = std::stoi(tempPrintLvl);
        }else if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose") {
            printLvl = 10;
        }
    }
    
    print(1, "Debug mode enabled");

    //Get the saved queue
    print(1, "Getting saved queue");
    sessionQueue = splitIntoVector(readFromFile(archiveQueue));
    //For every cli arg
    for(int i = 1; i < argc; ++i)
    {
        print(1, "On argument " + std::to_string(i) + " of argv: " + argv[i]);
        if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            print(1, "Help requested");
            help();
        }else if (std::string(argv[i]) == "-a" || std::string(argv[i]) == "--add") {
            print(1, "Add requested");
            //Try parsing the url and adding it to the queue
            try {
                std::string channelId = parseChannelURL(argv[i+1]);
                std::string nickname = argv[i+2];
                print(1, "Adding " + channelId + ", " + nickname);
                add(channelId, nickname);
            }
            catch (...)
            {
                print(-1, "The information you've entered is not valid. Please retry.");
            }
        }else if (std::string(argv[i]) == "-r" || std::string(argv[i]) == "--remove") {
            print(1, "Remove requested");
            //Remove the entry
            std::string remArg = argv[i+1];
            print(1, "Removing " + remArg);
            removeEntry(remArg);
        }else if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--list") {
            print(1, "List requested");
            //List all the entries
            list();
        }else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--start") {
            print(1, "Start requested, adding start flag");
            //Set the start flag to true
            run = true;
        }else if (std::string(argv[i]) == "-i" || std::string(argv[i]) == "--interval") {
            print(1, "Interval change requested");
            //Set the interval time
            std::string tempInterval = argv[i+1];
            print(1, "Requested interval time: " + tempInterval);
            setInterval(std::stoi(tempInterval));
        }else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--temp") {
            print(1, "Temporary add requested");
            //Try parsing the url and add it to the temp queue
            try {
                std::string tempChannelId = parseChannelURL(argv[i+1]);
                std::string tempNickname = argv[i+2];
                print(1, "Adding " + tempChannelId + ", " + tempNickname + " to temporary queue");
                includeTemp(tempChannelId, tempNickname);
            }
            catch (...)
            {
                print(-1, "The information you've entered is not valid. Please retry.");
            }
        }else if (std::string(argv[i]) == "-f" || std::string(argv[i]) == "--format") {
            print(1, "Format change requested");
            //Change the format of the video
            finalFormat = argv[i+1];
            print(1, "Set format to " + finalFormat);
        }else if (std::string(argv[i]) == "-m" || std::string(argv[i]) == "--move") {
            //Change the move location
            moveLocation = argv[i+1];
            print(1, "Setting move location to " + moveLocation);
        }else if (std::string(argv[i]) == "-q" || std::string(argv[i]) == "--quality-args") {
            print(1, "Quality string: " + std::string(argv[i+1]));
            parseQArgs(std::string(argv[i+1]));
        }else if (std::string(argv[i]) == "-u" || std::string(argv[i]) == "--unique-ids") {
            print(1, "Unique IDs requested");
            UIDEnabled = true;
        }else if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--custom-title") {
            print(1, "Setting custom title to " + std::string(argv[i+1]));
            unparsedSaveName = argv[i+1];
        }else if (std::string(argv[i]) == "--username") {
            print(1, "User has opted to add a username: " + std::string(argv[i+1]));
            loginEnabled = true;
            userUsername = argv[i+1];
        }else if (std::string(argv[i]) == "--password") {
            print(1, "User has opted to add a password: " + std::string(argv[i+1]));
            loginEnabled = true;
            userPassword = argv[i+1];
        }

    }
    
    //All the options have been completed except for run, so write the final saved queue to file
    print(1, "Writing session queue to file");
    writeToFile(archiveQueue, saveToString(sessionQueue));

    //If we want to run the actual archiver
    if(run)
    {
        print(1, "Starting archiver");
        periodicCaller();
    }
    print(1, "Program finished");
}
