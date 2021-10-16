#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <ctype.h>

/* Functia verifica daca un string este valid
    un string se considera valid daca acesta coincide
    cu stringurile din enunt
*/
int check_for_valid(int flag, char *string) {
    if (flag == LOGIN_MSG || flag == REGISTER_MSG) {
        if (!strncmp(string, "username", 8))
            return 1;
        if (!strncmp(string, "password", 8))
            return 1;
    }
    if (flag == GET_BOOK_MSG || flag == DELETE_BOOK) {
        if(!strncmp(string, "id", 2))
            return 1;
    }
    if (flag == ADD_BOOK_MSG) {
        if(!strncmp(string, "title", 5))
            return 1;
        if(!strncmp(string, "author", 6))
            return 1;
        if(!strncmp(string, "genre", 5))
            return 1;
        if(!strncmp(string, "publisher", 9))
            return 1;
        if(!strncmp(string, "page_count", 10))
            return 1;
    }
    return 0;
}
/* Functia extrage cookie dintr-un HTTP-Reply
*/
char *get_cookies(char *http_reply) {
    const char s[4] = "\r\n"; 
    char* tok = calloc(BUFLEN, sizeof(char)); 
    char* copy_http = calloc(BUFLEN, sizeof(char));
    strcpy(copy_http, http_reply);
    int j = 1;
    tok = strtok(copy_http, s); 
    while (tok != 0 && j != COOKIES_LEN) { 
        tok = strtok(0, s);
        j++; 
    }
    char *final_cookie = calloc(BUFLEN, sizeof(char));
    sprintf(final_cookie, "%s", tok + 12);
    return final_cookie;
}

/* Functia extrage JWT-ul din HTTP Reply */
char *get_jwt(char *http_reply) {
    const char s[4] = "\r\n"; 
    char* tok = calloc(BUFLEN, sizeof(char));  
    char* copy_http = calloc(BUFLEN, sizeof(char));
    strcpy(copy_http, http_reply);
    int j = 1;
    tok = strtok(copy_http, s); 
    while (tok != 0 && j != JWT_LEN) { 
        tok = strtok(0, s);
        j++; 
    }
    char *my_jwt = calloc(BUFLEN, sizeof(char));
    char *final_jwt = calloc(BUFLEN, sizeof(char));
    sprintf(my_jwt, "Bearer %s", tok + 10);
    strncpy(final_jwt, my_jwt, strlen(my_jwt)-2);
    return final_jwt;
}

/* Creaza un payload pentru o carte care
    sa coincida cu payload-yl dorit */

char *add_book(char *title, char *author, char *genre,
    char *publisher, int page_count) {
    char g = '"';
    char *payload = calloc(BUFLEN, sizeof(char));
    sprintf(payload, "{\n\t%ctitle%c: %c%s%c,\n\t%cauthor%c: %c%s%c,\n\t%cgenre%c: %c%s%c,\n\t%cpage_count%c: %c%d%c,\n\t%cpublisher%c: %c%s%c\n}", 
    g,g,g,title,g ,g,g,g,author,g, g,g,g,genre,g, g,g,g,page_count,g, g,g,g,publisher,g);
    return payload;
}

/* Creaza un payload pentru un user cu login si parola acestuia 
*/
char *add_user(char *user, char *pass) {
    char *payload = calloc(BUFLEN, sizeof(char));
    char g = '"';
    sprintf(payload, "{\n\t%cusername%c: %c%s%c,\n\t%cpassword%c: %c%s%c\n}", g,g,g,user,g,g,g,g,pass,g);
    return payload;
}

/* Extrage din string-ul introdus de la 
    tastatura valoare ce se afla dupa "="
*/
void get_input(char **input) {
    char *new_input = calloc(INPUT_LEN, sizeof(char));
    new_input = strstr(*input, "=");
    if (new_input == NULL) {
        printf("Please enter valid version of command\n");
        (*input) = NULL;
        return;
    } else {
        strcpy(*input, new_input+1);
    }
}

/* Afisarea unor informatii cum ar 
trebui sa arate comenzile by-default
*/
void register_ex() {
    printf("Register needs to be:\n");
    printf("\tusername=something\n\tpassword=something\n");
}

void login_ex() {
    printf("Login needs to be:\n");
    printf("\tusername=something\n\tpassword=something\n");
}
void get_book_ex() {
    printf("Get book needs to be:\n");
    printf("\tid=any number\n");
}

void add_book_ex() {
    printf("Add book needs to be:\n");
    printf("\ttitle=something\n\tauthor=something\n");
    printf("\tgenre=something\n\tpublisher=something\n");
    printf("\tpage_count=any number\n");

}
void delete_book_ex() {
    printf("Delete book needs to be:\n");
    printf("\tid=any number\n");
}

/* Parseaza mesajele de erori care sunt intoarse de HTTP reply */
char *parse_error_msg(char *msg) {
    char *new_msg = calloc(BUFLEN, sizeof(char));
    char *final_msg = calloc(BUFLEN, sizeof(char));
    new_msg = strstr(msg, ":");
    strncpy(final_msg, new_msg+1, strlen(new_msg)-2);
    return final_msg;
}

/* Niste outputuri pentru comenzile introduse */
void client_output(int flag, char *http_reply) {
    char *ok_reply = calloc(BUFLEN, sizeof(char));
    char *bad_reply = calloc(BUFLEN, sizeof(char));
    char* copy_http = calloc(BUFLEN, sizeof(char));
    char *message_from_ok_reply = calloc(BUFLEN, sizeof(char));
    strcpy(copy_http, http_reply);
    strcpy(ok_reply, "HTTP/1.1 200 OK");
    
    /* Comenzile care s-au efectuat cu succes */
    if (!strncmp(copy_http, ok_reply, strlen(ok_reply))) {
        message_from_ok_reply = strstr(copy_http, "{");
        if (flag == LOGIN_MSG)
            printf("You have loggened in succesfully! This is the cookies:\n%s\n", get_cookies(copy_http));
        if (flag == REGISTER_MSG)
            printf("You have registered succesfully\n");
        if (flag == GET_BOOKS_MSG)
            printf("This is the list of books:\n[%s]\n", message_from_ok_reply);
        if (flag == ADD_BOOK_MSG)
            printf("You have added a new book to the library.\n");
        if (flag == GET_BOOK_MSG) {
            printf("This is the book you want to get:\n%s\n", message_from_ok_reply);
        }
        if (flag == ENTER_LIBRARY) {
            printf("You have entered the library succesfully! This is the JWT:\n");
            message_from_ok_reply = parse_error_msg(message_from_ok_reply);
            printf("%s\n", message_from_ok_reply);
        }
        if (flag == DELETE_BOOK)
            printf("You have deleted a book succesfully!\n");
        if (flag == LOGOUT_MSG)
            printf("You have logged out succesfully!\n");
    }
    /* Comenzile care au esuat */
    else {
        bad_reply = strstr(copy_http, "{");
        bad_reply = parse_error_msg(bad_reply);
        if (flag == REGISTER_MSG)
            printf("Register failed with message: %s\n", bad_reply);
        if (flag == LOGIN_MSG)
        printf("Login failed with message: %s\n", bad_reply);
        if (flag == ADD_BOOK_MSG)
            printf("Failed to add a book with message: %s\n", bad_reply);
        if (flag == ENTER_LIBRARY)
            printf("Failed to enter the library with message: %s\n", bad_reply);
        if (flag == GET_BOOK_MSG)
            printf("Failed to get a book with message: %s\n", bad_reply);
        if (flag == GET_BOOKS_MSG)
            printf("Failed to get books with message: %s\n", bad_reply);
        if (flag == DELETE_BOOK)
            printf("Failed to delete a book with message: %s\n", bad_reply);
        if (flag == LOGOUT_MSG)
            printf("Failed to log out with message: %s\n", bad_reply);
    }
}
int main(int argc, char *argv[]) {
    char *message, *response;
    int sockfd;
    char *my_book = calloc(BUFLEN, sizeof(char));
    char *buffer = calloc(BUFLEN, sizeof(char));
    char *user_cookies = calloc(BUFLEN, sizeof(char));
    char *user_jwt = calloc(BUFLEN, sizeof(char));

    printf("To access the library I recommand you to register or login!\n");
    while(scanf("%s", buffer) != EOF) {
        sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
        if (!strcmp(buffer, "register")) {
            /* /api/v1/tema/auth/register */
            char *user = calloc(MAX_LEN, sizeof(char));
            char *pass = calloc(MAX_LEN, sizeof(char));
            register_ex();
            char *new_user = calloc(BUFLEN, sizeof(char));
            scanf("%s", user);
            if (check_for_valid(REGISTER_MSG, user) == 0) {
                printf("Incorrect line, please try again! Re-write...\n");
                continue;
            }
            get_input(&user);
            scanf("%s", pass);
            if (check_for_valid(REGISTER_MSG, pass) == 0) {
                printf("Incorrect line, please try again! Re-write\n");
                continue;
            }           
            new_user = add_user(user, pass);
            message = compute_post_request(HOST_IP, URL_REGISTER, CONTENT_TYPE, NULL, &new_user, 1, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            client_output(REGISTER_MSG, response);
        }
        if (!strcmp(buffer, "login")) {
            /* /api/v1/tema/auth/login */
            char *user = calloc(MAX_LEN, sizeof(char));
            char *pass = calloc(MAX_LEN, sizeof(char));
            login_ex();
            char *new_user = calloc(BUFLEN, sizeof(char));
            scanf("%s", user);
            if (check_for_valid(LOGIN_MSG, user) == 0) {
                printf("Incorrect line, please try again! Re-write...\n");
                continue;
            }
            get_input(&user);
            scanf("%s", pass);
            if (check_for_valid(LOGIN_MSG, pass) == 0) {
                printf("Incorrect line, please try again! Re-write\n");
                continue;
            }
            get_input(&pass);
            new_user = add_user(user, pass);
            message = compute_post_request(HOST_IP, URL_LOGIN, CONTENT_TYPE, NULL, &new_user, 1, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            user_cookies = get_cookies(response);
            client_output(LOGIN_MSG, response);
        }
        if (!strcmp(buffer, "enter_library")) {
            /* /api/v1/tema/library/access */
            if (user_cookies == NULL) {
                message = compute_get_request(HOST_IP, URL_ACCESS, NULL, NULL, NULL, 0);
            }
            else {
                message = compute_get_request(HOST_IP, URL_ACCESS, NULL, NULL, &user_cookies, 1);
            }
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            user_jwt = get_jwt(response);
            client_output(ENTER_LIBRARY, response);
        }
        if (!strcmp(buffer, "get_books")) {
            /* /api/v1/tema/library/books */
            if (user_jwt == NULL) {
                message = compute_get_request(HOST_IP, URL_BOOKS, NULL, NULL, NULL, 0);    
            } else {
                message = compute_get_request(HOST_IP, URL_BOOKS, NULL, user_jwt, NULL, 0);
            }
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            client_output(GET_BOOKS_MSG, response);
        }
        if (!strcmp(buffer, "get_book")) {
            /* /api/v1/tema/library/books/:bookId */
            char *book_id = calloc(10, sizeof(char));
            char *book_url = calloc(BUFLEN, sizeof(char));
            get_book_ex();
            scanf("%s", book_id);
            if (check_for_valid(GET_BOOK_MSG, book_id) == 0) {
                printf("Incorrect line, please try again! Re-write\n");
                continue;
            }            
            get_input(&book_id);
            sprintf(book_url, "%s%s", URL_BOOK_NUM, book_id);
            if (user_jwt == NULL) {
                message = compute_get_request(HOST_IP, book_url, NULL, NULL, NULL, 0);
            } else {
                message = compute_get_request(HOST_IP, book_url, NULL, user_jwt, NULL, 0);
            }
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            client_output(GET_BOOK_MSG, response);
        }
        if (!strcmp(buffer, "add_book")) {
            /* add a book with title, author, etc.. */
            char *title = calloc(BUFLEN, sizeof(char));
            char *author = calloc(BUFLEN, sizeof(char));
            char *genre = calloc(BUFLEN, sizeof(char));
            char *publisher = calloc(BUFLEN, sizeof(char));
            char *page_string = calloc(BUFLEN, sizeof(char));
            int page_count = 0;
            add_book_ex();
            scanf("%s%s%s%s%s", title, author,genre,publisher,page_string);
            if ((check_for_valid(ADD_BOOK_MSG, title) && check_for_valid(ADD_BOOK_MSG, author) &&
            check_for_valid(ADD_BOOK_MSG, genre) && check_for_valid(ADD_BOOK_MSG, publisher) &&
            check_for_valid(ADD_BOOK_MSG, page_string)) == 0) {
                printf("Incorrect line, please try again! Re-write\n");
                continue;
            }
            get_input(&title);
            get_input(&author);
            get_input(&genre);
            get_input(&publisher);
            get_input(&page_string);            
            page_count = atoi(page_string);         
            my_book = add_book(title, author, genre, publisher, page_count);
            if (user_jwt == NULL) {
                message = compute_post_request(HOST_IP, URL_BOOKS, CONTENT_TYPE, NULL, &my_book, 1, NULL, 0);
            } else {
                message = compute_post_request(HOST_IP, URL_BOOKS, CONTENT_TYPE, user_jwt, &my_book, 1, NULL, 0);
            }
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            client_output(ADD_BOOK_MSG, response);
        }
        if (!strcmp(buffer, "delete_book")) {
            /* /api/v1/tema/library/books/:bookId */
            delete_book_ex();
            char *book_id = calloc(10, sizeof(char));
            char *book_url = calloc(BUFLEN, sizeof(char));
            scanf("%s", book_id);
            if (check_for_valid(GET_BOOK_MSG, book_id) == 0) {
                printf("Incorrect line, please try again! Re-write\n");
                continue;
            }
            get_input(&book_id);
            sprintf(book_url, "%s%s", URL_BOOK_NUM, book_id);
            if (user_jwt == NULL) {
                message = compute_delete_request(HOST_IP, book_url, NULL, NULL, NULL, 0);
            } else {
                message = compute_delete_request(HOST_IP, book_url, NULL, user_jwt, NULL, 0);
            }
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            client_output(DELETE_BOOK, response);
        }
        if (!strcmp(buffer, "logout")) {
            //printf("%s\n", user_cookies);
            sockfd = open_connection(HOST_IP, 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(HOST_IP, URL_LOGOUT, NULL, NULL, &user_cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            client_output(LOGOUT_MSG, response);
            free(user_cookies);
            free(user_jwt);
            user_cookies = NULL;
            user_jwt = NULL;
        }
        if (!strcmp(buffer, "exit")) {
            close_connection(sockfd);
            free(buffer);
            break;
        }
    }
    return 0;
}
