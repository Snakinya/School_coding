#sql注入检测业务
import re
import random
import bs4
from lib.core import Downloader
#根据url检测sql是否可以注入




def sqlcheck(url):
    print("检测开始")

    if (url.find("?")==-1):
        # url中没有问号，不能通过get方式请求
        return False


    #开始业务处理
    BOOLEAN_TESTS=("AND %d=%d","OR NOT (%d=%d)")
    #定义数据库报错的
    DBMS_ERRORS={
        "MySQL": (r"SQL syntax. *MySQL", r"Warning. *mysql_. *", r"valid. MySQL，result", r"MySq1Client\."),
        "PostgreSQL":(r"PostgreSQL. *ERROR", r"Warning. *\Wpg_ . *",r"valid.PostgreSQL . result", r"Npgsql\."),
        "Microsoft SQL Server": (r"Driver. *.SQL[\-\_ \.]*Server", r"OLE .DB. *.SQL .Server", r"(\W/ \A)SQL.Server.*Driver"),
        "Microsoft Access":(r"Microsoft AccessDriver",r"JET.Database. Engine",r"Access .Database.Engine"),
        "Oracle": (r" \b0RA-[0-9][0-9][0-9][0-9]", r"Oracle . error", r"Oracle. *Driver", r"Warning. *\Woci_.*",r"Warning.*\Wora_.*"),
        "IBM. DB2": (r"CLI .Driver. *DB2", r"DB2. SQL error", r" \bdb2_ \w+\("),
        "SQLite": (r"SQLite/JDBCDriver", r"SQLite . Exception", r"System. Data . SQL ite . SQL iteException",r"Warning.*sqlite_*"),
        "Sybase": (r" (?i)Warning. *sybase. *",r"Sybase . message", r"Sybase. *Server . message.*"),
    }
    _url=url +"%29%28%22%27" # )("'
    Downloader1 = Downloader.Downloader()
    _content=Downloader1.get(_url)

    #用正则表达式来比对结果
    for (dbms,regex) in ((dbms,regex) for dbms in DBMS_ERRORS for regex in DBMS_ERRORS[dbms]):
        if (re.search(regex,_content)):
            return True
        content={}
        content["origin"]=Downloader1.get(_url)
        for test_payload in BOOLEAN_TESTS:
            #能够进入到这里的都是检测到注入的页面点
            randint=random.randint(1,255)
            _url=url+test_payload % (randint,randint)
            content["true"]=Downloader1.get(_url)
            _url = url + test_payload % (randint, randint+1)
            content["false"] = Downloader1.get(_url)
            if content["origin"]==content["true"]!=content["false"]:
                return "找到注入点，URL：" + url






