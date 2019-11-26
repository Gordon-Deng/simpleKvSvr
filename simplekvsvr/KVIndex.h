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
#include<list>
#include<memory>

const int CACHE_SIZE=1500;
using namespace std;

class KVIndex{
private:
	map<string,pair<int,int> > kv_;
	int file_size_;
	int fd_;
	int cache_index_=-1;
	int request_ = 0, hit_ = 0, cache_hit_ = 0;
	char *cache_[CACHE_SIZE];
	map<string,pair<int,int> >::iterator cache_element_[CACHE_SIZE];
	char *store_;
	list<pair<int,int> > mem_list_;
	recursive_mutex mutex_;
public:
	void Quit(){
		puts("begin quit");
		freopen("key","w",stdout);
		cout<<file_size_<<endl;
		for(auto kv : kv_)
		{
			cout<< kv.first <<" "<< kv.second.second<<endl;;
		}
		puts("here");
		//fclose(stdout);
		freopen("/dev/tty","w",stdout);
		close(fd_);
		mem_list_.clear();
		puts("quit ok");
	}
	string GetStats(){
		char file[20]="key";
		FILE *pfile;
		pfile = fopen(file, "rb");
		fseek(pfile, 0, SEEK_END);
		int file_size = ftell(pfile);
		fclose(pfile);

		string count = "count: " + to_string(kv_.size());
		printf("file_size_ %d\n",file_size_);
		string mem = ", mem: " + to_string(file_size_);
		string files = ", file: " + to_string(file_size);
		string hit = ", hits: " + to_string(hit_);
		string miss = ", misses: " + to_string(request_ - hit_);
		string cache_hit = ", cache_hits: " + to_string(cache_hit_);
		string cache_miss = ", cache_misses: " + to_string(hit_ - cache_hit_);
		return count + mem + files + hit + miss + cache_hit + cache_miss;
	}
	void ShowMemList(){
		//return ;
		puts("-------MemList----------");
		for(auto ele : mem_list_)
		{
			printf("%d %d\n", ele.first, ele.second);
		}
		puts("-------MemList----------");
	}
	int Start(){
		freopen("key","r",stdin);
		string key;
		int offset;
		cin>>file_size_;
		cout<<"cin "<<file_size_<<endl;
		while(cin>>key>>offset){
			kv_[key]=make_pair(-1,offset);
		}
		freopen("/dev/tty","r",stdin);
		fd_=open("data",O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		printf("start fd=%d\n",fd_);
		if(fd_<0){
			cerr<<"failed to open data"<<endl;
			return -1;
		}
		if(ftruncate(fd_,file_size_)<0){
			cerr<<"failed to ftruncate"<<endl;
			return -2;
		}
		store_=(char *)mmap(NULL,file_size_,
				PROT_READ | PROT_WRITE,MAP_SHARED,fd_,0);
		if(store_==MAP_FAILED){
			cerr<<"failed to mmap file"<<endl;
			return -3;
		}
		int head,tail;
		for(int i=0;i<file_size_;++i)
		{
			//printf("%d\n",store_[i]);
			if(store_[i]!=0) continue;
			//puts("inside");
			if(i==0 || store_[i-1]!=0){
				if(i==0){
					head=i;
				}else head=i+1;
			}
			if(i==file_size_-1 || store_[i+1]!=0){
				tail=i;
				//printf("push %d %d\n",head,tail);
				if(tail>=head)
					mem_list_.push_back(make_pair(head, tail));
			}
		}
		for(int i=0;i<CACHE_SIZE;++i){
			cache_element_[i] = kv_.end();
		}
		ShowMemList();
		ShowStore();
	}
	int Resize(){
		mutex_.lock();
		munmap(store_, file_size_);
		printf("fd=%d file_size_=%d\n",fd_, file_size_);
		if (ftruncate(fd_, file_size_*2)<0){
			cerr<<"failed to ftruncate in resize"<<endl;
			cerr<<"erron="<<errno<<endl;
			mutex_.unlock();
			return -1;
		}
		store_ = (char *)mmap(NULL,file_size_*2,
				PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
		if (store_ == MAP_FAILED){
			cerr<<"failed to mmap file"<<endl;
			mutex_.unlock();
			return -1;
		}

		if (mem_list_.size() && mem_list_.back().second == file_size_-1)
		{
			mem_list_.back().second = file_size_*2 - 1;
		}else {
			mem_list_.push_back(make_pair(file_size_, file_size_ * 2 - 1));
		}

		file_size_*=2;
		printf("file_size_ resize %d\n",file_size_);
		mutex_.unlock();
		return 0;
	}
	int Insert(string key,string value){
		mutex_.lock();
		char *c_value=(char *)value.c_str();
		int len=value.length()+1;
		bool find=0;
		while(find==0)
		{
			for(auto it=mem_list_.begin();it!=mem_list_.end();it++){
				int head=it->first,tail=it->second;
				if(tail-head+1<len) continue;
				find=1;
				kv_[key]=make_pair(-1,head);
				strcpy(store_+head,c_value);
				if(tail-head+1==len)
				{
					mem_list_.erase(it++);
				}else{
					it->first+=len;
				}
				//printf("insert %d %d\n", head, head+len-1);
				break;
			}
			if(find==0){
				if(Resize()<0){
					mutex_.unlock();
					return -1;
				}
			}
		}

		//cout<<"insert:"<<value<<endl;
		ShowStore();
		ShowMemList();
		ShowMap();
		//puts("");

		mutex_.unlock();
		return 0;
	}
	int Delete(string key){
		mutex_.lock();
		if (kv_.find(key) == kv_.end())
		{
			mutex_.unlock();
			return -1;
		}
		char *value=store_+kv_[key].second;
		int len=strlen(value);
		int head=kv_[key].second;
		int tail=kv_[key].second+len;
		auto p=mem_list_.begin();
		for(auto it = mem_list_.begin() ;1;++it){
			if(it==mem_list_.end() || it->first > tail){
				mem_list_.insert(it,make_pair(head,tail));
				p=it;
				p--;
				break;
			}
		}
		if(p != mem_list_.begin())
		{
			auto pre = p;
			pre--;
			if(p->first == pre->second+1){
				p->first = pre->first;
				mem_list_.erase(pre);
			}
		}
		auto nxt = p;
		nxt++;
		if(nxt != mem_list_.end() && p->second+1 == nxt->first)
		{
			p->second = nxt->second;
			mem_list_.erase(nxt);
		}
		memset(value, 0, strlen(value));
		if(kv_[key].first!=-1)
			EraseCache(kv_[key].first);
		kv_.erase(key);
		//cout<<"del:"<<key<<endl;
		ShowStore();
		ShowMemList();
		//puts("");
		mutex_.unlock();
		return 0;
	}
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
	string Get(string key){
		mutex_.lock();
		hit_++;
		int cache_id = kv_[key].first;
		string ret;
		char *tar_str;
		if(cache_id != -1)
		{
			cache_hit_++;
			tar_str = cache_[cache_id];
		}else{
			tar_str = store_+kv_[key].second;
		}
		char *c_value = new char[strlen(tar_str)+1];
		strcpy(c_value, tar_str);

		cache_index_ = (cache_index_ + 1) % CACHE_SIZE;
		EraseCache(cache_index_);

		cache_[cache_index_] = new char[strlen(c_value)+1];
		strcpy(cache_[cache_index_], c_value);
		kv_[key].first=cache_index_;
		cache_element_[cache_index_]=kv_.find(key);

		ret = c_value;

		mutex_.unlock();

		//debug
		printf("file_size_ in get %d\n",file_size_);

		//cout<<"get:"<<key<<" "<<ret<<endl;
		//puts("");
		return ret;
	}
	void EraseCache(int cache_index){
		mutex_.lock();
		if(cache_element_[cache_index]!=kv_.end()){
			delete cache_[cache_index];
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
	int Set(string key,string value){
		mutex_.lock();
		if(kv_.find(key)!=kv_.end())
			Delete(key);
		if(Insert(key,value)<0){
			mutex_.unlock();
			return -1;
		}
		mutex_.unlock();
		return 0;
	}
};


#endif
