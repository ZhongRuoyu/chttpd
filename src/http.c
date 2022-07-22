#include "http.h"

#include <stdlib.h>
#include <string.h>

RequestMethod GetRequestMethod(const char *request_method) {
    for (size_t i = 0; i < sizeof kRequestMethods / sizeof(const char *); ++i) {
        if (strcmp(request_method, kRequestMethods[i]) == 0) {
            return i;
        }
    }
    return kUndefined;
}

const char *GetResponseStatus(ResponseStatusCode code) {
    switch (code) {
        case kContinue:
            return "100 Continue";
        case kSwitchingProtocols:
            return "101 Switching Protocols";
        case kProcessing:
            return "102 Processing";
        case kEarlyHints:
            return "103 Early Hints";

        case kOK:
            return "200 OK";
        case kCreated:
            return "201 Created";
        case kAccepted:
            return "202 Accepted";
        case kNonAuthoritativeInformation:
            return "203 Non-Authoritative Information";
        case kNoContent:
            return "204 No Content";
        case kResetContent:
            return "205 Reset Content";
        case kPartialContent:
            return "206 Partial Content";
        case kMultiStatus:
            return "207 Multi-Status";
        case kAlreadyReported:
            return "208 Already Reported";
        case kIMUsed:
            return "226 IM Used";

        case kMultipleChoices:
            return "300 Multiple Choices";
        case kMovedPermanently:
            return "301 Moved Permanently";
        case kFound:
            return "302 Found";
        case kSeeOther:
            return "303 See Other";
        case kNotModified:
            return "304 Not Modified";
        case kUseProxy:
            return "305 Use Proxy";
        case kTemporaryRedirect:
            return "307 Temporary Redirect";
        case kPermanentRedirect:
            return "308 Permanent Redirect";

        case kBadRequest:
            return "400 Bad Request";
        case kUnauthorized:
            return "401 Unauthorized";
        case kPaymentRequired:
            return "402 Payment Required";
        case kForbidden:
            return "403 Forbidden";
        case kNotFound:
            return "404 Not Found";
        case kMethodNotAllowed:
            return "405 Method Not Allowed";
        case kNotAcceptable:
            return "406 Not Acceptable";
        case kProxyAuthenticationRequired:
            return "407 Proxy Authentication Required";
        case kRequestTimeout:
            return "408 Request Timeout";
        case kConflict:
            return "409 Conflict";
        case kGone:
            return "410 Gone";
        case kLengthRequired:
            return "411 Length Required";
        case kPreconditionFailed:
            return "412 Precondition Failed";
        case kPayloadTooLarge:
            return "413 Payload Too Large";
        case kURITooLong:
            return "414 URI Too Long";
        case kUnsupportedMediaType:
            return "415 Unsupported Media Type";
        case kRangeNotSatisfiable:
            return "416 Range Not Satisfiable";
        case kExpectationFailed:
            return "417 Expectation Failed";
        case kImATeapot:
            return "418 I'm a teapot";
        case kMisdirectedRequest:
            return "421 Misdirected Request";
        case kUnprocessableEntity:
            return "422 Unprocessable Entity";
        case kLocked:
            return "423 Locked";
        case kFailedDependency:
            return "424 Failed Dependency";
        case kTooEarly:
            return "425 Too Early Experimental";
        case kUpgradeRequired:
            return "426 Upgrade Required";
        case kPreconditionRequired:
            return "428 Precondition Required";
        case kTooManyRequests:
            return "429 Too Many Requests";
        case kRequestHeaderFieldsTooLarge:
            return "431 Request Header Fields Too Large";
        case kUnavailableForLegalReasons:
            return "451 Unavailable For Legal Reasons";

        case kInternalServerError:
            return "500 Internal Server Error";
        case kNotImplemented:
            return "501 Not Implemented";
        case kBadGateway:
            return "502 Bad Gateway";
        case kServiceUnavailable:
            return "503 Service Unavailable";
        case kGatewayTimeout:
            return "504 Gateway Timeout";
        case kHTTPVersionNotSupported:
            return "505 HTTP Version Not Supported";
        case kVariantAlsoNegotiates:
            return "506 Variant Also Negotiates";
        case kInsufficientStorage:
            return "507 Insufficient Storage";
        case kLoopDetected:
            return "508 Loop Detected";
        case kNotExtended:
            return "510 Not Extended";
        case kNetworkAuthenticationRequired:
            return "511 Network Authentication Required";

        default:
            return NULL;
    }
}