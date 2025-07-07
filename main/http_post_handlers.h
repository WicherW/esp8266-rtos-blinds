#ifndef HTTP_POST_HANDLERS_H
#define HTTP_POST_HANDLERS_H

#include "esp_err.h"
#include "esp_http_server.h"


extern httpd_uri_t parameters_t;
extern httpd_uri_t schedule_t;

/**
 * @brief Function handling the POST request for the "/parameters" endpoint.
 * TODO: provide a detailed description of the functionality.
 * 
 * @param req Pointer to the httpd_req structure containing request information.
 * @return Returns ESP_OK on success, or another error code on failure.
 */
esp_err_t parameters_post_handler(httpd_req_t *req);


/**
 * @brief Function handling the POST request for the "/schedule" endpoint.
 * TODO: provide a detailed description of the functionality.
 * 
 * @param req Pointer to the httpd_req structure containing request information.
 * @return Returns ESP_OK on success, or another error code on failure.
 */
esp_err_t schedule_post_handler(httpd_req_t *req);


#endif
