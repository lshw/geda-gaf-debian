# geda-gaf 是一个简单易用的eda设计工具

# 刚刚发现其实geda-gaf的后续分叉是https://github.com/lepton-eda/lepton-eda，  
# 是时候转换到lepton-eda了， 文件格式是兼容的，debian11和debian12都带了 lepton-eda. 

在debian11中， 不再包含geda-gaf， 在github上找到了这个项目:<br> 
https://github.com/rlutz/geda-gaf-debian<br>
把它和上游的git库合并在一起， 做成了目前这个项目.<br>上游的git库:<br> 
git://git.geda-project.org/geda-gaf.git<br> 


## 在debian11下安装 geda-gaf的方法
### 编译好的deb包， 我会放在几个镜像站:<br>
https://mirrors.tuna.tsinghua.edu.cn/bjlx/pool/main/g/geda-gaf/<br>
https://mirrors.ustc.edu.cn/bjlx/pool/main/g/geda-gaf/<br>
https://mirrors.cloud.tencent.com/bjlx/pool/main/g/geda-gaf/<br>
https://mirrors.nju.edu.cn/bjlx/pool/main/g/geda-gaf/<br>
https://mirrors.bfsu.edu.cn/bjlx/pool/main/g/geda-gaf/<br>
https://mirrors.aliyun.com/bjlx/pool/main/g/geda-gaf/<br>

### 包括amd64和mips64el架构, 安装方法如下: 
wget -O - https://mirrors.tuna.tsinghua.edu.cn/bjlx/bjlx.key |apt-key add -<br>
echo 'deb http://mirrors.ustc.edu.cn/bjlx bullseye main' > /etc/apt/sources.list.d/bjlx.list<br>
apt update<br>
apt install geda-gnetlist geda-gschem geda-symbols<br>

### 在debian11下编译的方法:
给/etc/apt/sources.list.d/bjlx.list增加一行内容<br>
deb-src http://mirrors.cloud.tencent.com/bjlx bullseye main<br>

### 然后就可以获得源码包了:
apt update<br>
apt-get source geda-gaf<br>

### 安装编译环境:
apt install fakeroot dpkg-dev gcc<br>
 
### 安装依赖包:
apt-get build-dep geda-gaf<br>

### 开始编译: 
cd geda-gaf-1.10.2~20210706<br>
dpkg-buildpackage -rfakeroot<br>
