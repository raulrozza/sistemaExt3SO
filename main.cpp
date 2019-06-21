#include <string>
#include <iostream>
#include "fileSystem.h"

void runSystem(){
    FileSystem* system = nullptr;

    cout<<"Welcome to File System!\n\n";
    cout<<"Enter your commands\n";

    while(1){
        char command[CHAR_SIZE];
        memset(command, 0, sizeof(command));

        cout<<">>";
        fgets(command, sizeof(command), stdin);

        if(strncmp(command, "init ", 5) == 0){
            int i;
            char fileName[CHAR_SIZE], blockCount[CHAR_SIZE], blockSize[CHAR_SIZE], inodeCount[CHAR_SIZE];
            int iBlockCount, iBlockSize, iInodeCount;

            memmove(command, command + 5, strlen(command));
            if(system->split(fileName, command, ' ') == 1){
                cout<<"Please inform the size (in bytes) of the memory blocks.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            if(system->split(blockSize, command, ' ') == 1){
                cout<<"Please inform the number of memory blocks.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            if(system->split(blockCount, command, ' ') == 1){
                cout<<"Please inform the number of inodes.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            if(system->split(inodeCount, command, '\0') == 1){
                cout<<"Invalid init statement.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            iBlockCount = atoi(blockCount);
            iBlockSize = atoi(blockSize);
            iInodeCount = atoi(inodeCount);
            if(iInodeCount > iBlockCount){
                cout<<"Number of inodes can't be bigger than the number of blocks.\n";
                continue;
            }

            system = nullptr;
            system = new FileSystem(fileName, iBlockCount, iBlockSize, iInodeCount);
            system->init();

            cout<<"File system "<<fileName<<" initialized.\n";

            memset(command, 0, sizeof(command));
        }
        else if(strncmp(command, "addDir ", 7) == 0){
            char fileName[CHAR_SIZE], dirName[CHAR_SIZE];

            memmove(command, command + 7, strlen(command));
            
            if(system->split(fileName, command, ' ') == 1){
                cout<<"Please inform name of the directory.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            if(system->split(dirName, command, (char) 10) == 1){
                cout<<"Invalid addDir statement.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            /* char oldDirName[CHAR_SIZE];
            strcpy(oldDirName, dirName);
            system->split(dirName, oldDirName, ' '); */

            if(system == nullptr)
                system = FileSystem::initSystem(fileName);
            else if(strncmp(system->_fileSystem, fileName, strlen(fileName)) != 0)
                system = FileSystem::initSystem(fileName);

            system->addDir(dirName);

            memset(command, 0, sizeof(command));
        }
        else if(strncmp(command, "addFile ", 8) == 0){
            char systemName[CHAR_SIZE], fileName[CHAR_SIZE], fileContent[CHAR_SIZE];
            memset(systemName, 0, sizeof(systemName));
            memset(fileName, 0, sizeof(fileName));
            memset(fileContent, 0, sizeof(fileContent));

            memmove(command, command + 8, strlen(command));
            
            if(system->split(systemName, command, ' ') == 1){
                cout<<"Please inform name of the file.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            if(system->split(fileName, command, ' ') == 1){
                cout<<"The file can't be empty.\n";
                memset(command, 0, sizeof(command));
                continue;
            }
            if(system->split(fileContent, command, (char) 10) == 1){
                cout<<"Invalid addFile statement.\n";
                memset(command, 0, sizeof(command));
                continue;
            }

            if(system == nullptr)
                system = FileSystem::initSystem(systemName);
            else if(strncmp(system->_fileSystem, systemName, strlen(systemName)) != 0)
                system = FileSystem::initSystem(systemName);

            system->addFile(fileName, fileContent);

            memset(command, 0, sizeof(command));
        }
        else{
            cout<<"Invalid command!\n";
        }
    }
}

int main(){

    runSystem();

    return 0;
}