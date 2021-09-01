#获得url的下载器
#本下载器需要支持get和post
import requests
class Downloader(object):
    """docstring for Downloader"""

    def get(self,url):
        r=requests.get(url,timeout=30)
        if r.status_code !=200:
            #表示不成功
            return None
        _str=r.text
        return _str

    #post 请求方法，主要通过表单提交过来的请求
    def post(self,url,data):
        r=requests.post(url,data)
        _str=r.text
        return _str
    #通过url获得对应的地址的内容
    def download(self,url,htmls):
        if url is None:
            return "None"
        _str={}
        _str["url"]=url

        try:
            r = requests.get(url, timeout=10)
            if r.status_code != 200:
                # 表示不成功
                return None
            #将得到的页面文本内容放到_str
            _str["html"] = r.text
        except Exception as e:
            return None
        htmls.append(_str)

