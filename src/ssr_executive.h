#if !defined(__SSR_EXECUTIVE__)
#define __SSR_EXECUTIVE__ 1


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

struct cipher_env_t;
struct obfs_t;
struct tunnel_ctx;
struct cstl_set;

struct server_config {
    char *listen_host;
    unsigned short listen_port;
    char *remote_host;
    unsigned short remote_port;
    char *password;
    char *method;
    char *protocol;
    char *protocol_param;
    char *obfs;
    char *obfs_param;
    bool udp;
    unsigned int idle_timeout; /* Connection idle timeout in ms. */
    char *remarks;
};

#if !defined(_LOCAL_H)
struct server_env_t {
    void *data;

    struct server_config *config; // __weak_ptr
    
    struct cstl_set *tunnel_set;

    struct cipher_env_t *cipher;

    void *protocol_global;
    void *obfs_global;
};
#endif // _LOCAL_H

struct tunnel_cipher_ctx {
    struct server_env_t *env; // __weak_ptr
    struct enc_ctx *e_ctx;
    struct enc_ctx *d_ctx;
    struct obfs_t *protocol; // __strong_ptr
    struct obfs_t *obfs; // __strong_ptr
};

#define SSR_ERR_MAP(V)                                                         \
  V( 0, ssr_ok,                 "All is OK.")                                  \
  V(-1, ssr_error_client_decode,      "client decode error.")                  \
  V(-2, ssr_error_invalid_password,   "invalid password or cipher.")           \
  V(-3, ssr_error_client_post_decrypt,"client post decrypt error.")            \

typedef enum ssr_error {
#define SSR_ERR_GEN(code, name, _) name = code,
    SSR_ERR_MAP(SSR_ERR_GEN)
#undef SSR_ERR_GEN
    ssr_max_errors,
} ssr_error;

const char *ssr_strerror(enum ssr_error err);

struct tunnel_cipher_ctx;
struct buffer_t;

void object_safe_free(void **obj);
void string_safe_assign(char **target, const char *value);

#define SECONDS_PER_MINUTE    1000

#define DEFAULT_BIND_HOST     "127.0.0.1"
#define DEFAULT_BIND_PORT     1080
#define DEFAULT_IDLE_TIMEOUT  (60 * SECONDS_PER_MINUTE)
#define DEFAULT_METHOD        "rc4-md5"

#if !defined(TCP_BUF_SIZE_MAX)
#define TCP_BUF_SIZE_MAX 32 * 1024
#endif

struct server_config * config_create(void);
void config_release(struct server_config *cf);
void config_change_for_server(struct server_config *config);

struct server_env_t * ssr_cipher_env_create(struct server_config *config, void *data);
void ssr_cipher_env_release(struct server_env_t *env);
bool is_completed_package(struct server_env_t *env, const uint8_t *data, size_t size);

struct cstl_set * objects_container_create(void);
void objects_container_destroy(struct cstl_set *set);
void objects_container_add(struct cstl_set *set, void *obj);
void objects_container_remove(struct cstl_set *set, void *obj);
void objects_container_traverse(struct cstl_set *set, void(*fn)(void *obj, void *p), void *p);

struct cstl_map;
struct cstl_map * obj_map_create(int(*compare_key)(void*,void*), void (*destroy_key)(void*), void (*destroy_value)(void*));
void obj_map_destroy(struct cstl_map *map);
bool obj_map_add(struct cstl_map *map, void *key, size_t k_size, void *value, size_t v_size);
bool obj_map_exists(struct cstl_map *map, void *key);
bool obj_map_replace(struct cstl_map *map, void *key, void *value, size_t v_size);
void obj_map_remove(struct cstl_map *map, void *key);
const void * obj_map_find(struct cstl_map *map, const void *key);
void obj_map_traverse(struct cstl_map *map, void(*fn)(const void *key, const void *value, void *p), void *p);

struct tunnel_cipher_ctx * tunnel_cipher_create(struct server_env_t *env, const struct buffer_t *init_pkg, size_t tcp_mss);
void tunnel_cipher_release(struct tunnel_cipher_ctx *tc);
bool tunnel_cipher_client_need_feedback(struct tunnel_cipher_ctx *tc);
enum ssr_error tunnel_cipher_client_encrypt(struct tunnel_cipher_ctx *tc, struct buffer_t *buf);
enum ssr_error tunnel_cipher_client_decrypt(struct tunnel_cipher_ctx *tc, struct buffer_t *buf, struct buffer_t **feedback);

struct buffer_t * tunnel_cipher_server_encrypt(struct tunnel_cipher_ctx *tc, const struct buffer_t *buf);
struct buffer_t * tunnel_cipher_server_decrypt(struct tunnel_cipher_ctx *tc, const struct buffer_t *buf, struct buffer_t **receipt, struct buffer_t **confirm);

bool pre_parse_header(struct buffer_t *data);

#endif // defined(__SSR_EXECUTIVE__)
