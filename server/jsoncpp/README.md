# `jsoncpp`库

一个轻量级的开源的C++解析和封装`json`的库，以下是简单的使用示例：

```c++
int main()
{
    // json对象转字符串
    // 创建一个Json::Value对象
    Json::Value obj1;
    // 添加数据到Json对象
    obj1["name"] = "test"; // 字符串
    obj1["age"] = 18; // 数字
    // 生成 JSON 字符串
    std::string str1 = json2str(obj1, true); // 转换为带格式的 JSON 字符串
    str1 = json2str(obj1, false); // 转换为紧凑的 JSON 字符串

    // 字符串转json对象
    std::string str2 = "{\"name\":\"test\",\"age\":18}";
    Json::Value obj2;
    if(str2json(str2, obj2)){
        std::string name = obj2["name"].asString();
        int age = obj2["age"].asInt();
    }else{
        std::cout<<"json字符串解析失败"<<std::endl;
    }
    return 0;
}
```

