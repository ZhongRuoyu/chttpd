#ifndef CHTTPD_HTTP_H_
#define CHTTPD_HTTP_H_

#include <ctype.h>
#include <string.h>

typedef enum {
    kGET = 1,
    kHEAD = 2,
    kPOST = 3,
    kPUT = 4,
    kDELETE = 5,
    kCONNECT = 6,
    kOPTIONS = 7,
    kTRACE = 8,
    kPATCH = 9,
} RequestMethod;

RequestMethod GetRequestMethod(const char *request_method);

typedef enum {
    // informational responses
    kContinue = 100,
    kSwitchingProtocols = 101,
    kProcessing = 102,
    kEarlyHints = 103,

    // successful responses
    kOK = 200,
    kCreated = 201,
    kAccepted = 202,
    kNonAuthoritativeInformation = 203,
    kNoContent = 204,
    kResetContent = 205,
    kPartialContent = 206,
    kMultiStatus = 207,
    kAlreadyReported = 208,
    kIMUsed = 226,

    // redirection messages
    kMultipleChoices = 300,
    kMovedPermanently = 301,
    kFound = 302,
    kSeeOther = 303,
    kNotModified = 304,
    kUseProxy = 305,
    kTemporaryRedirect = 307,
    kPermanentRedirect = 308,

    // client error responses
    kBadRequest = 400,
    kUnauthorized = 401,
    kPaymentRequired = 402,
    kForbidden = 403,
    kNotFound = 404,
    kMethodNotAllowed = 405,
    kNotAcceptable = 406,
    kProxyAuthenticationRequired = 407,
    kRequestTimeout = 408,
    kConflict = 409,
    kGone = 410,
    kLengthRequired = 411,
    kPreconditionFailed = 412,
    kPayloadTooLarge = 413,
    kURITooLong = 414,
    kUnsupportedMediaType = 415,
    kRangeNotSatisfiable = 416,
    kExpectationFailed = 417,
    kImATeapot = 418,
    kMisdirectedRequest = 421,
    kUnprocessableEntity = 422,
    kLocked = 423,
    kFailedDependency = 424,
    kTooEarly = 425,
    kUpgradeRequired = 426,
    kPreconditionRequired = 428,
    kTooManyRequests = 429,
    kRequestHeaderFieldsTooLarge = 431,
    kUnavailableForLegalReasons = 451,

    // server error responses
    kInternalServerError = 500,
    kNotImplemented = 501,
    kBadGateway = 502,
    kServiceUnavailable = 503,
    kGatewayTimeout = 504,
    kHTTPVersionNotSupported = 505,
    kVariantAlsoNegotiates = 506,
    kInsufficientStorage = 507,
    kLoopDetected = 508,
    kNotExtended = 510,
    kNetworkAuthenticationRequired = 511,
} ResponseStatusCode;

const char *GetResponseStatusString(ResponseStatusCode code);

int GetHTTPVersion(const char *http_version, int *http_version_major,
                   int *http_version_minor);

#endif  // CHTTPD_HTTP_H_
