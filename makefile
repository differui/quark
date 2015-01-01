# setting
SHELL := /bin/bash

# config
build := ./build/

start: json
	@$(build)./json

json: clean build
	@cc -c *.c
	@mv *.o $(build)
	@cc $(build)*.o -o $(build)json

build:
	@mkdir $(build)

clean:
	@rm -rf $(build)
