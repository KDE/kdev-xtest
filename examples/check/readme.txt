A tiny examplary Check test suite. To run it you will need

*/ Check
*/ KDevelop4
*/ KDevelop4 check main wrapper, ie `checkwrapper.h' with the
   CHECK_XTEST_MAIN macro. This should be located in
   kdevelop/veritas/ on your KDE4 include path.

Steps required [will get user-friendlier]:

*/ Run Kdevelop4
*/ Open project 'check.kdev4'
*/ Build, by right click -> build item(s) in the project
   toolview
*/ Configure check toolview:
      Project -> Configure -> Check -> point to check_money
*/ Open Check toolview:
      View -> Add Tool View -> Check Runner

Now the Money test case should be visible. The create_zero test
is expected to fail with a seg fault.
