#include "log/logfile.h"
#include "log/logging.h"
#include <libgen.h>

int main(int argc, char *argv[])
{
    char name[256];
    strncpy(name,argv[0],256);

    yamq::log::LogFile file(::dirname(name),basename(name),5*1000);
    const char * line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";

    LOG(TRACE) << "Size:" << strlen(line);

    char data[256];
    
    for (int i=0;i<10000000;++i) {
        snprintf(data,256,"%d:%s",i,line);
        file.append(data,strlen(data));
        if (i == 10) {
            LOG(INFO) << "I:" << i;
            file.roll(false);
        }
        file.flush();
        usleep(2*100*1000);
    }

    return 0;
}
