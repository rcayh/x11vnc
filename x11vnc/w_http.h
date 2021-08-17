
#ifndef W_HTTP_H_
#define W_HTTP_H_


#define HTTP_METHOD_GET 	"GET"
#define HTTP_METHOD_POST 	"POST"

#define SCHEME_HTTP 		"http"
#define SCHEME_HTTPS 		"https"

#include "w_common.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//neon
#include <ne_session.h>
#include <ne_request.h>
#include <ne_utils.h>
#include <ne_uri.h>


bool request_acceptnum(ControlWaitInfoPtr data);

#endif
