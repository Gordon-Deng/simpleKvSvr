#include "cstring"
#include "string"

using namespace std;

class DataNode{
private:
    char key_[30], value_[30];
    bool flag_;
public:
    bool operator == (const DataNode& a)const{
        if(strcmp(key_, a.key_)!=0 || strcmp(value_, a.value_)!=0 || flag_!=a.flag_)
            return false;
        else return true;
    }
    DataNode(){}
    DataNode(string key,string value){
        strcpy(key_, key.c_str());
        strcpy(value_,value.c_str());
        flag_=1;
    }
    void Expire(){
        flag_=0;
    }
    bool IsValid(){
        return flag_;
    }
    string Getkey(){
        string ret = key_;
        return ret;
    }
    string Getvalue(){
        string ret = value_;
        return ret;
    }
};
