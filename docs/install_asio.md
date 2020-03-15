# Как ставить asio 1.12.2

Ниже описана установка из исходников. Команды написаны для Ubuntu 18.04.

- Логинимся в контейнер под рутом:
    ```shell
    docker exec -it --user=root tpcc-image /bin/bash
    ```

- Ставим зависимости:
	```shell
	apt-get install libboost-all-dev wget autoconf
  ```
  
- Скачиваем архив с библиотекой:
	```shell
	mkdir asio-lib
	cd asio-lib
	wget https://launchpad.net/ubuntu/+archive/primary/+sourcefiles/asio/1:1.12.2-1/asio_1.12.2.orig.tar.gz
	tar -zxvf asio_1.12.2.orig.tar.gz
	cd ./asio-1.12.2/
	```
	
- Собираем и устанавливаем библиотеку:
	```shell
	autoreconf -i
	./configure --without-boost
	make
	make install
	```

## CLion

Чтобы asio был виден в CLion при работе в Remote Mode, нужно позвать `Tools` > `Resync with Remote Hosts`.

[Full Remote Mode | Resync header search paths](https://www.jetbrains.com/help/clion/remote-projects-support.html#resync)