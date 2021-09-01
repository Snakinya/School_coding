#url 管理器
class UrlManager(object):
    def __init__(self):
        self.new_urls= set()
        self.old_urls= set()
    #用于过滤重复的url地址
    def add_new_url(self,url):
        if url is None:
            return
        if url not in self.new_urls and url not in self.old_urls:
            self.new_urls.add(url)
    #url的解析方法，用于将多个url最后过滤重复后，放到new_urls对象中统一管理
    def add_new_urls(self,urls):
        if urls is None or len(urls)==0:
            return
        for url in urls:
            self.add_new_url(url)
    #判断是否有url的方法
    def has_new_url(self):
        return len(self.new_urls)!=0
    #获得url的方法
    def get_new_url(self):
        #print(self.new_urls)
        new_url=self.new_urls.pop()
        self.old_urls.add(new_url)
        return new_url
