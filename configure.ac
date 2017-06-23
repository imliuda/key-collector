AC_INIT([osclt], [1.0], [imliuda@163.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CHECK_LIB(ev, ev_default_loop, , AC_MSG_FAILURE("can't find libev", 1))
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
