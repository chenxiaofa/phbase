dnl $Id$
dnl config.m4 for extension phbase


 PHP_ARG_WITH(phbase, for phbase support,
 Make sure that the comment is aligned:
 [  --with-phbase             Include phbase support])


if test "$PHP_PHBASE" != "no"; then
  dnl Write more examples of tests here...

  if test ! -f ./thirdparty/installed/bin/thrift;then
    AC_MSG_ERROR("no thrift bin was found")
  fi

  ./thirdparty/installed/bin/thrift --gen cpp --out . hbase.thrift



  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++, "", EXTRA_LDFLAGS)
  PHP_ADD_LIBRARY_WITH_PATH(thrift, ./thirdparty/installed/lib, EXTRA_LDFLAGS)
  PHP_ADD_INCLUDE(./thirdparty/installed/include/)
  CXXFLAGS="${CXXFLAGS} -std=c++11"

  PHP_NEW_EXTENSION(phbase, hbase_constants.cpp hbase_types.cpp THBaseService.cpp phbase.cpp, $ext_shared)

fi
