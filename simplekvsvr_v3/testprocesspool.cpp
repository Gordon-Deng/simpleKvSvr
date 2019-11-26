
#include "processpool.h"
#include <iostream>

using namespace mminternpractice;

/**
 * 测试用Task
 */
class TestTask : public Task {
public:
	TestTask(int value) : m_value(value)
	{}

	virtual ~TestTask() 
	{}

    virtual int DoTask(void *ptr) 
	{
  		int oldValue = m_value;
        m_value = -m_value;
        std::cout << "\tdo work " << oldValue << " -> " << m_value << std::endl;
        return 0; 
    }
	void Debug()
	{
		std::cout << "\t" << m_value << std::endl;
	}
private:
	int m_value;
};


int main(int argc, char** argv) 
{
	if(signal(SIGCHLD , SIG_IGN) == SIG_ERR)
		return -1;

	const long key = ftok("./process_file_key" , 0);	

    TaskProcessPool pool(key , 100, NULL);

    if (pool.Start(3) != 0) {
        std::cout << "Test Start ERROR" << std::endl;
        return 1;
    }

	for(int i = 0 ; i < 100 ; ++i)
	{
		TestTask t(i);
		if(pool.AddTask(t , sizeof(t)) != 0)
		{
			printf("Add Task Error!\n");
			return -1;
		}
	}

	sleep(2);

    if (pool.KillAll() != 0) {
        std::cout << "Test Start ERROR" << std::endl;
        return 2;
    }

    return 0;
}
