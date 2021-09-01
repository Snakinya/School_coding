#爬虫核心业务实现程序
#支持多线程
#实现目标url内容获取和A标签上的连接解析，并再次根据A标签的url进行二层获取
from lib.core import Downloader,UrlManager
import threading
from urllib import parse
from urllib.parse import urljoin
from bs4 import BeautifulSoup
#导入sql检测的脚本
from script import sqlcheck

class SpiderMain():
    def __init__(self,checkurlroot,threadNum):
        self.urls=UrlManager.UrlManager()
        self.download=Downloader.Downloader()
        self.checkurlroot=checkurlroot
        self.threadNum=threadNum
    def _judge(self,domain,url):
        if(url.find(domain)!=-1):
            return True
        return False

    def _parse(self,page_url,content):
        if content is None:
            return
        soup=BeautifulSoup(content,'html.parser')
        #需要获得页面内的url地址
        _news=self._get_new_urls(page_url,soup)
        return _news
    #获取页面内符合同一个域名下的url地址
    def _get_new_urls(self,page_url,soup):
        new_urls=set()
        links=soup.find_all("a") #在页面中找所有的A标签
        for link in links:
            #遍历解析出href
            new_url=link.get("href")
            new_full_url=urljoin(page_url,new_url)
            #判断这些url是不是自己域名下的，过滤同一域名下的地址
            if (self._judge(self.checkurlroot,new_full_url)):
                new_urls.add(new_full_url)
        return new_urls
    #业务入口
    def main(self):
        self.urls.add_new_url(self.checkurlroot)
        while self.urls.has_new_url():
            _content=[]
            th=[]
            for i in list(range(self.threadNum)):
                if self.urls.has_new_url() is False:
                    break
                new_url=self.urls.get_new_url()
                print("取到的地址"+new_url)


                #检测sql注入
                try:
                    if(sqlcheck.sqlcheck(new_url)):
                        print("url:%s is useful:" % new_url)
                except Exception as e:
                    raise e

                t=threading.Thread(target=self.download.download,args=(new_url,_content))
                t.start() #启动一个线程
                th.append(t)
            for t in th:
                t.join()
            for _str in _content:
                if _str is None:
                    continue
                new_urls=self._parse(new_url,_str["html"])
                self.urls.add_new_urls(new_urls)


