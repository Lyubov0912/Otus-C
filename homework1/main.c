#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(1)
typedef struct EOCD
{
    // Обязательная сигнатура, равна 0x06054b50
    uint32_t signature;
    // Номер диска
    uint16_t diskNumber;
    // Номер диска, где находится начало Central Directory
    uint16_t startDiskNumber;
    // Количество записей в Central Directory в текущем диске
    uint16_t numberCentralDirectoryRecord;
    // Всего записей в Central Directory
    uint16_t totalCentralDirectoryRecord;
    // Размер Central Directory
    uint32_t sizeOfCentralDirectory;
    // Смещение Central Directory
    uint32_t centralDirectoryOffset;
    // Длина комментария
    uint16_t commentLength;
    // Комментарий (длиной commentLength)
    uint8_t *comment;
}eocd;

typedef struct CentralDirectoryFileHeader
{
    // Обязательная сигнатура, равна 0x02014b50
    uint32_t signature;
    // Версия для создания
    uint16_t versionMadeBy;
    // Минимальная версия для распаковки
    uint16_t versionToExtract;
    // Битовый флаг
    uint16_t generalPurposeBitFlag;
    // Метод сжатия (0 - без сжатия, 8 - deflate)
    uint16_t compressionMethod;
    // Время модификации файла
    uint16_t modificationTime;
    // Дата модификации файла
    uint16_t modificationDate;
    // Контрольная сумма
    uint32_t crc32;
    // Сжатый размер
    uint32_t compressedSize;
    // Несжатый размер
    uint32_t uncompressedSize;
    // Длина название файла
    uint16_t filenameLength;
    // Длина поля с дополнительными данными
    uint16_t extraFieldLength;
    // Длина комментариев к файлу
    uint16_t fileCommentLength;
    // Номер диска
    uint16_t diskNumber;
    // Внутренние аттрибуты файла
    uint16_t internalFileAttributes;
    // Внешние аттрибуты файла
    uint32_t externalFileAttributes;
    // Смещение до структуры LocalFileHeader
    uint32_t localFileHeaderOffset;
    // Имя файла (длиной filenameLength)
    uint8_t* filename;
    // Дополнительные данные (длиной extraFieldLength)
    uint8_t* extraField;
    // Комментарий к файла (длиной fileCommentLength)
    uint8_t* fileComment;
}central_header;
#pragma pack()

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("USAGE: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE* file = fopen(argv[1], "rb");

    if(!file)
    {
        printf("Failed to open %s for reading!\n", argv[1]);
        return EXIT_FAILURE;
    }

    // TODO: ваш код
    eocd cdg;
    uint32_t buf;
    fseek(file, -4, SEEK_END);
    fread(&buf, sizeof(uint32_t), 1, file);

    while((buf!=0x06054b50)&&(!feof(file)))
    {
        fseek(file, -5, SEEK_CUR);
        fread(&buf, sizeof(uint32_t), 1, file);
    }

    if (buf!=0x06054b50)
    {
        printf("This is not an archive!\n");
        return EXIT_FAILURE;
    }

    fseek(file, -4, SEEK_CUR);
    fread(&cdg, sizeof(eocd), 1, file);
    fseek(file, cdg.centralDirectoryOffset, SEEK_SET);
    central_header header;
    char s[128]="";

    for (int i=0; i<cdg.numberCentralDirectoryRecord; ++i)
    {
        fread(&header, 46, 1, file);
        if (header.signature != 0x02014b50)
        {
            printf("This is not an archive!\n");
            return EXIT_FAILURE;
        }
        fread(&s, header.filenameLength, 1, file);
        if (s[header.filenameLength-1] != '/')
            printf("%s\n",s);
        memset(s, 0, 128);
        fseek(file, header.extraFieldLength+header.fileCommentLength, SEEK_CUR);
    }

    return EXIT_SUCCESS;
}
