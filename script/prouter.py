import pscript

# 路由表
mapper={
    'GET':{
        '/user/login':pscript.login,
        '/user/logout':pscript.logout
    },
    'POST':{
        '/user/register':pscript.register,
        '/user/update':pscript.update,
    }
}

# 路由函数
def router(conn, method, url, options):
    functions = mapper[method]
    if url in functions.keys():
        function = functions[url]
        return function(conn, options)
    else:
        return '404'