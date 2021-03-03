# TEMPORARY NOTE
After some more testing, a new problem was revealed. Depending on the specs of your computer, downloading too many streams at once can corrupt small sections of the video and, in extreme cases, even cause the program to crash.
For now, it is recommended that you don't download more than one stream at a time if you're using a bad-medium laptop or a bad PC. If you have a good/gaming laptop or a medium/good/gaming PC, it's recommended that you don't download more than three streams at a time.
For reference, on a ThinkPad T530 (intel i7 @ 2.9GHz, 16GB DDR3), two streams could be downloaded at once with a very small amout of corruption.

If you want to maxumize preformance while downloading more than the reccomended amount of streams, you should not change the format (i.e. omit the -f option), as this takes up a lot of system resources. A fix will be added in the next release.

# Contents
- [Dependencies](#Dependencies)
- [Installation](#Installation)
- [Description](#Description-and-Disclaimer)
- [Example Commands](#Options-and-Examples)
- [Contact Me](#Contact-Me)

# Dependencies
1. [ffmpeg](https://ffmpeg.org/ffmpeg.html) version n4.3.1 or higher:
```bash
#Check version
ffmpeg -version
```

2. [youtube-dl](https://github.com/ytdl-org/youtube-dl) version 2021.02.04 or higher:
```bash
#Check version
youtube-dl --version
```

3. [Python](https://www.python.org/) version 3.9.1 or higher:
```bash
#Check version
python --version
```

4. [Requests](https://requests.readthedocs.io/en/master/) version 2.25.1 or higher:
```bash
#Check version
pip show requests
```

# Installation
To install on UNIX:
1. Download the latest release
2. Extract it into a new folder
```bash
cd ~/Downloads/
mkdir ./youtube-archive-extracted/
tar -xpvzf ./youtube-archive-X.X.X.tar.gz -C ./youtube-archive-extracted/
```

3. Add the executable permission to the install script
```bash
cd ./youtube-archive-extracted/
chmod +x ./install.sh
```

4. Execute the install script as root
```bash
sudo ./install.sh
```

5. Remove the install folder
```bash
cd ../
rm -r ./youtube-archive-extracted/
```

6. youtube-archive is ready to be used!\
\
No windows or mac packages are officially available yet.

# Description and Disclaimer
Youtube-archive is a command line only tool that passively archives YouTube livestreams. It has a customizeable saved and temporary queue that can be different for each user. It is built mainly in C++ but uses a python script for web interactions.\
\
As mentioned in the help page, youtube-archive uses web scraping as an alternative to the YouTube API. This means that it requests the page source from YouTube instead of using the intended YouTube interface. The upside to this (and the reason I chose it) is that it's completely free! The downside is that it breaks often, really whenever YouTube has a major update. Also, it requires extra security so that YouTube doesn't flag your computer as a bot. I've taken most of the usual precautions to ensure this doesn't happen(and it's very nulikely that it will), but still know that there's a chance YouTube might flag your computer. If this app stops working, *please open an error on the github page.* If you're not sure what the problem is, or you suspect that YouTube's flagged your computer as a bot, I'd appreciate it if you attach a copy of the page source of one of the YouTube channels in your queue.

# Options and Examples
This app has quite a few features which are all described in the help page and can be found by running `youtube-archive --help`, however I'll still briefly explain how to get a working session running.\
\
**Add an entry to the saved queue:**
```bash
youtube-archive --add https://www.youtube.com/channel/CHANNELCODE EntryNickname
```

**Add an entry to the temporary queue(this will not do anything since it is not being run in the same session):**
```bash
youtube-archive --temp https://www.youtube.com/channel/CHANNELCODE EntryNickname
```

**Start the program scanning for entries in the saved queue:**
```bash
youtube-archive --start
```

# Contact Me
If you have any feedback, suggestions, errors or just general comments, please email me at Jake_Guy_11@protonmail.ch, or open an error through GitHub.
