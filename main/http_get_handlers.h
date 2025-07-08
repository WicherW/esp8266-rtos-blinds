#ifndef HTTP_GET_HANDLERS_H
#define HTTP_GET_HANDLERS_H

#include "esp_err.h"
#include "esp_http_server.h"

#define JSON_CONST_SIZE 150
#define JSON_BLIND_DATA 120

extern httpd_uri_t main_page_t;
extern httpd_uri_t small_dev_data_t;
extern httpd_uri_t big_dev_data_t;
extern httpd_uri_t fill_inputs_t;


/**
 * @brief Handler for processing GET requests for the main page.
 * 
 * @param req Pointer to the httpd_req_t structure.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t html_get_handler(httpd_req_t *req);


/**
 * @brief Handler for processing GET requests for data from the big blind.
 * 
 * @param req Pointer to the httpd_req_t structure.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t devdata_get_feedback_bigblind_handler(httpd_req_t *req);


/**
 * @brief Handler for processing GET requests for data from the small blind.
 * 
 * @param req Pointer to the httpd_req_t structure.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t devdata_get_feedback_smallblind_handler(httpd_req_t *req);


/**
 * @brief Handler for processing GET requests for the schedule data.
 * 
 * @param req Pointer to the httpd_req_t structure.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t fillinputs_get_feedback_handler(httpd_req_t *req);


#endif