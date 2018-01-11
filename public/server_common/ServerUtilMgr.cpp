#include "ServerUtilMgr.h"
#include "common/Debug_log.h"

unsigned char* ServerUtilMgr::GetData(const string &filename, bool for_string, int &buf_len)
{
    if(filename.empty())
    {
        return NULL;
    }

    const char* mode = NULL;
    if(for_string)
        mode = "rt";
    else
        mode = "rb";

    int readsize;
    unsigned char *buf = NULL;
    do
    {
        FILE *fp = fopen(filename.c_str(), mode);
        if(!fp)
        {
            break;
        }
        fseek(fp,0,SEEK_END);
        buf_len = ftell(fp);
        fseek(fp,0,SEEK_SET);

        if (for_string)
        {
            buf = (unsigned char*)malloc(sizeof(unsigned char) * (buf_len + 1));
            buf[buf_len] = '\0';
        }
        else
        {
            buf = (unsigned char*)malloc(sizeof(unsigned char) * buf_len);
        }

        readsize = fread(buf, sizeof(unsigned char), buf_len, fp);
        fclose(fp);

        if (for_string && readsize < buf_len)
        {
            buf[readsize] = '\0';
        }
    } while (0);

    if (NULL == buf || 0 == readsize)
    {
        return NULL;
    }

    printf("file_name = %s, read size = %d \n", filename.c_str(), readsize);
    return buf;
}
