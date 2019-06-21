#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <cmath>
#include "structs.h"
//#include "sha256.h"

#define CHAR_SIZE 1024

using namespace std;

class FileSystem{
    private:
        int _blockCount;
        int _blockSize;
        int _inodeCount;
        INODE* _inodeArray;
        int _bitmapStart;
        int _inodeStart;
        int _rootStart;
        int _blockStart;

    public:
        char* _fileSystem;

        FileSystem(char* fileSystem, int blocks, int blockSize, int inodes){
            _fileSystem = fileSystem;
            _blockCount = blocks;
            _blockSize = blockSize;
            _inodeCount = inodes;
            _inodeArray = nullptr;
        }

    // General functions
        void init();
        void addFile(char* fileName, char* fileContent);
        void addDir(char* dirName);

    // Specific functions
        static FileSystem* initSystem(char* fileName);
        void readFile(FILE* file);
        void changeInode(FILE* file, INODE inode, int addr);
        INODE* readInode(FILE* file, int addr);
        int findFreeInode(FILE* file);
        char* readBlock(FILE* file, int addr);
        int findFreeBlockByte(FILE* file, unsigned char addr, unsigned char data);
        vector2 findEmptyBit(FILE* file);
        void setBit(FILE* file, int bitmap, int bit);
        void clearBit(FILE* file, int bitmap, int bit);
        int findParentDir(char* dirName);

    // Utility
        static int split(char* newString, char* targetString, char token);
};

#endif