#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PROXY_PORT 8888                // 代理服务器监听端口
#define BUFFER_SIZE 65507                // 缓冲区大小
#define ALLOWED_CLIENT_IP "127.0.0.1"  // 允许的客户端IP
#define ALLOWED_SERVER "hcl.baidu.com"  // 排除的目标服务器（不为此服务器代理）

// 解析 HTTP 请求中的方法、URL 和 Host
void parse_http_request(const char *request, char *method, char *url, char *host) {
    // 初始化
    method[0] = '\0';
    url[0] = '\0';
    host[0] = '\0';

    // 解析请求方法（GET 或 POST）
    if (strncmp(request, "GET", 3) == 0) {
        strcpy(method, "GET");
    } else if (strncmp(request, "POST", 4) == 0) {
        strcpy(method, "POST");
    }

    // 解析 URL
    const char *url_start = strchr(request, ' ') + 1; // 跳过方法
    const char *url_end = strchr(url_start, ' ');     // 找到 URL 结束位置
    if (url_start && url_end) {
        strncpy(url, url_start, url_end - url_start);
        url[url_end - url_start] = '\0';
    }

    // 解析 Host
    const char *host_start = strstr(request, "Host: ");
    if (host_start) {
        host_start += 6; // 跳过 "Host: "
        const char *host_end = strstr(host_start, "\r\n");
        if (host_end) {
            strncpy(host, host_start, host_end - host_start);
            host[host_end - host_start] = '\0';
        }
    }
}

// 处理客户端的请求
void handle_client(int client_socket) {
    // 缓冲区，存储客户端请求数据和服务器响应数据
    char buffer[BUFFER_SIZE];
    // 客户端的 IP 地址
    char client_ip[INET_ADDRSTRLEN];
    // 客户端的地址信息
    struct sockaddr_in client_addr;
    // 客户端地址结构体的长度
    socklen_t addr_len = sizeof(client_addr);

    // 1. 获取客户端IP地址
    // 获取与客户端套接字关联的客户端地址信息
    getpeername(client_socket, (struct sockaddr*)&client_addr, &addr_len);
    // 将客户端的 IP 地址从二进制形式转换为点分十进制字符串形式
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    // 2. 读取客户端请求数据
    // recv 函数从客户端套接字接收数据，并返回接收到的字节数
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("recv failed");
        close(client_socket);
        return;
    }

    // 3. 解析请求中的方法、URL 和 Host
    char method[16] = {0};
    char target_url[1024] = {0};
    char target_host[1024] = {0};
    parse_http_request(buffer, method, target_url, target_host);

    // 打印访问的 URL 和方法
    printf("请求方法: %s, 访问的 URL: %s\n", method, target_url);

    // 4. 先判断客户端 IP 是否为允许的 IP
    if (strcmp(client_ip, ALLOWED_CLIENT_IP) != 0) {
        printf("禁止代理的客户端IP：%s\n", client_ip);
        const char *deny_msg = "HTTP/1.1 403 Forbidden\r\n"
                               "Content-Type: text/plain\r\n\r\n"
                               "Proxy service denied.\n";
        send(client_socket, deny_msg, strlen(deny_msg), 0);
        close(client_socket);
        return;
    }

    // 5. 再判断目的服务器是否为排除的服务器
    if (strcmp(target_host, ALLOWED_SERVER) == 0) {
        printf("禁止代理访问的 URL: %s\n", target_url);
        const char *deny_msg = "HTTP/1.1 403 Forbidden\r\n"
                               "Content-Type: text/plain\r\n\r\n"
                               "Proxy service denied.\n";
        send(client_socket, deny_msg, strlen(deny_msg), 0);
        close(client_socket);
        return;
    }

    // 6. 通过验证，创建与目标服务器的连接
    // 创建一个 TCP 套接字，用于与目标服务器通信
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket failed");
        close(client_socket);
        return;
    }

    // 解析目标服务器地址（支持域名）
    // 将目标主机名解析为对应的 IP 地址信息
    struct hostent *host_entry = gethostbyname(target_host);
    // 如果 DNS 解析失败
    if (!host_entry) {
        printf("DNS resolution failed for %s\n", target_host);
        close(server_socket);
        close(client_socket);
        return;
    }

    // 存储目标服务器的地址信息
    struct sockaddr_in server_addr;
    // 设置地址族为 IPv4
    server_addr.sin_family = AF_INET;
    // 设置目标服务器的端口号
    server_addr.sin_port = htons(80);  // 默认HTTP端口
    // 将解析得到的 IP 地址复制到服务器地址结构体中
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    // 连接到目标服务器
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(server_socket);
        close(client_socket);
        return;
    }

    // 7. 转发请求并中继响应
    // 将客户端的请求数据转发给目标服务器
    send(server_socket, buffer, bytes_received, 0);

    // 如果是 POST 请求，继续读取请求体并转发
    if (strcmp(method, "POST") == 0) {
        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            send(server_socket, buffer, bytes_received, 0);
        }
    }

    // 中继目标服务器的响应
    // 循环读取目标服务器的响应数据并转发给客户端
    ssize_t bytes;
    while ((bytes = recv(server_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        if (bytes < 0) break;
        send(client_socket, buffer, bytes, 0);
    }

    close(server_socket);
    close(client_socket);
}

int main() 
{
    // 创建套接字，监听客户端的连接请求
    int proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 绑定地址和端口
    // 定义结构体，存储代理服务器的地址信息
    struct sockaddr_in proxy_addr;
    // 设置地址族为 IPv4
    proxy_addr.sin_family = AF_INET;
    // 设置代理服务器监听所有可用的网络接口
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    // 代理服务器监听的端口号
    proxy_addr.sin_port = htons(PROXY_PORT);

    // 将代理服务器的套接字与地址和端口绑定
    if (bind(proxy_socket, (struct sockaddr*)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("bind failed");
        close(proxy_socket);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(proxy_socket, 5) < 0) {
        perror("listen failed");
        close(proxy_socket);
        exit(EXIT_FAILURE);
    }

    printf("[+] Proxy server listening on port %d\n", PROXY_PORT);

    // 单线程处理循环
    while (1) 
    {
        // 定义结构体，存储客户端的地址信息
        struct sockaddr_in client_addr;
        // 客户端地址结构体的长度
        socklen_t addr_len = sizeof(client_addr);
        // 接受客户端的连接请求，返回一个新的套接字描述符与客户端通信
        int client_socket = accept(proxy_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) 
        {
            perror("accept failed");
            continue;
        }

        // 输出客户端连接的信息
        printf("Connection from: %s\n", inet_ntoa(client_addr.sin_addr));
        // 调用 handle_client 函数处理客户端的请求
        handle_client(client_socket); 
    }

    close(proxy_socket);
    return 0;
}
