# nginx
learning nginx in C++
使用valgrind检测Memcheck
valgrind --tool=memcheck --leak-check=full --show-reachable=yes --trace-children=yes bin/nginx
参数解释:

–leak-check=full 指的是完全检查内存泄漏，

–show-reachable=yes是显示内存泄漏的地点，

–trace-children=yes是跟入子进程