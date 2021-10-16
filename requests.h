#ifndef _REQUESTS_
#define _REQUESTS_

#define HOST_IP "3.8.116.10"
#define MAX_LEN 15
#define URL_LOGIN "/api/v1/tema/auth/login"
#define URL_REGISTER "/api/v1/tema/auth/register"
#define URL_ACCESS "/api/v1/tema/library/access"
#define URL_BOOKS "/api/v1/tema/library/books"
#define URL_BOOK_NUM "/api/v1/tema/library/books/"
#define URL_LOGOUT "/api/v1/tema/auth/logout"
#define CONTENT_TYPE "application/json"
#define COOKIES_LEN 9
#define JWT_LEN 14
#define INPUT_LEN 50
#define PORT 8080
#define REGISTER_MSG 0
#define LOGIN_MSG 1
#define ADD_BOOK_MSG 2
#define GET_BOOK_MSG 3
#define GET_BOOKS_MSG 4
#define ENTER_LIBRARY 5
#define DELETE_BOOK 6
#define LOGOUT_MSG 7
#define EXIT_MSG 8

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params, char *jwt,
							char **cookies, int cookies_count);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, char *jwt, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count);

char *compute_delete_request(char *host, char *url, char *query_params, char *jwt,
                            char **cookies, int cookies_count);
#endif
