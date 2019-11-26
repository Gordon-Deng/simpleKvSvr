#include "task.h"
#include "simplekvstore.h"
#include "kvlogiclayer.h"
#include "kvnetworklayer.h"

#define PORT 7777
static const char* localIp = "127.0.0.1";

int main()
{
    SimpleKVStore* store = new SimpleKVStore(1024);
    Task* task;

    KVLogicLayer* logic = new KVLogicLayer(store, 1024, 1, 1);
    KVNetworkLayer* network = new KVNetworkLayer(logic, 1024, 100, 1, 1);
    network->run(PORT, localIp);
}
