AC_INIT([osclt], [1.0], [imliuda@163.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CC
AM_PROG_CC_C_O
AC_CHECK_HEADER(ev.h, , AC_MSG_FAILURE("can't find header ev.h, you need install libev devel packagei", 1))
AC_CHECK_HEADER(jansson.h, , AC_MSG_FAILURE("can't find header jansson.h, you need instal libev devel package", 1))
AC_CHECK_LIB(ev, ev_default_loop, , AC_MSG_FAILURE("can't find library libev", 1))
AC_CHECK_LIB(jansson, json_dumps, , AC_MSG_FAILURE("can't find library libjansson", 1))
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
