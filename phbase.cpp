/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_phbase.h"
#include "Zend/zend_exceptions.h"
#include "ext/standard/php_string.h"

#include "Hbase.h"
#include <vector>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::hadoop::hbase::thrift;



/* PHBase class entry */
zend_class_entry *phbase_ce = NULL, *phbase_exception_ce = NULL;

#define OBJECT_ARRAY_INCREASE_STEP 2

typedef struct {
    void** array;
    int size;
}object_array;

object_array phbase_object_array = {0};

void object_array_init(){
    phbase_object_array.size = OBJECT_ARRAY_INCREASE_STEP;
    phbase_object_array.array = (void**)malloc(sizeof(void*) * OBJECT_ARRAY_INCREASE_STEP);
}

typedef struct {
    HbaseClient *client = NULL;
} phbase_object;


static zend_always_inline void* phbase_get_object(zval *object)
{
#if PHP_MAJOR_VERSION < 7
    zend_object_handle handle = Z_OBJ_HANDLE_P(object);
#else
    int handle = (int)Z_OBJ_HANDLE(*object);
#endif
    assert(handle < phbase_object_array.size);
    return phbase_object_array.array[handle];
}

void zend_always_inline phbase_set_object(zval *object, void *ptr)
{
#if PHP_MAJOR_VERSION < 7
    zend_object_handle handle = Z_OBJ_HANDLE_P(object);
#else
    int handle = (int) Z_OBJ_HANDLE(*object);
#endif

    if (handle >= phbase_object_array.size)
    {
        uint32_t old_size = phbase_object_array.size;
        uint32_t new_size = old_size + OBJECT_ARRAY_INCREASE_STEP;

        void *old_ptr = phbase_object_array.array;
        void *new_ptr = NULL;

        new_ptr = realloc(old_ptr, sizeof(void*) * new_size);
        if (!new_ptr)
        {
            zend_error(E_ERROR, "malloc(%d) failed.", (int )(new_size * sizeof(void *)));
            return;
        }
//        printf("new size %d \n", new_size);
        bzero(new_ptr + (old_size * sizeof(void*)), (new_size - old_size) * sizeof(void*));
        phbase_object_array.array = (void**)new_ptr;
        phbase_object_array.size = new_size;
    }
    phbase_object_array.array[handle] = ptr;
}

/* {{{ CoThread::__construct()
 */
ZEND_METHOD(phbase, __construct) {
    char *host = NULL;
    size_t host_len = 0;
    long port = 0;
    char *strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &host, &host_len, &port) == FAILURE) {
    	return;
    }


    zend_update_property_string(phbase_ce, getThis(), "host", 4, (const char*)host);
    zend_update_property_long(phbase_ce, getThis(), "port", 4, port);

    phbase_object *obj = (phbase_object*)malloc(sizeof(phbase_object));



    boost::shared_ptr<TSocket> socket(new TSocket(host, port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    obj->client =  new HbaseClient(protocol);

    phbase_set_object(getThis(), obj);

}
/* }}} */

/* {{{ CoThread::__construct()
 */
ZEND_METHOD(phbase, __destruct) {
    phbase_object *obj = (phbase_object *)phbase_get_object(getThis());
    obj->client->getInputProtocol()->getInputTransport()->close();
    delete obj->client;
    free(obj);
    phbase_set_object(getThis(), NULL);
}
/* }}} */

/* {{{ PHBase::connect()
 */
ZEND_METHOD(phbase, connect) {
    phbase_object *obj = (phbase_object *)phbase_get_object(getThis());

    try{
        obj->client->getInputProtocol()->getInputTransport()->open();
        RETURN_TRUE
    }
    catch( apache::thrift::transport::TTransportException e){
        RETURN_FALSE
    }

}
/* }}} */


/* {{{ PHBase::get()
 */
ZEND_METHOD(phbase, get) {

    char *table_name = NULL, *row_key = NULL, *family = NULL, *qualifier = NULL;
    size_t table_name_len = 0, row_key_len = 0, family_len = 0, qualifier_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s", &table_name, &table_name_len, &row_key, &row_key_len, &family, &family_len, &qualifier, &qualifier_len) == FAILURE) {
    	return;
    }

    phbase_object *obj = (phbase_object *)phbase_get_object(getThis());

    std::vector<TCell> list;
    Text table(table_name);
    Text row(row_key);
    Text column(family);
    std::map<Text, Text> attributes;




    try{
        obj->client->get(list, table, row, family, attributes);
    }
    catch(apache::thrift::transport::TTransportException e){
        zend_throw_exception(phbase_exception_ce, e.what(), -1);
        RETVAL_NULL(); return;
    }

    std::vector<TCell>::const_iterator iter;
    array_init(return_value);
    for(iter=list.begin();iter!=list.end();iter++) {
        add_next_index_string(return_value, (*iter).value.c_str(), 0);
    }


}
/* }}} */


/* {{{ PHBase::exists()
 */
ZEND_METHOD(phbase, exists) {
//
//    char *table_name = NULL, *row_key = NULL, *family = NULL, *qualifier = NULL;
//    size_t table_name_len = 0, row_key_len = 0, family_len = 0, qualifier_len = 0;
//
//    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|ss", &table_name, &table_name_len, &row_key, &row_key_len, &family, &family_len, &qualifier, &qualifier_len) == FAILURE) {
//    	return;
//    }
//
//    phbase_object *obj = (phbase_object *)phbase_get_object(getThis());
//
//    TResult tresult;
//    TGet get;
//    const std::string table(table_name);
//    const std::string row(row_key);
//    get.__set_row(row);
//
//    if (family) {
//        std::vector<TColumn> vec;
//        TColumn c;
//        c.__set_family(family);
//        if (qualifier){
//            c.__set_qualifier(qualifier);
//        }
//        vec.push_back(c);
//        get.__set_columns(vec);
//    }
//
//    try{
//        bool exists = obj->client->exists(table, get);
//        if (true == exists){
//            RETURN_TRUE
//        }
//        RETURN_FALSE
//    }
//    catch(apache::thrift::transport::TTransportException e){
//        zend_throw_exception(phbase_exception_ce, e.what(), -1);
//        RETVAL_NULL(); return;
//    }
//    catch(apache::hadoop::hbase::thrift2::TIOError e){
//        printf("\n\napache::hadoop::hbase::thrift2::TIOError::message=%s\n\n", e.message.c_str());
//        RETURN_FALSE
//    }


}
/* }}} */


ZEND_BEGIN_ARG_INFO_EX(arginfo_phbase_get, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, table, 0, 0)
	ZEND_ARG_TYPE_INFO(0, row_key, 0, 0)
	ZEND_ARG_TYPE_INFO(0, family, 0, 0)
	ZEND_ARG_TYPE_INFO(0, qualifier, 0, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phbase_exists, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, table, 0, 0)
	ZEND_ARG_TYPE_INFO(0, row_key, 0, 0)
ZEND_END_ARG_INFO()

/* PHBase class method entry */
/* {{{ phbase_method */
static zend_function_entry phbase_method[] = {
	//ZEND_ME(phbase, yield, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(phbase, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(phbase, __destruct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR|ZEND_ACC_FINAL)
    ZEND_ME(phbase, connect, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(phbase, get, arginfo_phbase_get, ZEND_ACC_PUBLIC)
    ZEND_ME(phbase, exists, arginfo_phbase_exists, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};
/* }}} */

/* PHBase Exception class method entry */
/* {{{ phbase_exception_method */
static zend_function_entry phbase_exception_method[] = {
    { NULL, NULL, NULL }
};
/* }}} */


PHP_MINIT_FUNCTION(phbase)
{

    zend_class_entry ce;
    zend_class_entry exception_ce;

    INIT_CLASS_ENTRY(ce, "PHBase",phbase_method);
    INIT_CLASS_ENTRY(exception_ce, "PHBaseException",phbase_exception_method);

    phbase_ce = zend_register_internal_class(&ce TSRMLS_CC);
    phbase_exception_ce = zend_register_internal_class_ex(&exception_ce, zend_exception_get_default(), NULL TSRMLS_CC);

    zend_declare_property_string(phbase_ce, "host", 4, "127.0.0.1", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(phbase_ce, "port", 4, 9090, ZEND_ACC_PROTECTED TSRMLS_CC);

    object_array_init();

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(phbase)
{
	return SUCCESS;
}



PHP_RINIT_FUNCTION(phbase)
{
	return SUCCESS;
}



PHP_RSHUTDOWN_FUNCTION(phbase)
{
	return SUCCESS;
}


PHP_MINFO_FUNCTION(phbase)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "phbase support", "enabled");
	php_info_print_table_end();

}


const zend_function_entry phbase_functions[] = {
	PHP_FE_END
};


zend_module_entry phbase_module_entry = {
	STANDARD_MODULE_HEADER,
	"phbase",
	phbase_functions,
	PHP_MINIT(phbase),
	PHP_MSHUTDOWN(phbase),
	PHP_RINIT(phbase),
	PHP_RSHUTDOWN(phbase),
	PHP_MINFO(phbase),
	PHP_PHBASE_VERSION,
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_PHBASE
ZEND_GET_MODULE(phbase)
#endif

