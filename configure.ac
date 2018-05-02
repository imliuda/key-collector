AC_INIT([osclt], [1.0], [imliuda@163.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CC
AM_PROG_CC_C_O
AC_CHECK_HEADER(uv.h, , AC_MSG_FAILURE("can't find header uv.h", 1))
AC_CHECK_LIB(uv, uv_run, , AC_MSG_FAILURE("can't find library libuv", 1))
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  test/Makefile
])
AC_OUTPUT
