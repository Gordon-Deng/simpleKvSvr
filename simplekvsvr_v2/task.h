#ifndef TASK_H
#define TASK_H

#define GET 0
#define SET 1
#define DEL 2
#define STATS 3
#define QUIT 4

#define SUCCESS 0
#define FAIL 1

class Task
{
private:
public:
    char buf[4096];
    char res[4096];
    int CMD;
    char* key;
    int keyLen;
    char* value;
    int valueLen;
    int state;
    int fd;
    int n;
    int resLen;
    bool cached;
    Task() { cached = false; }
};

#endif
