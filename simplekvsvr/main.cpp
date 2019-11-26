#include<iostream>
#include"string"
#include<cstring>
#include<cstdio>
#include"KVSvr.h"
#include"workqueue.h"
#include"ThreadPool.h"
#include"Worker.h"
#include"NewKVIndex.h"

using namespace std;
class KVWorker : public Worker{
public:
	KVWorker(KVIndex *kv,workqueue<pair<string,int> > *from_queue){
		kv_=kv;
		from_queue_=from_queue;
	}
	int Process(){
		while(1)
		{
			pair<string,int> pa=from_queue_->Pop();
            /* TODO 容错性
             * cstr == NULL 的情况要处理下
             * */
			char *cstr = new char[pa.first.length()+1];
            /* TODO 资源管理
             * 内存泄漏
             * */
			strcpy(cstr,pa.first.c_str());
			if(cstr[pa.first.length()-1] == '\n')
			{
				cstr[pa.first.length()-1] = 0;
			}






			string ret;
			const char delim[2]=" ";
			char *p_save = NULL;
			char *cur=strtok_r(cstr,delim,&p_save);

			//puts("come to thread");
			//cout<<pa.first<<endl;

			if (strcmp(cur, "quit") == 0)
			{
				ret = "OK";
			}else if (strcmp(cur,"get") == 0)
			{
				string key = strtok_r(NULL,delim,&p_save);
				//cout<<"getkeylen:"<<key.length()<<endl;
				if (kv_->Exist(key))
				{
					ret = kv_->Get(key);
				}else {
					ret = "NULL";
				}
			}else if(strcmp(cur,"delete") == 0)
			{
				string key = strtok_r(NULL,delim,&p_save);
				int retnum = kv_->Delete(key);
				if(retnum == 0)
					ret = "OK";
				else ret = "error";
			}else if(strcmp(cur,"stats") == 0)
			{
				ret = kv_->GetStats();
			}else if(strcmp(cur,"set") == 0){
				string key = strtok_r(NULL,delim,&p_save);
				//cout<<"setkeylen:"<<key.length()<<endl;
				string value = strtok_r(NULL,delim,&p_save);
				//cout<<"setvallen:"<<value.length()<<endl;
				if(kv_->Set(key,value) == 0)
				{
					ret = "OK";
				}else {
					ret = "error";
				}
			}else {
				ret = "command not found";
			}
			char line[2]="\n";
			 //cout<<kv_->GetStats()<<endl;
            /* TODO 容错性
             * write 返回码需要处理下
             * 返回值 < ret.length() 的情况下需要再写一次
             * */
			write(pa.second, ret.c_str(), ret.length());
			write(pa.second, line, 1);
			close(pa.second);
		}
		return 0;
	}
private:
	KVIndex *kv_;
	workqueue<pair<string,int> > *from_queue_;
};
int main(){
	KVIndex kv;
	kv.Start();
	workqueue<pair<string,int> > from_queue;
	KVWorker kvworker(&kv,&from_queue);
	char ip[20]="127.0.0.1";
	KVSvr kvsvr(7777,ip,&from_queue);
	ThreadPool threadpool(5,&kvworker);
	threadpool.Start();
	kvsvr.Run();
	threadpool.Stop();
	return 0;
}
