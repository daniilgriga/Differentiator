#ifndef FILE_DATA_H_
#define FILE_DATA_H_

char* file_reader (struct Buffer_t* buffer, const char* filename);

int count_symbols (struct Buffer_t* buffer, FILE* file);

#endif // FILE_DATA_H_
