#include "../../include/ssl/SslContext.h"
#include <openssl/err.h>
#include <muduo/base/Logging.h>

namespace ssl {

SslContext::SslContext(const SslConfig& config)
    : ctx_(nullptr), config_(config)
{
}

SslContext::~SslContext()
{
    if (ctx_) {
        SSL_CTX_free(ctx_);
        ctx_ = nullptr;
    }
}

bool SslContext::initialize()
{
    ctx_ = SSL_CTX_new(TLS_server_method());
    if (!ctx_) {
        handleSslError("SSL_CTX_new failed");
        return false;
    }
    if (!setupProtocol()) return false;
    if (!loadCertificates()) return false;
    setupSessionCache();
    return true;
}

bool SslContext::loadCertificates()
{
    if (SSL_CTX_use_certificate_file(ctx_, config_.getCertificateFile().c_str(), SSL_FILETYPE_PEM) <= 0) {
        handleSslError("Failed to load certificate file");
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx_, config_.getPrivateKeyFile().c_str(), SSL_FILETYPE_PEM) <= 0) {
        handleSslError("Failed to load private key file");
        return false;
    }
    if (!SSL_CTX_check_private_key(ctx_)) {
        handleSslError("Private key does not match certificate");
        return false;
    }
    return true;
}

bool SslContext::setupProtocol()
{
    SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);
    if (!config_.getCipherList().empty()) {
        SSL_CTX_set_cipher_list(ctx_, config_.getCipherList().c_str());
    }
    return true;
}

void SslContext::setupSessionCache()
{
    SSL_CTX_set_session_cache_mode(ctx_, SSL_SESS_CACHE_SERVER);
    SSL_CTX_set_timeout(ctx_, config_.getSessionTimeout());
}

void SslContext::handleSslError(const char* msg)
{
    LOG_ERROR << msg;
    ERR_print_errors_fp(stderr);
}

} // namespace ssl
