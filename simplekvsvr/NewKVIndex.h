#ifndef KVINDEX
#define KVINDEX

#include<cstring>
#include<sys/mman.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<algorithm>
#include<mutex>
#include<map>
#include "DataNode.h"
#include<list>
#include<memory>

const int CACHE_SIZE=1500;
using namespace std;

class KVIndex{
private:
	map<string,pair<int,int> > kv_;
	int file_size_;
	int size_;
	int fd_;
	int cache_index_=-1;
	int request_ = 0, hit_ = 0, cache_hit_ = 0, delete_count_ = 0;
	DataNode cache_[CACHE_SIZE];
	map<string,pair<int,int> >::iterator cache_element_[CACHE_SIZE];
	DataNode *store_;
	recursive_mutex mutex_;
public:
	void MemoryRecycle(){
		int new_size=0;
		kv_.clear();
		for(int i = 0; i < size_; ++i){
			if(store_[i].IsValid() == false)
				continue;
			string key = store_[i].Getkey();
			store_[new_size] = store_[i];
			kv_[key].first = -1;
			kv_[key].second = new_size;
			new_size++;
		}
		size_ = new_size;
		DataNode end;
		memset(&end,0,sizeof end);
		for(int i = size_; i < file_size_; ++i){
			store_[i] = end;
		}
	}
	string GetStats(){
		int mem_size = sizeof(kv_.begin())*kv_.size();
		string count = "count: " + to_string(kv_.size());
		string mem = ", mem: " + to_string(mem_size);
		string files = ", file: " + to_string(file_size_*sizeof(DataNode));
		string hit = ", hits: " + to_string(hit_);
		string miss = ", misses: " + to_string(request_ - hit_);
		string cache_hit = ", cache_hits: " + to_string(cache_hit_);
		string cache_miss = ", cache_misses: " + to_string(hit_ - cache_hit_);
		return count + mem + files + hit + miss + cache_hit + cache_miss;
	}
	/*
	void ShowMemList(){
		//return ;
		puts("-------MemList----------");
		for(auto ele : mem_list_)
		{
			printf("%d %d\n", ele.first, ele.second);
		}
		puts("-------MemList----------");
	}
	*/
	int Start(){
		int file_sizec = -1;
		FILE *fp;
		fp = fopen("data", "r");
		if(fp != NULL)
		{
			fseek(fp, 0L, SEEK_END);
			file_sizec = ftell(fp);
			fclose(fp);
		}
		fd_=open("data",O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if(fd_<0){
			cerr<<"failed to open data"<<endl;
			return -1;
		}
		if(file_sizec == -1)
		{
			file_size_ = 4;
		}else{
			file_size_ = file_sizec/sizeof(DataNode);
		}
		if(ftruncate(fd_,file_size_*sizeof(DataNode))<0){
			cerr<<"failed to ftruncate"<<endl;
			return -2;
		}
		store_=(DataNode *)mmap(NULL,file_size_*sizeof(DataNode),
				PROT_READ | PROT_WRITE,MAP_SHARED,fd_,0);
		if(store_==MAP_FAILED){
			cerr<<"failed to mmap file"<<endl;
			return -3;
		}

		DataNode end;
		memset(&end,0,sizeof end);
		size_=0;
		for(int i = 0; i < file_size_; ++i){
			if (store_[i] == end)
				break;
			size_ = i + 1;
			if(store_[i].IsValid() == false)
				continue;
			string key = store_[i].Getkey();
			kv_[key].first = -1;
			kv_[key].second = i;
		}


		for(int i=0;i<CACHE_SIZE;++i){
			cache_element_[i] = kv_.end();
		}
		//ShowMemList();
		//ShowStore();
	}
	int Resize(){
		mutex_.lock();
		munmap(store_, file_size_*sizeof(DataNode));
		printf("fd=%d file_size_=%d\n",fd_, file_size_);
		if (ftruncate(fd_, file_size_*sizeof(DataNode)*2)<0){
			cerr<<"failed to ftruncate in resize"<<endl;
			cerr<<"erron="<<errno<<endl;
            /* TODO 资源管理
             * mutex_可以在函数的开头用unique_lock封装起来，避免每次return都写unlock
             * */
			mutex_.unlock();
			return -1;
		}
		store_ = (DataNode *)mmap(NULL,file_size_*sizeof(DataNode)*2,
				PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
		if (store_ == MAP_FAILED){
			cerr<<"failed to mmap file"<<endl;
			mutex_.unlock();
			return -1;
		}

		file_size_*=2;
		printf("file_size_ resize %d\n",file_size_);
		mutex_.unlock();
		return 0;
	}
	int Set(string key,string value){
		mutex_.lock();

		if (size_ == file_size_)
		{
			if(Resize() < 0){
				mutex_.unlock();
				return -1;
			}
		}
		if(kv_.count(key) == 1)
		{
			Delete(key);
		}
		store_[size_] = DataNode(key, value);
		kv_[key].first = -1;
		kv_[key].second = size_;
		size_++;
		//ShowStore();
		//ShowMemList();
		//ShowMap();
		//puts("");

		mutex_.unlock();
		return 0;
	}
	int Delete(string key){
		mutex_.lock();

		store_[kv_[key].second].Expire();
		if(kv_[key].first!=-1)
			EraseCache(kv_[key].first);
		kv_.erase(key);
		//cout<<"del:"<<key<<endl;
		//ShowStore();
		//ShowMemList();
		//puts("");
		delete_count_++;
		if(delete_count_ == 1000)
		{
			delete_count_ = 0;
			//MemoryRecycle();
		}
		mutex_.unlock();
		return 0;
	}
	/*
	void ShowMap(){
		//return ;
		puts("----------Map----------");
		for(auto ele:kv_){
			cout<<ele.first<<" "<<ele.second.first<<" "<<ele.second.second<<endl;
		}
		puts("----------Map----------");
	}
	void ShowStore()
	{
		//return ;
		puts("---store----");
		for(int i=0;i<file_size_;++i)
		{
			if(store_[i]==0)
				printf("*");
			else printf("%c", store_[i]);
		}
		puts("");
		puts("---store----");
	}
	*/
	string Get(string key){
		mutex_.lock();
		hit_++;
		int cache_id = kv_[key].first;
		string ret;
		if(cache_id != -1)
		{
			cache_hit_++;
			ret = cache_[cache_id].Getvalue();
		}else{
			ret = store_[kv_[key].second].Getvalue();
		}

		cache_index_ = (cache_index_ + 1) % CACHE_SIZE;
		EraseCache(cache_index_);

		cache_[cache_index_] = store_[kv_[key].second];
		kv_[key].first=cache_index_;
		cache_element_[cache_index_]=kv_.find(key);


		mutex_.unlock();

		return ret;
	}
	void EraseCache(int cache_index){
		mutex_.lock();
		if(cache_element_[cache_index]!=kv_.end()){
			(cache_element_[cache_index])->second.first=-1;
			cache_element_[cache_index]=kv_.end();
		}
		mutex_.unlock();
	}
	bool Exist(string key){
		mutex_.lock();
		request_++;
		if (kv_.find(key) != kv_.end())
		{
			mutex_.unlock();
			return true;
		} else
		{
			mutex_.unlock();
			return false;
		}
	}
};


#endif
