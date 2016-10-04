# setting
SHELL := /bin/bash

# config
build := ./build/

json: clean build
	@cc -c *.c
	@mv *.o $(build)
	@cc $(build)*.o -o $(build)json
	@$(build)./json

build:
	@mkdir $(build)

clean:
	@rm -rf $(build)
