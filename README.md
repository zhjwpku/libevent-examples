This repo is based on [jasonish/libevent-examples](https://github.com/jasonish/libevent-examples) with following changes:

- use libevent [release-2.1.12-stable](https://github.com/libevent/libevent/releases/tag/release-2.1.12-stable)
- add cmake build
- delete chat-server (will get back to this later)

**install libevent**

```
vagrant@ubuntu-xenial:~$ wget https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
vagrant@ubuntu-xenial:~$ tar -xf libevent-2.1.12-stable.tar.gz
vagrant@ubuntu-xenial:~$ cd libevent-2.1.12-stable/
vagrant@ubuntu-xenial:~/libevent-2.1.12-stable$ ./configure --disable-openssl && make && sudo make install
vagrant@ubuntu-xenial:~/libevent-2.1.12-stable$ pkg-config --cflags --libs libevent_core
-I/usr/local/include -L/usr/local/lib -levent_core

vagrant@ubuntu-xenial:~/libevent-2.1.12-stable$ ls /usr/local/include/event2/
buffer.h              dns.h           event_compat.h  keyvalq_struct.h  tag.h
buffer_compat.h       dns_compat.h    event_struct.h  listener.h        tag_compat.h
bufferevent.h         dns_struct.h    http.h          rpc.h             thread.h
bufferevent_compat.h  event-config.h  http_compat.h   rpc_compat.h      util.h
bufferevent_struct.h  event.h         http_struct.h   rpc_struct.h      visibility.h
vagrant@ubuntu-xenial:~/libevent-2.1.12-stable$ ls -l /usr/local/lib/ | grep libevent_core
lrwxrwxrwx 1 root root       26 Aug 20 11:20 libevent_core-2.1.so.7 -> libevent_core-2.1.so.7.0.1
-rwxr-xr-x 1 root root   908680 Aug 20 11:20 libevent_core-2.1.so.7.0.1
-rw-r--r-- 1 root root  1534838 Aug 20 11:20 libevent_core.a
-rwxr-xr-x 1 root root      994 Aug 20 11:20 libevent_core.la
lrwxrwxrwx 1 root root       26 Aug 20 11:20 libevent_core.so -> libevent_core-2.1.so.7.0.1
```

**build**

```
vagrant@ubuntu-xenial:~$ cd libevent-examples/
vagrant@ubuntu-xenial:~/libevent-examples$ mkdir build && cd build
vagrant@ubuntu-xenial:~/libevent-examples/build$ cmake .. && make
```

**test**

start server:

```
vagrant@ubuntu-xenial:~/libevent-examples/build/echo-server$ ./echosrv
vagrant@ubuntu-xenial:~/libevent-examples/build/echo-server$ ./buffered-echosrv
```

use telnet to connect

```
vagrant@ubuntu-xenial:~$ telnet localhost 5555
```

*All commands are executed in Ubuntu, check [Vagrantfile](Vagrantfile) for more infomation*

**read http://www.wangafu.net/~nickm/libevent-book/ for more information**
