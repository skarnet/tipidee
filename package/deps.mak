#
# This file has been generated by tools/gen-deps.sh
#

src/include/tipidee/conf.h: src/include/tipidee/uri.h
src/include/tipidee/response.h: src/include/tipidee/rql.h
src/include/tipidee/rql.h: src/include/tipidee/method.h src/include/tipidee/uri.h
src/include/tipidee/tipidee.h: src/include/tipidee/body.h src/include/tipidee/conf.h src/include/tipidee/config.h src/include/tipidee/headers.h src/include/tipidee/method.h src/include/tipidee/response.h src/include/tipidee/rql.h src/include/tipidee/uri.h
src/tipideed/tipideed-internal.h: src/include/tipidee/tipidee.h
src/config/confnode.o src/config/confnode.lo: src/config/confnode.c src/config/tipidee-config-internal.h
src/config/conftree.o src/config/conftree.lo: src/config/conftree.c src/config/tipidee-config-internal.h
src/config/defaults.o src/config/defaults.lo: src/config/defaults.c src/config/tipidee-config-internal.h
src/config/lexparse.o src/config/lexparse.lo: src/config/lexparse.c src/config/tipidee-config-internal.h src/include/tipidee/config.h
src/config/tipidee-config-preprocess.o src/config/tipidee-config-preprocess.lo: src/config/tipidee-config-preprocess.c
src/config/tipidee-config.o src/config/tipidee-config.lo: src/config/tipidee-config.c src/config/tipidee-config-internal.h src/include/tipidee/config.h
src/libtipidee/tipidee_chunked_read.o src/libtipidee/tipidee_chunked_read.lo: src/libtipidee/tipidee_chunked_read.c src/include/tipidee/body.h
src/libtipidee/tipidee_conf_free.o src/libtipidee/tipidee_conf_free.lo: src/libtipidee/tipidee_conf_free.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_get.o src/libtipidee/tipidee_conf_get.lo: src/libtipidee/tipidee_conf_get.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_get_argv.o src/libtipidee/tipidee_conf_get_argv.lo: src/libtipidee/tipidee_conf_get_argv.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_get_content_type.o src/libtipidee/tipidee_conf_get_content_type.lo: src/libtipidee/tipidee_conf_get_content_type.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_get_redirection.o src/libtipidee/tipidee_conf_get_redirection.lo: src/libtipidee/tipidee_conf_get_redirection.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_get_string.o src/libtipidee/tipidee_conf_get_string.lo: src/libtipidee/tipidee_conf_get_string.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_get_uint32.o src/libtipidee/tipidee_conf_get_uint32.lo: src/libtipidee/tipidee_conf_get_uint32.c src/include/tipidee/conf.h
src/libtipidee/tipidee_conf_init.o src/libtipidee/tipidee_conf_init.lo: src/libtipidee/tipidee_conf_init.c src/include/tipidee/conf.h
src/libtipidee/tipidee_headers_get_content_length.o src/libtipidee/tipidee_headers_get_content_length.lo: src/libtipidee/tipidee_headers_get_content_length.c src/include/tipidee/headers.h
src/libtipidee/tipidee_headers_init.o src/libtipidee/tipidee_headers_init.lo: src/libtipidee/tipidee_headers_init.c src/include/tipidee/headers.h
src/libtipidee/tipidee_headers_parse.o src/libtipidee/tipidee_headers_parse.lo: src/libtipidee/tipidee_headers_parse.c src/include/tipidee/headers.h
src/libtipidee/tipidee_headers_search.o src/libtipidee/tipidee_headers_search.lo: src/libtipidee/tipidee_headers_search.c src/include/tipidee/headers.h
src/libtipidee/tipidee_method_conv_table.o src/libtipidee/tipidee_method_conv_table.lo: src/libtipidee/tipidee_method_conv_table.c src/include/tipidee/method.h
src/libtipidee/tipidee_method_tonum.o src/libtipidee/tipidee_method_tonum.lo: src/libtipidee/tipidee_method_tonum.c src/include/tipidee/method.h
src/libtipidee/tipidee_method_tostr.o src/libtipidee/tipidee_method_tostr.lo: src/libtipidee/tipidee_method_tostr.c src/include/tipidee/method.h
src/libtipidee/tipidee_response_error.o src/libtipidee/tipidee_response_error.lo: src/libtipidee/tipidee_response_error.c src/include/tipidee/method.h src/include/tipidee/response.h src/include/tipidee/rql.h
src/libtipidee/tipidee_response_header_builtin.o src/libtipidee/tipidee_response_header_builtin.lo: src/libtipidee/tipidee_response_header_builtin.c src/include/tipidee/config.h src/include/tipidee/response.h
src/libtipidee/tipidee_response_header_common_put.o src/libtipidee/tipidee_response_header_common_put.lo: src/libtipidee/tipidee_response_header_common_put.c src/include/tipidee/config.h src/include/tipidee/response.h
src/libtipidee/tipidee_response_header_date_fmt.o src/libtipidee/tipidee_response_header_date_fmt.lo: src/libtipidee/tipidee_response_header_date_fmt.c src/include/tipidee/response.h
src/libtipidee/tipidee_response_status.o src/libtipidee/tipidee_response_status.lo: src/libtipidee/tipidee_response_status.c src/include/tipidee/response.h
src/libtipidee/tipidee_rql_read.o src/libtipidee/tipidee_rql_read.lo: src/libtipidee/tipidee_rql_read.c src/include/tipidee/method.h src/include/tipidee/rql.h src/include/tipidee/uri.h
src/libtipidee/tipidee_uri_parse.o src/libtipidee/tipidee_uri_parse.lo: src/libtipidee/tipidee_uri_parse.c src/include/tipidee/uri.h
src/tipideed/cgi.o src/tipideed/cgi.lo: src/tipideed/cgi.c src/include/tipidee/headers.h src/include/tipidee/method.h src/include/tipidee/response.h src/include/tipidee/uri.h src/tipideed/tipideed-internal.h
src/tipideed/harden.o src/tipideed/harden.lo: src/tipideed/harden.c src/tipideed/tipideed-internal.h
src/tipideed/log.o src/tipideed/log.lo: src/tipideed/log.c src/include/tipidee/method.h src/tipideed/tipideed-internal.h
src/tipideed/options.o src/tipideed/options.lo: src/tipideed/options.c src/include/tipidee/response.h src/tipideed/tipideed-internal.h
src/tipideed/regular.o src/tipideed/regular.lo: src/tipideed/regular.c src/include/tipidee/method.h src/include/tipidee/response.h src/tipideed/tipideed-internal.h
src/tipideed/responses.o src/tipideed/responses.lo: src/tipideed/responses.c src/include/tipidee/response.h src/include/tipidee/rql.h src/tipideed/tipideed-internal.h
src/tipideed/send_file.o src/tipideed/send_file.lo: src/tipideed/send_file.c src/tipideed/tipideed-internal.h
src/tipideed/tipideed.o src/tipideed/tipideed.lo: src/tipideed/tipideed.c src/include/tipidee/tipidee.h src/tipideed/tipideed-internal.h
src/tipideed/trace.o src/tipideed/trace.lo: src/tipideed/trace.c src/include/tipidee/method.h src/include/tipidee/response.h src/tipideed/tipideed-internal.h

tipidee-config: EXTRA_LIBS := -lskarnet ${SPAWN_LIB}
tipidee-config: src/config/tipidee-config.o src/config/confnode.o src/config/conftree.o src/config/defaults.o src/config/lexparse.o
tipidee-config-preprocess: EXTRA_LIBS := -lskarnet
tipidee-config-preprocess: src/config/tipidee-config-preprocess.o
ifeq ($(strip $(STATIC_LIBS_ARE_PIC)),)
libtipidee.a.xyzzy: src/libtipidee/tipidee_chunked_read.o src/libtipidee/tipidee_conf_free.o src/libtipidee/tipidee_conf_get.o src/libtipidee/tipidee_conf_get_argv.o src/libtipidee/tipidee_conf_get_content_type.o src/libtipidee/tipidee_conf_get_redirection.o src/libtipidee/tipidee_conf_get_string.o src/libtipidee/tipidee_conf_get_uint32.o src/libtipidee/tipidee_conf_init.o src/libtipidee/tipidee_headers_get_content_length.o src/libtipidee/tipidee_headers_init.o src/libtipidee/tipidee_headers_parse.o src/libtipidee/tipidee_headers_search.o src/libtipidee/tipidee_method_conv_table.o src/libtipidee/tipidee_method_tonum.o src/libtipidee/tipidee_method_tostr.o src/libtipidee/tipidee_response_error.o src/libtipidee/tipidee_response_header_builtin.o src/libtipidee/tipidee_response_header_common_put.o src/libtipidee/tipidee_response_header_date_fmt.o src/libtipidee/tipidee_response_status.o src/libtipidee/tipidee_rql_read.o src/libtipidee/tipidee_uri_parse.o
else
libtipidee.a.xyzzy: src/libtipidee/tipidee_chunked_read.lo src/libtipidee/tipidee_conf_free.lo src/libtipidee/tipidee_conf_get.lo src/libtipidee/tipidee_conf_get_argv.lo src/libtipidee/tipidee_conf_get_content_type.lo src/libtipidee/tipidee_conf_get_redirection.lo src/libtipidee/tipidee_conf_get_string.lo src/libtipidee/tipidee_conf_get_uint32.lo src/libtipidee/tipidee_conf_init.lo src/libtipidee/tipidee_headers_get_content_length.lo src/libtipidee/tipidee_headers_init.lo src/libtipidee/tipidee_headers_parse.lo src/libtipidee/tipidee_headers_search.lo src/libtipidee/tipidee_method_conv_table.lo src/libtipidee/tipidee_method_tonum.lo src/libtipidee/tipidee_method_tostr.lo src/libtipidee/tipidee_response_error.lo src/libtipidee/tipidee_response_header_builtin.lo src/libtipidee/tipidee_response_header_common_put.lo src/libtipidee/tipidee_response_header_date_fmt.lo src/libtipidee/tipidee_response_status.lo src/libtipidee/tipidee_rql_read.lo src/libtipidee/tipidee_uri_parse.lo
endif
libtipidee.so.xyzzy: EXTRA_LIBS := -lskarnet
libtipidee.so.xyzzy: src/libtipidee/tipidee_chunked_read.lo src/libtipidee/tipidee_conf_free.lo src/libtipidee/tipidee_conf_get.lo src/libtipidee/tipidee_conf_get_argv.lo src/libtipidee/tipidee_conf_get_content_type.lo src/libtipidee/tipidee_conf_get_redirection.lo src/libtipidee/tipidee_conf_get_string.lo src/libtipidee/tipidee_conf_get_uint32.lo src/libtipidee/tipidee_conf_init.lo src/libtipidee/tipidee_headers_get_content_length.lo src/libtipidee/tipidee_headers_init.lo src/libtipidee/tipidee_headers_parse.lo src/libtipidee/tipidee_headers_search.lo src/libtipidee/tipidee_method_conv_table.lo src/libtipidee/tipidee_method_tonum.lo src/libtipidee/tipidee_method_tostr.lo src/libtipidee/tipidee_response_error.lo src/libtipidee/tipidee_response_header_builtin.lo src/libtipidee/tipidee_response_header_common_put.lo src/libtipidee/tipidee_response_header_date_fmt.lo src/libtipidee/tipidee_response_status.lo src/libtipidee/tipidee_rql_read.lo src/libtipidee/tipidee_uri_parse.lo
tipideed: EXTRA_LIBS := -lskarnet
tipideed: src/tipideed/tipideed.o src/tipideed/cgi.o src/tipideed/harden.o src/tipideed/log.o src/tipideed/options.o src/tipideed/regular.o src/tipideed/responses.o src/tipideed/send_file.o src/tipideed/tipideed.o src/tipideed/trace.o libtipidee.a.xyzzy
INTERNAL_LIBS :=
