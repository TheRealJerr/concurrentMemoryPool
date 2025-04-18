main : main.cpp
	g++ -o $@ $^ -std=c++11 -Wall -g

.PHONY: clean
clean:
	rm -rf main 