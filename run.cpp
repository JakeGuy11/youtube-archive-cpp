#include<iostream>
#include<fstream>

int main(int argc, char** argv)
{

    if(argc == 1)
    {
    std::ifstream helpFile("help");
    if (helpFile.is_open()) std::cout << helpFile.rdbuf();
    }

}