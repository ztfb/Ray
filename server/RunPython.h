#ifndef RUNPYTHON
#define RUNPYTHON

#include <map>
#include <Python.h>
#include <memory>
#include <string>
#include "Log.h"

class RunPython{
public:
    // python脚本运行器
    static std::shared_ptr<RunPython> instance(); // 获取RunPython的单例对象

    void init(const std::string& scriptPath); // 初始化python解释器，scriptPath为脚本路径
    void loadModule(const std::string& moduleName); // 根据脚本（模块）名载入模块
    void loadFunction(const std::string& moduleName,const std::string& funcName); //根据模块名和函数名导入函数
    PyObject* initArgs(int size); // 根据参数个数初始化参数列表
    template<typename T>
    void buildArgs(PyObject* args,int i,T value){ // 设置参数列表第i个参数的值
        std::string type;
	    if(std::is_same<T,char>::value)type="c";
	    else if(std::is_same<T,short>::value)type="h";
	    else if(std::is_same<T,int>::value)type="i";
	    else if(std::is_same<T,long>::value)type="l";
	    else if(std::is_same<T,double>::value)type="d";
	    else if(std::is_pointer<T>::value)type="s";
	    PyTuple_SetItem(args,i,Py_BuildValue(type.c_str(), value));
    }
    // 传入一个T类型的参数ret，用于获得返回值。不需要返回值时，传入NULL
    template<typename T>
    void callFunc(const std::string& moduleName,const std::string& funcName,PyObject* args,T& ret){
        // 运行函数，并获取返回值
	    PyObject* pFunc=functions[{moduleName,funcName}];
	    PyObject* pRet = PyObject_CallObject(pFunc, args);
	    if(pRet){ // 有返回值
		    if(std::is_same<T,char>::value)PyArg_Parse(pRet,"c",&ret);
		    else if(std::is_same<T,short>::value)PyArg_Parse(pRet,"h",&ret);
		    else if(std::is_same<T,int>::value)PyArg_Parse(pRet,"i",&ret);
		    else if(std::is_same<T,long>::value)PyArg_Parse(pRet,"l",&ret);
		    else if(std::is_same<T,double>::value)PyArg_Parse(pRet,"d",&ret);
		    else if(std::is_pointer<T>::value)PyArg_Parse(pRet,"s",&ret);
	    }
    }
    ~RunPython();

    RunPython(const RunPython&) = delete; // 禁用拷贝构造函数
    RunPython& operator=(const RunPython&) = delete; // 禁用赋值运算符
private:
    // 有const或引用类型的成员时，不能使用default默认初始化，否则编译器将把构造函数标记为delete
    RunPython(); // 禁用外部构造
    std::map<std::string,PyObject*> modules; // 模块名到模块指针的映射
    std::map<std::pair<std::string,std::string>,PyObject*> functions; // 模块名和函数名到函数指针的映射
};

#endif