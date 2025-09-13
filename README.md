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
   
   HTTP请求报文的起始行结构如表所示

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
   
   HTTP响应报文的起始行结果如表所示

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

   #### 3.4.1 HTTP请求解析

   目标：从获取到的客户端的HTTP请求报文中提取请求方法（GET或POST）、目标URL和目标Host字段，为后面访问控制和请求转发提供依据。
   
   实现方法：由HTTP请求报文的起始行结构，检查前几个字符是否为“GET”或者“POST”；再定位第一个空格后的字符，一直到第二个空格前的内容即为URL；定位Host字段，只要搜索“Host： ”字符串，提取后面直到换行符的内容，就是Host字段的内容。
   
   关键代码：

   ```c
   void parse_http_request(const char *request, char *method, char *url, char *host) 
   {
      // 初始化 
      method[0] = '\0';
      url[0] = '\0';
      host[0] = '\0';

      // 解析请求方法（GET 或 POST）
      if (strncmp(request, "GET", 3) == 0) 
      strcpy(method, "GET");
      else if (strncmp(request, "POST", 4) == 0) 
      strcpy(method, "POST");
    
      // 解析URL
      const char *url_start = strchr(request, ' ') + 1;//跳过方法
      const char *url_end = strchr(url_start, ' ');
      if (url_start && url_end) 
      { 
         strncpy(url, url_start, url_end - url_start);
         url[url_end - url_start] = '\0'; 
      }    
      // 解析Host
      const char *host_start = strstr(request, "Host: ");
      if (host_start) 
      {
         host_start += 6;  // 跳过 "Host: "
         const char *host_end = strstr(host_start, "\r\n");
         if (host_end) 
         { 
         strncpy(host, host_start, host_end - host_start); 
         host[host_end - host_start] = '\0'; 
         }    
      }
   }
   ```

   #### 3.4.2 获取客户端IP

   目标：验证客户端IP是否与设定的客户端IP一致。
   
   实现方法：先使用getpeername()函数获取与client_socket关联的客户端地址结构体；然后调用inet_ntop()将获得的客户端地址结构体struct sockaddr_in中的IPv4地址从二进制形式转换为点分十进制字符串的形式。
   
   关键代码：

   ```c
struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(client_addr);
getpeername(client_socket, (struct sockaddr*)&client_addr, &addr_len);
inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
   ```

   #### 3.4.3 解析目标主机名

   目标：将解析得到的HTTP请求中的目标Host字段解析为IP地址，用于建立代理服务器与目标服务器的连接。
   
   实现方法：调用gethostbyname()函数，通过DNS查询获取目标主机的IP地址列表，选取返回的IP地址列表中的第一个有效地址。
   
   关键代码：

   ```c
struct hostent *host_entry = gethostbyname(target_host);
if (!host_entry) 
{
   printf("DNS resolution failed for %s\n", target_host);
   return;
}
memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);
   ```

   #### 3.4.4 访问控制策略

   目标：只有当客户端IP地址与设定的客户端IP地址一致，并且请求服务器与设定的服务器域名或IP地址不一致时，该应用代理防火墙才提供代理服务。
   
   实现代码：

   ```c
if (strcmp(client_ip, ALLOWED_CLIENT_IP) == 0 && strcmp(target_host, ALLOWED_SERVER) != 0) 
{  
   // 允许代理，建立连接并转发  
} 
else 
{  
   // 拒绝提供代理服务  
}  
   ```

   #### 3.4.5 双向数据转发

   目标：实现客户端与目标服务器间的请求与响应实时转发。
   
   关键代码：

   ```c
// 转发客户端请求给目标服务器  
send(server_socket, buffer, bytes_received, 0);  
   
// 将目标服务器响应转发给客户端  
while ((bytes = recv(server_socket, buffer, BUFFER_SIZE, 0)) > 0) 
{  
   send(client_socket, buffer, bytes, 0);  
}  
   ```

***

## 4. 系统实现

   ### 4.1 开发环境

   虚拟机系统：SEED-Ubuntu20.04
   HTTP客户端：火狐浏览器
   网络抓包工具：Wireshark

   ### 4.2 系统运行结果

   + 本项目中设定的客户端IP为127.0.0.1，设定的服务器域名为www.baidu.com（被排除，不提供代理服务）
   + HTTP代理服务器的地址和端口为127.0.0.1：8888
   + wireshark抓取环回接口lo流量，验证代理服务器与客户端/目标服务器的通信。

   #### 4.2.1 火狐浏览器设置代理 127.0.0.1:8888（代理服务器程序未运行）
   
   手动设置代理服务器，IP地址为127.0.0.1，端口设为8888，与程序中监听端口保持一致
   
   <img width="828" height="727" alt="代理服务器设置" src="https://github.com/user-attachments/assets/bf86faa5-8f6d-4ea2-b8ae-c832b1ae6903" />
   
   （1）访问设定的服务器域名 www.baidu.com
   
   <img width="895" height="674" alt="访问域名" src="https://github.com/user-attachments/assets/ea3ebb09-f6fe-4abe-a1d9-239b6ce5ef2a" />
   
   <img width="1287" height="398" alt="访问域名时的流量" src="https://github.com/user-attachments/assets/d0db0c28-d9af-4809-810d-3c36d583e959" />
   
   （2）访问其他服务器域名（如 www.csdn.net）
   
   <img width="882" height="628" alt="访问域名csdn" src="https://github.com/user-attachments/assets/279799fe-bf03-474e-8cbd-b8ca9886d46c" />
   
   <img width="1290" height="215" alt="访问域名csdn时的流量" src="https://github.com/user-attachments/assets/d3e840fd-20c7-4865-b61c-cc8dc55e059f" />
  
   由（1）、（2）中的结果可以发现，在未运行代理服务器程序前，客户端无法访问网址。分析访问域名时的流量，客户端访问网址时，客户端向代理服务器端口8888发送SYN包，尝试通过此端口建立TCP连接，但是端口8888返回了RST数据包，拒绝了连接请求，从而无法访问网站。
   所以，代理服务器未运行，客户端（浏览器）无法通过127.0.0.1：8888建立TCP连接，进而无法访问目标网站。

   #### 4.2.2 运行代理服务器程序

   运行代理服务器程序
   （1）访问设定的服务器域名 www.baidu.com
   
   <img width="781" height="238" alt="访问域名时程序的运行结果" src="https://github.com/user-attachments/assets/431fbd74-fb9a-41c8-94f5-a9ca0ecb0980" />
   
   <img width="904" height="320" alt="客户端访问域名" src="https://github.com/user-attachments/assets/ab3338ca-db4d-4652-9c5b-86cd8094c04a" />
   
   可以看到，虽然客户端的IP地址与设定的客户端IP地址一致，但是由于客户端请求访问的服务器域名与设定的服务器域名一致，所以代理服务器拒绝提供代理服务，客户端无法访问网站www.baidu.com。
   
   <img width="1278" height="277" alt="访问域名时的流量2" src="https://github.com/user-attachments/assets/110bed65-1c1f-47b4-94c2-1ec3f46c68cd" />
   
   分析流量情况，客户端与代理服务器之间的连接成功，TCP三次握手成功，说明代理服务器在运行并成功建立了连接，但目标Host和设定的服务器域名一致，所以代理拒绝转发请求，拒绝提供代理服务，代理服务器返回403 Forbidden后，客户端发送FIN-ACK终止连接。
   （2）访问其他服务器域名（如 www.csdn.net）
   
   <img width="624" height="136" alt="访问域名csdn时程序的显示结果" src="https://github.com/user-attachments/assets/97288d36-3447-4c63-a85a-7265588f92e3" />
   
   <img width="900" height="420" alt="客户端访问域名csdn" src="https://github.com/user-attachments/assets/ca477203-97e9-452a-844b-f6dcf2933b07" />
   
   可以看到，当客户端的IP地址与设定的客户端IP地址一致，且客户端请求访问的服务器域名与设定的服务器域名不一致时，代理服务器提供代理服务，客户端成功访问网站www.csdn.net。
   
   <img width="1271" height="332" alt="访问域名csdn时的流量2" src="https://github.com/user-attachments/assets/2821054f-5e63-4be1-94ca-1a822b6d02ba" />
   
   分析流量情况，客户端与代理服务器之间的连接成功，TCP三次握手成功，说明代理服务器在运行并成功建立了连接，且目标Host和设定的服务器域名不一致，所以代理服务器提供代理服务，之后数据包14显示代理服务器返回了301重定向，这可能是因为目标服务器有重定向。表现了代理服务器正确转发了请求和响应，客户端最终成功访问了网站。

   #### 4.2.3 验证功能“如果客户端的IP不是设定的IP 127.0.0.1的话，就不提供代理”

   修改代码，将代理服务器中设定的允许的客户端IP从127.0.0.1修改为127.0.0.2，然后重新运行代理服务器程序，客户端127.0.0.1访问网站www.csdn.net，结果如下：
   
   <img width="628" height="129" alt="修改后访问域名csdn时程序的显示结果" src="https://github.com/user-attachments/assets/b1451922-52b8-4449-8f49-37d2e733b736" />
   
   <img width="699" height="262" alt="修改后客户端访问域名csdn" src="https://github.com/user-attachments/assets/d74c2e72-1b14-4271-b3c7-c1fd15c92670" />
   
   可以看到，客户端的IP地址与设定的客户端IP地址不一致，即使客户端请求访问的服务器域名与设定的服务器域名不一致，代理服务器也拒绝提供代理服务，客户端无法访问网站www.csdn.net。
   
   <img width="1282" height="210" alt="修改后访问域名csdn时的流量" src="https://github.com/user-attachments/assets/64e375f1-0ef3-4e8c-8b42-d7ec8bc8b40a" />
   
   分析流量情况，客户端与代理服务器之间的连接成功，TCP三次握手成功，说明代理服务器在运行并成功建立了连接，当客户端发起HTTP请求时，由于客户端IP与设定的客户端IP不一致，所以拒绝提供代理服务，代理服务器返回403 Forbidden后，客户端发送FIN-ACK终止连接。

***

## 5. 项目开发总结

   ### 5.1 项目成果

   成功开发了一个基于sockret的HTTP代理服务器，系统运行结果充分体现了本项目实现的HTTP代理服务器成功完成了功能：当客户端IP地址与设定的客户端IP地址一致，并且请求服务器与设定的服务器域名或IP地址不一致时，该应用代理防火墙才提供代理服务（提供代理服务是指客户端能够通过此应用代理服务器正常连接服务器，并且此应用代理服务器能够返回服务器的响应内容给客户端），否则不提供代理服务。

   ### 5.2 项目不足之处

   + 并发处理能力不足
     项目使用的是单线程，没有使用多线程处理客户端的请求，导致访问网站速度慢，无法支撑高并发场景，可以在这方面进行优化。
   + 考虑的情况不全
     没有处理网络中断、目标服务器宕机等异常情况，对畸形HTTP请求（比如包含非法字符等）没有过滤。这样代理服务器困难因为异常输入而崩溃，稳定性差。
   + 配置灵活性差
     运行代理的客户端IP和排除的目标服务器域名硬编码在代码中。这导致每次策略变更需重新编译程序，运维成本高，且难以适应复杂网络环境。
