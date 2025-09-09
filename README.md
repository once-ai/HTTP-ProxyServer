# HTTP-ProxyServer
基于socket的HTTP代理服务器，系统采用C语言进行开发，运行于Linux环境，实现了客户端IP校验、特定目标服务器排除、HTTP请求解析与转发等功能。

***
## 1. 开发目的
   ### 1.1 熟悉并掌握socket网络编程
   HTTP代理服务器的客户端监听、目标服务器连接以及双向数据转发等功能都是基于socket网络编程实现的，通过该开发课题，理解socket编程中的套接字创建、地址绑定、连接监听、请求接受和数据读写等主要的编程步骤。
   ### 1.2 理解HTTP协议和代理服务器的工作原理
   理解代理服务器的工作原理，剖析代理服务器的中间人角色，同时要掌握HTTP协议报文的格式，了解报文接收与转发的流程。掌握 HTTP协议的请求方法、消息头、消息体等结构组成，以及请求和响应的交互流程，进而理解 HTTP代理服务器在客户端和目标服务器之间转发数据、处理请求的基本工作原理。
   ### 1.3 掌握代理服务器设计与实现的核心功能
   通过访问控制策略（客户端IP与目标服务器域名过滤）、协议兼容性处理（支持GET/POST方法）及异常响应（返回403错误）等功能的编码实践，提升对网络编程中安全策略设计、协议解析优化及系统健壮性保障的能力，为后续复杂网络应用开发奠定基础。

***
## 2. 系统功能
   ### 2.1 客户端IP过滤
   只有当客户端IP地址与设定的客户端IP地址一致时，才允许客户端连接代理服务器。
   本系统设置为：仅允许IP为127.0.0.1的客户端连接代理服务器。
   ### 2.2 目标服务器排除
   当请求服务器与设定的服务器域名或IP地址不一致时，该应用代理防火墙才提供代理服务。
   本系统设置为：若请求的目标主机为www.baidu.com，则拒绝代理服务。
   ### 2.3 HTTP请求解析
   解析客户端发送的HTTP请求，提取请求方法（GET/POST）、URL及Host等关键信息，支持域名解析与端口绑定。
   ### 2.4 数据转发
   代理服务器将合法请求转发到目标服务器，并实时将服务器响应传回客户端，支持POST请求体传输。

***
## 3. 系统实现
   ### 3.1 HTTP报文结构
   HTTP有两类报文，请求报文和响应报文。客户端向服务器发送请求报文，从服务器到客户端的回答为响应报文。
   
   <img width="1270" height="440" alt="HTTP请求报文" src="https://github.com/user-attachments/assets/d43fbcc8-6f72-47d8-8371-43ad0b590410" />
   可以得到HTTP请求报文的起始行结构如表所示
   
   <table>
    <tr>
     <th style="background-color: yellow; text-align: center">METHOD</th>
     <th style="text-align: center">空格</th>
     <th style="background-color: yellow; text-align: center">URL</th>
     <th style="text-align: center">空格</th>
     <th style="background-color: yellow; text-align: center">VERSION</th>
     <th style="text-align: center">换行</th>
    </tr>
   </table>
   
   METHOD表示请求方法，如GET/POST/HEAD/PUT等，是对资源的操作，其中GET和POST是最常用的两个方法，本系统中暂时只考虑了这两种方法。
   URL表示请求方法要操作的资源，也是俗称的“网址”。
   VERSION表示报文使用的HTTP协议版本号。
   
   <img width="1266" height="393" alt="HTTP响应报文" src="https://github.com/user-attachments/assets/958cb09c-7dde-4094-91f3-f5063df61ab4" />
   可以得到HTTP响应报文的起始行结果如表所示
   
   <table>
    <tr>
     <th style="background-color: yellow; text-align: center">VERSION</th>
     <th style="text-align: center">空格</th>
     <th style="background-color: yellow; text-align: center">STATUS CODE</th>
     <th style="text-align: center">空格</th>
     <th style="background-color: yellow; text-align: center">REASON</th>
     <th style="text-align: center">换行</th>
    </tr>
   </table>
   
   VERSION表示报文使用的HTTP协议版本号。
   STATUS CODE是状态码，表示处理的结果，比如200是成功，500是服务器错误。
   REASON作为状态码的补充，是更详细的解释文字。
   ### 3.2 代理服务器的原理
   代理服务器相当于客户端与服务器之间的中间人，代理服务器监听指定端口，与客户端建立TCP连接，当客户端发起HTTP请求时，请求被代理服务器接收，而不是直接发送到目标服务器。当客户端的请求通过后，代理服务器创建TCP连接与目标服务器通信，将请求数据转发过去，代理服务器又将目标服务器的响应内容返回给客户端。
   ### 3.3 HTTP代理服务器的程序流程图
   本项目的HTTP代理服务器程序的主要过程为：
  （1）代理服务器监听客户端的连接请求；
  （2）客户端连接时，accept()返回新的套接字描述符client_socket，用于代理服务器与该客户端之间的通信，并获取与client_socket关联的客户端地址信息；
  （3）通过recv()读取客户端发送的HTTP请求报文，解析HTTP请求，获取HTTP请求方法、URL、Host字段；
  （4）双重条件判断，检查客户端IP是否与设定的客户端IP一致，然后验证目标服务器的域名是否和设定的不一致，如果没有通过双重检查，则返回HTTP/1.1 403 Forbidden响应并关闭代理与客户端之间的连接client_socket；如果通过了，继续执行后续的代理转发流程；
  （5）代理服务器与目标服务器建立连接；
  （6）代理服务器将客户端请求数据发送给目标服务器，接收到目标服务器的响应后，将响应数据发送回客户端；
  （7）关闭套接字，连接终止并释放资源。
  ```mermaid
  flowchart
  start([开始]) --> step1[创建并初始化套接字<br>使用 bind 绑定端口地址</br>]
  step1 --> step2[设置 listen ，进行监听]
  step2 --> step3[ accept 接收连接请求<br>生成 client_socket 与客户端进行通信</br>]
  step3 --> step4[获取客户端IP地址 getpeername]
  step4 --> step5[获取HTTP请求数据<br> recv 到缓冲区</br>]
  step5 --> step6[解析HTTP请求方法、URL、Host<br>parse_http_request</br>]
  step6 --> judge{客户端IP是否合法<br><b>and</b></br>目标服务器是否不被排除}
  judge -->|是| step7[创建目标服务器连接<br> socket + connect <br> server_socket ]
  judge -->|否| step11 
  step7 --> step8[转发请求到目标服务器<br> send 至 server_socket]
  step8 --> step9[接收目标服务器响应<br> recv 至缓冲区]
  step9 --> step10[回传响应给客户端<br> send 至 client_socket ]
  step10 --> step11[关闭连接]
  step11 -->|循环| step3  
  ```
   ### 3.4 系统的关键技术

***
## 4. 系统实现
   ### 4.1 开发环境
   ### 4.2 系统运行结果

***
## 5. 项目开发总结
   ### 5.1 项目成果
   ### 5.2 困难及解决方法
   ### 5.3 项目不足之处
