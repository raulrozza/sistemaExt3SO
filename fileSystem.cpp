#include "fileSystem.h"

void FileSystem::init(){
    int seek;
    FILE* file = fopen(_fileSystem, "wb");
    if(file == NULL){
        cout<<"Error creating file system.\n";
        return;
    }

    // Amounts of bytes (chars) created with the file must be:
    // 1 blockSize, 1 blockCount, 1 inodeCount = 3
    // Bitmap = ceil(blockCount/8)
    // Inode array = (sizeof(inode) * inodeCount)
    // Root dir = 1
    // Block array = blockSize * blockCount
    
    _bitmapStart = 3;
    _inodeStart = _bitmapStart + ceil(_blockCount/8.0);
    _rootStart = _inodeStart + (sizeof(INODE) * _inodeCount);
    _blockStart = _rootStart + 1;

    int byteCount = _blockStart + (_blockCount * _blockSize);

    for(int i = 0; i < byteCount; i++){
        fputc(0x00, file);
    }

    // Insert initial configurations
    seek = fseek(file, 0, 0);
    fputc((unsigned char)_blockSize, file);
    seek = fseek(file, 1, 0);
    fputc((unsigned char)_blockCount, file);
    seek = fseek(file, 2, 0);
    fputc((unsigned char)_inodeCount, file);

    // Prepare inode array
    if(_inodeArray != nullptr)
        free(_inodeArray);
    _inodeArray = (INODE*) malloc(sizeof(INODE)*_inodeCount);
    for(int i = 0; i < _inodeCount; i++){
        _inodeArray[i].IS_USED = '0';
        _inodeArray[i].IS_DIR = '0';
        _inodeArray[i].SIZE = '0';
        for(int j = 0; j < 10; j++)
            _inodeArray[i].NAME[j] = 0x00;
        for(int j = 0; j < 9; j++)
            _inodeArray[i].DIRECT_BLOCKS[j] = 0x00;
    }

    //SETTING THE ROOT DIRECTORY

    //bitmap change
    seek = fseek(file, _bitmapStart, 0);
    fputc(0x01, file);

    //define root dir
    seek = fseek(file, 0, _inodeStart);
    _inodeArray[0].NAME[0] = '/';
    _inodeArray[0].IS_USED = 0x01;
    _inodeArray[0].IS_DIR = 0x01;
    _inodeArray[0].SIZE = 0x00;
    changeInode(file, _inodeArray[0], 0);

    fclose(file);
}

void FileSystem::addDir(char* dirName){
    int seek;
    char thrash[CHAR_SIZE];
    char parentDir[CHAR_SIZE];
    int freeInode;
    int found = 0;
    int parentIndex = 0;
    vector2 freeBitmap;
    
    FILE* file = fopen(_fileSystem, "r+b");
    if(file == NULL){
        cout<<"Error opening file system.\n";
        return;
    }

    split(thrash, dirName, '/');

    // Searching for an empty inode
    freeInode = findFreeInode(file);
    if(freeInode == -1){
        fclose(file);
        return;
    }

    //Setting the parent directory

    while(split(thrash, dirName, '/') == 0){
        memset(parentDir, 0, CHAR_SIZE);
        strcpy(parentDir, thrash);
        found = 1;
    }
    
    if(found == 1){
        parentIndex = findParentDir(parentDir);
        if(parentIndex < 0){
            cout<<"There's no such directory as "<<parentDir<<endl;
            fclose(file);
            return;
        }
    }

    // Setting the new directory

    //Registering the new inode in the parent dir

    //CONSIDERING NO INDIRECT BLOCK IMPLEMENTATION

    found = 0;
    for(int i = 0; i < 9; i++){
        found = findFreeBlockByte(file, _inodeArray[parentIndex].DIRECT_BLOCKS[i], (unsigned char) (freeInode/sizeof(INODE)));
        if(found == 1)
            break;
    }
    if(found == 0){
        cout<<"There's no space to add further directories.\n";
        fclose(file);
        return;
    }
    _inodeArray[parentIndex].SIZE += 0x01;

    changeInode(file, _inodeArray[parentIndex], parentIndex*sizeof(INODE));

    // Creating the new inode
    INODE newNode;
    newNode.IS_USED = 0x01;
    newNode.IS_DIR = 0x01;
    for(int i = 0; i < 10; i++){
        if(i < (strlen(dirName)))
            newNode.NAME[i] = dirName[i];
        else
            newNode.NAME[i] = 0x00;
    }
    newNode.SIZE = 0x00;
    for(int i = 0; i < 9; i++)
        newNode.DIRECT_BLOCKS[i] = 0x00;

    freeBitmap = findEmptyBit(file);
    if(freeBitmap.x == -1 && freeBitmap.y == -1){
        fclose(file);
        return;
    }
    setBit(file, freeBitmap.x, freeBitmap.y);
    int blockAddr = _blockStart + ((freeBitmap.x - 3) + freeBitmap.y)*_blockSize;
    fseek(file, blockAddr, SEEK_SET);
    newNode.DIRECT_BLOCKS[0] = (unsigned char) (blockAddr - _blockStart)/_blockSize;

    _inodeArray[freeInode/sizeof(INODE)] = newNode;
    changeInode(file, newNode, freeInode);

    cout<<dirName<<" added to system.\n";

    fclose(file);
}

void FileSystem::addFile(char* fileName, char* fileContent){
    int freeInode, blockCount;
    vector2* freeBitmap = nullptr;
    int found = 0;
    char thrash[CHAR_SIZE];
    char parentDir[CHAR_SIZE];
    int parentIndex = 0;
    FILE* file = fopen(_fileSystem, "r+b");
    if(file == NULL){
        cout<<"Error opening file system.\n";
        return;
    }

    split(thrash, fileName, '/');

    // Get amount of blocks needed for data allocation
    blockCount = ceil(((float)strlen(fileContent))/_blockSize);
    if(blockCount >= 9){
        cout<<"The file's content is too large.\n";
        return;
    }
    freeBitmap = (vector2*) malloc(sizeof(vector2)*blockCount);

    // Searching for an empty inode
    freeInode = findFreeInode(file);
    if(freeInode == -1){
        fclose(file);
        return;
    }

    //Setting the parent directory

    while(split(thrash, fileName, '/') == 0){
        memset(parentDir, 0, CHAR_SIZE);
        strcpy(parentDir, thrash);
        found = 1;
    }
    
    if(found == 1){
        parentIndex = findParentDir(parentDir);
        if(parentIndex < 0){
            cout<<"There's no such directory as "<<parentDir<<endl;
            fclose(file);
            return;
        }
    }

    // Searching/setting bits on bitmap
    for(int i = 0; i < blockCount; i++){
        freeBitmap[i] = findEmptyBit(file);
        if(freeBitmap[i].x == -1 && freeBitmap[i].y == -1){
            for(int j = 0; j < i; j++)
                clearBit(file, freeBitmap[j].x, freeBitmap[j].y);
            fclose(file);
            return;
        }
        setBit(file, freeBitmap[i].x, freeBitmap[i].y);
    }

    // Creating the new inode
    INODE newNode;
    newNode.IS_USED = 0x01;
    newNode.IS_DIR = 0x00;
    for(int i = 0; i < 10; i++){
        if(i < (strlen(fileName)))
            newNode.NAME[i] = fileName[i];
        else
            newNode.NAME[i] = 0x00;
    }
    newNode.SIZE = (unsigned char) strlen(fileContent);
    for(int i = 0; i < 9; i++){
        if(i < blockCount){
            int blockAddr = _blockStart + ((freeBitmap[i].x - 3) + freeBitmap[i].y)*_blockSize;
            fseek(file, blockAddr, SEEK_SET);
            newNode.DIRECT_BLOCKS[i] = (unsigned char) (blockAddr - _blockStart)/_blockSize;
            
            for(int j = 0; j < _blockSize; j++){
                fputc(fileContent[i*_blockSize + j], file);
            }
        }
        else
            newNode.DIRECT_BLOCKS[i] = 0x00;
    }
    
    _inodeArray[freeInode/sizeof(INODE)] = newNode;
    changeInode(file, newNode, freeInode);

    //Registering the new inode in the parent dir

    //CONSIDERING NO INDIRECT BLOCK IMPLEMENTATION

    found = 0;
    for(int i = 0; i < 9; i++){
        found = findFreeBlockByte(file, _inodeArray[parentIndex].DIRECT_BLOCKS[i], (unsigned char) (freeInode/sizeof(INODE)));
        if(found == 1)
            break;
    }
    if(found == 0){
        cout<<"There's no space to add further directories.\n";
        fclose(file);
        return;
    }
    _inodeArray[parentIndex].SIZE += 0x01;

    changeInode(file, _inodeArray[parentIndex], parentIndex*sizeof(INODE));

    if(freeBitmap != nullptr)
        free(freeBitmap);

    cout<<fileName<<" added to parent directory.\n";
    fclose(file);
}

FileSystem* FileSystem::initSystem(char* fileName){
    FILE* file = fopen(fileName, "r");
    int blockSize, blockCount, inodeCount;
    FileSystem* system;
    if(file == NULL){
        cout<<"The file doesn't exist.\n";
        return nullptr;
    }

    fseek(file, 0, SEEK_SET);
    blockSize = fgetc(file);
    fseek(file, 1, SEEK_SET);
    blockCount = fgetc(file);
    fseek(file, 2, SEEK_SET);
    inodeCount = fgetc(file);

    system = new FileSystem(fileName, blockCount, blockSize, inodeCount);
    system->readFile(file);

    fclose(file);

    return system;
}

void FileSystem::readFile(FILE* file){
    _bitmapStart = 3;
    _inodeStart = _bitmapStart + ceil(_blockCount/8.0);
    _rootStart = _inodeStart + (sizeof(INODE) * _inodeCount);
    _blockStart = _rootStart + 1;

    if(_inodeArray != nullptr)
        free(_inodeArray);
    _inodeArray = (INODE*) malloc(sizeof(INODE)*_inodeCount);
    for(int i = 0; i < _inodeCount; i++)
        _inodeArray[i] = *(readInode(file, i*sizeof(INODE)));
}

void FileSystem::changeInode(FILE* file, INODE inode, int addr){
    int seek = fseek(file, _inodeStart + addr, SEEK_SET);

    fputc(inode.IS_USED, file);
    fputc(inode.IS_DIR, file);
    for(int i = 0; i < 10; i++)
        fputc(inode.NAME[i], file);

    fputc(inode.SIZE, file);
    for(int i = 0; i < 9; i++)
        fputc(inode.DIRECT_BLOCKS[i], file);
}

INODE* FileSystem::readInode(FILE* file, int addr){
    fseek(file, _inodeStart + addr, SEEK_SET);
    INODE* newNode = (INODE*) malloc(sizeof(INODE));

    newNode->IS_USED = fgetc(file);
    newNode->IS_DIR = fgetc(file);
    for(int i = 0; i < 10; i++)
        newNode->NAME[i] = fgetc(file);

    newNode->SIZE = fgetc(file);
    for(int i = 0; i < 9; i++)
        newNode->DIRECT_BLOCKS[i] = fgetc(file);

    return newNode;
}

char* FileSystem::readBlock(FILE* file, int addr){
    char* block = (char*) malloc(sizeof(char)*_blockSize);
    fseek(file, _blockStart + addr*_blockSize, SEEK_SET);

    for(int i = 0; i < _blockSize; i++)
        block[i] = fgetc(file);

    return block;
}

int FileSystem::findFreeBlockByte(FILE* file, unsigned char addr, unsigned char data){
    char* block = readBlock(file, addr);

    for(int i = 0; i < _blockSize; i++){
        if(block[i] == 0x00){
            fseek(file, _blockStart + addr*_blockSize + i, SEEK_SET);
            fputc(data, file);
            free(block);
            return 1;
        }
    }
    free(block);
    return 0;
}

vector2 FileSystem::findEmptyBit(FILE* file){
    vector2 info;
    info.x = -1;
    info.y = -1;
    int found;
    for(int i = _bitmapStart; i < _inodeStart; i++){
        fseek(file, i, SEEK_SET);
        unsigned char bitmap = fgetc(file);
        for(int j = 0; j < 8; j++){
            // Find a free bit
            if(((int)bitmap&((unsigned char)pow(2, j))) == 0){
                info.y = j;
                info.x = i;
                found = 1;
                break;
            }
        }
        if(found == 1)
            break;
    }
    if(found == 0)
        cout<<"There's no free space in the system.\n";
    
    return info;
}

int FileSystem::findFreeInode(FILE* file){
    for(int i = _inodeStart; i <_rootStart; i += sizeof(INODE)){
        INODE* curNode = readInode(file, i-_inodeStart);
        if(curNode->IS_USED == 0x00){
            free(curNode);
            return (i - _inodeStart);
        }
        free(curNode);
    }
    cout<<"There's no more room for files in the system.\n";
    return -1;
}

void FileSystem::setBit(FILE* file, int bitmap, int bit){
    fseek(file, bitmap, SEEK_SET);
    unsigned char content = fgetc(file);
    fseek(file, bitmap, SEEK_SET);
    fputc(content+((unsigned char) pow(2, bit)), file);
}

void FileSystem::clearBit(FILE* file, int bitmap, int bit){
    fseek(file, bitmap, SEEK_SET);
    unsigned char content = fgetc(file);
    fseek(file, bitmap, SEEK_SET);
    fputc(content-((unsigned char) pow(2, bit)), file);
}

int FileSystem::findParentDir(char* dirName){
    for(int i = 0; i < _inodeCount; i++){
        if(strcmp(_inodeArray[i].NAME, dirName) == 0){
            return i;
        }
    }

    return -1;
}

int FileSystem::split(char* newString, char* targetString, char token){
    int i;
    memset(newString, 0, CHAR_SIZE);
    for(i = 0; i < CHAR_SIZE; i++){
        if(targetString[i] == token){
            strncpy(newString, targetString, i);
            newString[i+1] = '\0';
            break;
        }
    }
    
    if(i == CHAR_SIZE)
        return 1;
    else{
        memmove(targetString, targetString + i + 1, strlen(targetString));
        return 0; 
    }
}