#ifndef structs_h
#define structs_h

typedef struct {
    unsigned char IS_USED;             // 0x01 se utilizado, 0x00 se livre
    unsigned char IS_DIR;               // 0x01 se diretorio, 0x00 se arquivo
    char NAME[10];                     // nome do arquivo/dir
    char SIZE;                         // tamanho do arquivo/dir em bytes
    unsigned char DIRECT_BLOCKS[9];
    //unsigned char INDIRECT_BLOCKS[3];
    //unsigned char DOUBLE_INDIRECT_BLOCKS[3];
} INODE;

typedef struct {
    int x;
    int y;
} vector2;

#endif /* inode_h */
