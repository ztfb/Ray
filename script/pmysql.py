import json
import mysqllib

def json2obj(str):
	# 将json字符串解析为python对象
	# 将从数据库中读出的json字符串解析为对象，然后进行相应的处理
	return json.loads(str)

def obj2json(obj):
	# 将python对象存为json字符串
	# 用于构造HTTP响应（对象转换得到的json字符串将被加到HTTP响应的请求体中）
	return json.dumps(obj)

def query(conn,sql):
	# 返回SQL语句查询得到的结果集
	# 如果data==0，则说明查询失败
	return mysqllib.query(conn,sql)

def getResult(data):
	# 获得结果集中的一条数据
	# 如果result为空，说明所有数据均已经取出
	result=mysqllib.getResult(data)
	return json2obj(result)

def getAllResult(data):
	# 获得结果集中的全部数据
	result=mysqllib.getAllResult(data)
	return json2obj(result)

# 业务层自己构造sql更新（增删改）语句并传入
def update(conn,sql):
	return mysqllib.query(conn,sql)

def insertObj(conn,table,obj):
	# 根据表名和对象（字典）构造insert语句并执行
	strlist=['insert into ',table,' (']

	# 将obj的键（字段加到strlist中）
	ks=list(obj.keys())
	for i in range(len(ks)-1):
		strlist.append(ks[i]+',')
	if(len(ks)>0):
		strlist.append(ks[len(ks)-1])

	strlist.append(') values(')

	# 将obj的值（字段值加到strlist中）
	vs=list(obj.values())
	for i in range(len(vs)-1):
		strlist.append('\''+vs[i]+'\',')
	if(len(vs)>0):
		strlist.append('\''+vs[len(vs)-1]+'\'')

	strlist.append(')')
	sql=''.join(strlist)
	# 将拼接好的SQL送入mysql中执行
	return mysqllib.update(conn,sql)

def insertJson(conn,table,str):
	# 根据表名和json字符串构造insert语句并执行
	obj=json2obj(str);
	return insertObj(conn,table,obj)

def updateObj(conn,table,modObj,condObj):
	# 根据表名，要修改的字段和条件字段构造update语句并执行
	strlist=['update ',table,' set ']
	ks=list(modObj.keys())
	vs=list(modObj.values())
	for i in range(len(ks)-1):
		strlist.append(ks[i]+'='+'\''+vs[i]+'\',')
	if(len(ks)>0):
		strlist.append(ks[len(ks)-1]+'='+'\''+vs[len(ks)-1]+'\'')
	
	# 如果条件字段不为空，则需要继续添加where子句（没有where子句的update会修改所有行）
	if len(condObj)>0:
		strlist.append(' where ')
		ks=list(condObj.keys())
		vs=list(condObj.values())
		for i in range(len(ks)-1):
			strlist.append(ks[i]+'='+'\''+vs[i]+'\' and ')
		if(len(ks)>0):
			strlist.append(ks[len(ks)-1]+'='+'\''+vs[len(ks)-1]+'\'')
	sql=''.join(strlist)
	# 将拼接好的SQL送入mysql中执行
	return mysqllib.update(conn,sql)

def updateJson(conn,table,modStr,condStr):
	modObj=json2obj(modStr)
	condObj=json2obj(condStr)
	return updateObj(conn,table,modObj,condObj)

def deleteObj(conn,table,condObj):
	# 根据表名和条件字段构造delete语句并执行
	strlist=['delete from ',table]
	# 如果条件字段不为空，则需要继续添加where子句（没有where子句的delete会删除整张表）
	if len(condObj)>0:
		strlist.append(' where ')
		ks=list(condObj.keys())
		vs=list(condObj.values())
		for i in range(len(ks)-1):
			strlist.append(ks[i]+'='+'\''+vs[i]+'\' and ')
		if(len(ks)>0):
			strlist.append(ks[len(ks)-1]+'='+'\''+vs[len(ks)-1]+'\'')
	sql=''.join(strlist)
	return mysqllib.update(conn,sql)

def deleteJson(conn,table,condStr):
	condObj=json2obj(condStr)
	return deleteObj(conn,table,condObj)