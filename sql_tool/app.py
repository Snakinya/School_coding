#程序入口
import sys
from lib.core.Spider import SpiderMain
def main():
    checkurlroot= "http://www.baidu.com"  #待检测地址
    threadNum=10#线程数
    sp=SpiderMain(checkurlroot,threadNum)
    sp.main()
if __name__=='__main__':
    main()