#include <boost/python.hpp>
#include <mysql/mysql.h>
#include <string>

bool update(long conn,const char *sql){
	// 该函数可以用来执行增删改语句
    int result = mysql_real_query((MYSQL*)conn,sql,strlen(sql));
	// SQL语句执行成功，result为0，返回true；否则返回false
	return (result==0);
}

long query(long conn,const char *sql){
	// 该函数用来执行查询语句
	int result = mysql_real_query((MYSQL*)conn,sql,strlen(sql));
	if(result!=0)return 0; // 查询失败直接返回0
	else{ // 查询成功
		MYSQL_RES *data=mysql_store_result((MYSQL*)conn); // 获取查询结果集
		// 将指针封装成long类型的数据并返回
		return (long)(data);
	}
}

std::string getResult(long data){
	// 获取查询结果（将查询结果封装成JSON返回）
	// 如果所有的结果都已经取出，则返回NULL
	MYSQL_ROW row=mysql_fetch_row((MYSQL_RES*)data); // 从结果集中取出一行
	if(row==NULL)return "";
	std::string result="{";
	int fields=mysql_num_fields((MYSQL_RES*)data); // 获取结果集的字段数
	MYSQL_FIELD* field;
	for(int i=0;i<fields;i++){
		result.push_back('\"');
    	field = mysql_fetch_field_direct((MYSQL_RES*)data,i);
		result.append(field->name);
		result.append("\":\"");
		result.append(row[i]);
		result.append("\",");
	}
	result.pop_back();
	result.push_back('}');
	return result;
}

std::string getAllResult(long data){
	// 将查询结果全部取出（封装成JSON数组并返回）
	std::string allResult="[";
	std::string result=getResult(data);
	if(result.empty())return "[]"; // 直接返回一个空数组
	while(!result.empty()){
		allResult.append(result);
		allResult.push_back(',');
		result=getResult(data);
	}
	allResult.pop_back();
	allResult.push_back(']');
	return allResult;
}
BOOST_PYTHON_MODULE(mysqllib)
{
	using namespace boost::python;
	def("update",update);
	def("query",query);
	def("getResult",getResult);
	def("getAllResult",getAllResult);
}
