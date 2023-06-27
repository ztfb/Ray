#include "RunPython.h"

static std::shared_ptr<RunPython> runPython=nullptr;
static std::mutex mutex;

RunPython::RunPython(){}

std::shared_ptr<RunPython> RunPython::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(runPython==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(runPython==nullptr){
            runPython=std::shared_ptr<RunPython>(new RunPython());
        }
    }
    return runPython;
}

void RunPython::init(const std::string& scriptPath){
    // 初始化python解释器
	Py_Initialize();
	if(!Py_IsInitialized()){
        log_error("python解释器初始化失败...");
		return;
	}
	// 初始化python系统文件路径，保证可以访问到.py文件
	PyRun_SimpleString("import sys");
	std::string cmd="sys.path.append('"+scriptPath+"')";
	PyRun_SimpleString(cmd.c_str());	
}

void RunPython::loadModule(const std::string& moduleName){
    // 导入python模块，模块和脚本同名，不用写.py后缀
	// python脚本不能直接以test命名
	PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
	if(pModule==NULL){
        log_warn("没有找到【"+moduleName+"】模块...");
		return ;
	}
    // 将成功导入的模块添加到映射中
    modules[moduleName]=pModule;
}

void RunPython::loadFunction(const std::string& moduleName,const std::string& funcName){
    PyObject* pFunc = PyObject_GetAttrString(modules[moduleName],funcName.c_str());
	if(!pFunc||!PyCallable_Check(pFunc)){
		log_warn("没有找到【"+moduleName+"】模块中的【"+funcName+"】函数...");
		return ;
	}
    // 将成功导入的函数添加到映射中
    functions[std::pair<std::string,std::string>{moduleName,funcName}]=pFunc;
}

PyObject* RunPython::initArgs(int size){
	// 初始化参数列表
	PyObject* args = PyTuple_New(size);
	return args;
}

RunPython::~RunPython(){
    // 关闭python解释器
	Py_Finalize();
}