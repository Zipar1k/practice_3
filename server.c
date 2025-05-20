#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define PORT 8080

void urldecode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && (a = src[1]) && (b = src[2]) && isxdigit(a) && isxdigit(b)) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

int main() {
    // Создаем сокет
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Настраиваем адрес сервера
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    
    // Привязываем сокет к адресу
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    
    // Слушаем соединения
    listen(server_fd, 5);
    printf("Сервер запущен на порту %d\n", PORT);

    while(1) {
        // Принимаем соединение
        int client_fd = accept(server_fd, NULL, NULL);
        
        // Читаем запрос
        char buffer[2048];
        read(client_fd, buffer, 2048);
        
        // Извлекаем сообщение
        char* msg_start = strstr(buffer, "message=");
        char encoded_msg[256] = "";
        char decoded_msg[256] = "";

        if (msg_start) {
            sscanf(msg_start + 8, "%255[^& \n]", encoded_msg);
            urldecode(decoded_msg, encoded_msg); // Декодируем
        }

        
        // Отправляем HTML ответ
        char response[2048];
        sprintf(response, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body>"
            "<h1>%s</h1>"
            "<img src='https://www.mirea.ru/upload/medialibrary/c1a/MIREA_Gerb_Colour.jpg' width='300'>"
            "</body></html>", 
            decoded_msg);
            
        write(client_fd, response, strlen(response));
        close(client_fd);
    }
    
    return 0;
}
