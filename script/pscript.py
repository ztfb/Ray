import pmysql

def login(conn, options):
    data=pmysql.queryJson(conn,"user",options)
    if data==0:
        raise Exception("select语句执行失败")
    result=pmysql.getResult(data)
    if result!=None:
        return pmysql.obj2json({'msg':'登陆成功'})
    else:
        return pmysql.obj2json({'msg':'用户名或密码不正确'})

def register(conn,options):
    obj=pmysql.json2obj(options)
    data=pmysql.queryObj(conn,"user",{'username':obj['username']})
    if data==0:
        raise Exception("select语句执行失败")
    result=pmysql.getResult(data)
    if result!=None:
        return pmysql.obj2json({'msg':'用户名已存在'})
    else:
        result=pmysql.insertJson(conn,"user",options)
        if result:
            return pmysql.obj2json({'msg':'注册成功'})
        else:
            raise Exception("insert语句执行失败")

def update(conn,options):
    obj=pmysql.json2obj(options)
    condObj={'id':obj['id']}
    obj.pop('id')
    result=pmysql.updateObj(conn,"user",obj,condObj)
    if result:
        return pmysql.obj2json({'msg':'用户信息更新成功'})
    else:
        raise Exception("update语句执行失败")

def logout(conn,options):
    result=pmysql.deleteJson(conn,"user",options)
    if result:
        return pmysql.obj2json({'msg':'用户注销成功'})
    else:
        raise Exception("delete语句执行失败")