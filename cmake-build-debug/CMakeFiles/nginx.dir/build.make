# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/xufang/Downloads/clion-2019.3.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/xufang/Downloads/clion-2019.3.2/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/xufang/GitLab/nginx

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xufang/GitLab/nginx/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/nginx.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/nginx.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/nginx.dir/flags.make

CMakeFiles/nginx.dir/app/nginx.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/app/nginx.cpp.o: ../app/nginx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/nginx.dir/app/nginx.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/app/nginx.cpp.o -c /home/xufang/GitLab/nginx/app/nginx.cpp

CMakeFiles/nginx.dir/app/nginx.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/app/nginx.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/app/nginx.cpp > CMakeFiles/nginx.dir/app/nginx.cpp.i

CMakeFiles/nginx.dir/app/nginx.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/app/nginx.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/app/nginx.cpp -o CMakeFiles/nginx.dir/app/nginx.cpp.s

CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.o: ../app/ngx_c_conf.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.o -c /home/xufang/GitLab/nginx/app/ngx_c_conf.cpp

CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/app/ngx_c_conf.cpp > CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.i

CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/app/ngx_c_conf.cpp -o CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.s

CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.o: ../app/ngx_c_memory.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.o -c /home/xufang/GitLab/nginx/app/ngx_c_memory.cpp

CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/app/ngx_c_memory.cpp > CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.i

CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/app/ngx_c_memory.cpp -o CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.s

CMakeFiles/nginx.dir/app/ngx_log.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/app/ngx_log.cpp.o: ../app/ngx_log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/nginx.dir/app/ngx_log.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/app/ngx_log.cpp.o -c /home/xufang/GitLab/nginx/app/ngx_log.cpp

CMakeFiles/nginx.dir/app/ngx_log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/app/ngx_log.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/app/ngx_log.cpp > CMakeFiles/nginx.dir/app/ngx_log.cpp.i

CMakeFiles/nginx.dir/app/ngx_log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/app/ngx_log.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/app/ngx_log.cpp -o CMakeFiles/nginx.dir/app/ngx_log.cpp.s

CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.o: ../app/ngx_setproctitle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.o -c /home/xufang/GitLab/nginx/app/ngx_setproctitle.cpp

CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/app/ngx_setproctitle.cpp > CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.i

CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/app/ngx_setproctitle.cpp -o CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.s

CMakeFiles/nginx.dir/app/ngx_string.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/app/ngx_string.cpp.o: ../app/ngx_string.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/nginx.dir/app/ngx_string.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/app/ngx_string.cpp.o -c /home/xufang/GitLab/nginx/app/ngx_string.cpp

CMakeFiles/nginx.dir/app/ngx_string.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/app/ngx_string.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/app/ngx_string.cpp > CMakeFiles/nginx.dir/app/ngx_string.cpp.i

CMakeFiles/nginx.dir/app/ngx_string.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/app/ngx_string.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/app/ngx_string.cpp -o CMakeFiles/nginx.dir/app/ngx_string.cpp.s

CMakeFiles/nginx.dir/signal/ngx_signal.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/signal/ngx_signal.cpp.o: ../signal/ngx_signal.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/nginx.dir/signal/ngx_signal.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/signal/ngx_signal.cpp.o -c /home/xufang/GitLab/nginx/signal/ngx_signal.cpp

CMakeFiles/nginx.dir/signal/ngx_signal.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/signal/ngx_signal.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/signal/ngx_signal.cpp > CMakeFiles/nginx.dir/signal/ngx_signal.cpp.i

CMakeFiles/nginx.dir/signal/ngx_signal.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/signal/ngx_signal.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/signal/ngx_signal.cpp -o CMakeFiles/nginx.dir/signal/ngx_signal.cpp.s

CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.o: ../proc/ngx_daemon.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.o -c /home/xufang/GitLab/nginx/proc/ngx_daemon.cpp

CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/proc/ngx_daemon.cpp > CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.i

CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/proc/ngx_daemon.cpp -o CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.s

CMakeFiles/nginx.dir/proc/ngx_event.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/proc/ngx_event.cpp.o: ../proc/ngx_event.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/nginx.dir/proc/ngx_event.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/proc/ngx_event.cpp.o -c /home/xufang/GitLab/nginx/proc/ngx_event.cpp

CMakeFiles/nginx.dir/proc/ngx_event.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/proc/ngx_event.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/proc/ngx_event.cpp > CMakeFiles/nginx.dir/proc/ngx_event.cpp.i

CMakeFiles/nginx.dir/proc/ngx_event.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/proc/ngx_event.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/proc/ngx_event.cpp -o CMakeFiles/nginx.dir/proc/ngx_event.cpp.s

CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.o: ../proc/ngx_process_cycle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.o -c /home/xufang/GitLab/nginx/proc/ngx_process_cycle.cpp

CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/proc/ngx_process_cycle.cpp > CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.i

CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/proc/ngx_process_cycle.cpp -o CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.s

CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.o: ../net/ngx_c_socket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.o -c /home/xufang/GitLab/nginx/net/ngx_c_socket.cpp

CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/net/ngx_c_socket.cpp > CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.i

CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/net/ngx_c_socket.cpp -o CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.s

CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.o: ../net/ngx_c_socket_accept.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.o -c /home/xufang/GitLab/nginx/net/ngx_c_socket_accept.cpp

CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/net/ngx_c_socket_accept.cpp > CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.i

CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/net/ngx_c_socket_accept.cpp -o CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.s

CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.o: ../net/ngx_c_socket_conn.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.o -c /home/xufang/GitLab/nginx/net/ngx_c_socket_conn.cpp

CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/net/ngx_c_socket_conn.cpp > CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.i

CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/net/ngx_c_socket_conn.cpp -o CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.s

CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.o: ../net/ngx_c_socket_inet.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building CXX object CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.o -c /home/xufang/GitLab/nginx/net/ngx_c_socket_inet.cpp

CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/net/ngx_c_socket_inet.cpp > CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.i

CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/net/ngx_c_socket_inet.cpp -o CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.s

CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.o: ../net/ngx_c_socket_request.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building CXX object CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.o -c /home/xufang/GitLab/nginx/net/ngx_c_socket_request.cpp

CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/net/ngx_c_socket_request.cpp > CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.i

CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/net/ngx_c_socket_request.cpp -o CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.s

CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.o: CMakeFiles/nginx.dir/flags.make
CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.o: ../misc/ngx_c_threadpool.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building CXX object CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.o -c /home/xufang/GitLab/nginx/misc/ngx_c_threadpool.cpp

CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xufang/GitLab/nginx/misc/ngx_c_threadpool.cpp > CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.i

CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xufang/GitLab/nginx/misc/ngx_c_threadpool.cpp -o CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.s

# Object files for target nginx
nginx_OBJECTS = \
"CMakeFiles/nginx.dir/app/nginx.cpp.o" \
"CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.o" \
"CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.o" \
"CMakeFiles/nginx.dir/app/ngx_log.cpp.o" \
"CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.o" \
"CMakeFiles/nginx.dir/app/ngx_string.cpp.o" \
"CMakeFiles/nginx.dir/signal/ngx_signal.cpp.o" \
"CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.o" \
"CMakeFiles/nginx.dir/proc/ngx_event.cpp.o" \
"CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.o" \
"CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.o" \
"CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.o" \
"CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.o" \
"CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.o" \
"CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.o" \
"CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.o"

# External object files for target nginx
nginx_EXTERNAL_OBJECTS =

../bin/nginx: CMakeFiles/nginx.dir/app/nginx.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/app/ngx_c_conf.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/app/ngx_c_memory.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/app/ngx_log.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/app/ngx_setproctitle.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/app/ngx_string.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/signal/ngx_signal.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/proc/ngx_daemon.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/proc/ngx_event.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/proc/ngx_process_cycle.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/net/ngx_c_socket.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/net/ngx_c_socket_accept.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/net/ngx_c_socket_conn.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/net/ngx_c_socket_inet.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/net/ngx_c_socket_request.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/misc/ngx_c_threadpool.cpp.o
../bin/nginx: CMakeFiles/nginx.dir/build.make
../bin/nginx: CMakeFiles/nginx.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Linking CXX executable ../bin/nginx"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nginx.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/nginx.dir/build: ../bin/nginx

.PHONY : CMakeFiles/nginx.dir/build

CMakeFiles/nginx.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/nginx.dir/cmake_clean.cmake
.PHONY : CMakeFiles/nginx.dir/clean

CMakeFiles/nginx.dir/depend:
	cd /home/xufang/GitLab/nginx/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xufang/GitLab/nginx /home/xufang/GitLab/nginx /home/xufang/GitLab/nginx/cmake-build-debug /home/xufang/GitLab/nginx/cmake-build-debug /home/xufang/GitLab/nginx/cmake-build-debug/CMakeFiles/nginx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/nginx.dir/depend

